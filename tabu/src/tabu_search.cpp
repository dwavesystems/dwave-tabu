//  Copyright 2018 D-Wave Systems Inc.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include "tabu_search.h"

#include <limits>

#include "common.h"
#include "utils.h"

using std::vector;
using std::size_t;

TabuSearch::TabuSearch(vector<vector<double>> Q, 
                       const vector<int> initSol, 
                       int tenure, 
                       long int timeout,
                       int numRestarts,
                       unsigned int seed,
                       double energyThreshold) 
    : bqp(Q) {
    
    size_t nvars = Q.size();
    if (initSol.size() != nvars)
        throw Exception("length of init_solution doesn't match the size of Q");

    if (tenure < 0 || tenure > (nvars - 1)) {
        throw Exception("tenure must be in the range [0, num_vars - 1]");
    }
    else if (tenure > 0) {
        tabooTenure = tenure;
    }
    else {
        tabooTenure = (20 < (int)(bqp.nVars / 4.0))? 20 : (int)(bqp.nVars / 4.0);
    }

    generator.seed(seed);

    // Solve and update bqp
    multiStartTabuSearch(timeout, numRestarts, energyThreshold, initSol, nullptr);
}

double TabuSearch::bestEnergy()
{
    return bqp.getObjective(bqp.solution);
}

vector<int> TabuSearch::bestSolution()
{
    return bqp.solution;
}

int TabuSearch::numRestarts()
{
    return bqp.restartNum;
}

void TabuSearch::multiStartTabuSearch(long long timeLimitInMilliSecs, 
                                      int numRestarts, 
                                      double energyThreshold,
                                      const vector<int> &initSolution, 
                                      const bqpSolver_Callback *callback) {

    long long startTime = realtime_clock();

    vector<int> I(bqp.nVars); // will store set of variables to apply steepest ascent to

    // Z coeffs are used to define the max number of iterations for each individual tabu search
    int Z1Coeff = (bqp.nVars <= 500)? 10000 : 25000;
    int Z2Coeff = (bqp.nVars <= 500)? 2500 : 10000;

    bqp.initialize(initSolution);

    bool useTimeLimit = timeLimitInMilliSecs >= 0;

    simpleTabuSearch(bqp.solution, 
                     bqp.solutionQuality, 
                     Z1Coeff, 
                     timeLimitInMilliSecs, 
                     useTimeLimit, 
                     energyThreshold, 
                     callback);

    double bestSolutionQuality = bqp.solutionQuality;
    vector<int> bestSolution(bqp.solution.begin(), bqp.solution.end());

    vector<vector<double>> C(bqp.nVars, vector<double>(bqp.nVars));

    for (long iter = 0; iter < numRestarts; iter++) {
        if ((bestSolutionQuality <= energyThreshold) ||
            (useTimeLimit && (realtime_clock() - startTime) > timeLimitInMilliSecs)) {
            break;
        }

        // Compute coefficients from current solution (used later to get solution from steepestAscent())
        computeC(C, bqp.solution);

        // Select a group of variables (I) and apply steepest ascent to it
        int numSelection = (10 > (int)(ALPHA * bqp.nVars))? 10 : (int)(ALPHA * bqp.nVars);
        if (numSelection > bqp.nVars) {
            numSelection = bqp.nVars;
        }

        selectVariables(numSelection, C, I);  

        // Construct new initial solution to apply taboo search to 
        vector<int> solution(bqp.nVars);
        steepestAscent(numSelection, C, I, solution);    

        for (int i = 0; i < numSelection; i++) {
            if (solution[I[i]] == 1) {
                bqp.solution[I[i]] = 1 - bqp.solution[I[i]];  // flipping variable
            }
        }
        bqp.solutionQuality = bqp.getObjective(bqp.solution);

        // Run taboo search and update solution again
        bqp.restartNum++;
        simpleTabuSearch(bqp.solution, 
                         bqp.solutionQuality, 
                         Z2Coeff, 
                         timeLimitInMilliSecs - (realtime_clock() - startTime), 
                         useTimeLimit, 
                         energyThreshold, 
                         callback);
    
        if (bestSolutionQuality > bqp.solutionQuality) {
            bestSolutionQuality = bqp.solutionQuality;
            bestSolution = bqp.solution;
        }

        if (callback != nullptr) {
            callback->func(callback, &bqp);
        }
    }
    
    bqp.solutionQuality = bestSolutionQuality;
    bqp.solution = bestSolution;
}

void TabuSearch::simpleTabuSearch(const vector<int> &starting,
                                  double startingObjective,
                                  long long ZCoeff,
                                  long long timeLimitInMilliSecs,
                                  bool useTimeLimit,
                                  double energyThreshold,
                                  const bqpSolver_Callback *callback) {

    long long startTime = realtime_clock();
    bqp.solutionQuality = startingObjective;

    vector<int> taboo(bqp.nVars);  // used to keep track of history of flipped bits
    vector<int> solution(bqp.nVars);
    vector<double> changeInObjective(bqp.nVars);

    for (int i = 0; i < bqp.nVars; i++) {
        taboo[i] = 0;
        solution[i] = starting[i];
        bqp.solution[i] = starting[i];
        changeInObjective[i] = bqp.getChangeInObjective(starting, i);
    }

    double prevCost = bqp.solutionQuality;
    double cost = 0;

    vector<int> tieList(bqp.nVars);

    long long iter = 0;
    long long maxIter = (500000 > ZCoeff * (long long)bqp.nVars)? 500000 : ZCoeff * (long long)bqp.nVars;

    while (iter < maxIter) {
        if ((bqp.solutionQuality <= energyThreshold) ||
            (useTimeLimit && (realtime_clock() - startTime) > timeLimitInMilliSecs)) {
            break;
        }

        bqp.iterNum++; // added to record more statistics
        double localMinCost = std::numeric_limits<double>::max();
        int bestK = -1;
        bool globalMinFound = false;
        int numTies = 0;
        bqp.evalNum += bqp.nVars; // added to record more statistics

        for (int k = 0; k < bqp.nVars; k++) {
            if (taboo[k] != 0) {
                // variable at k was not recently flipped
                continue;
            }
            iter++;
            cost = prevCost + changeInObjective[k];
            if (cost < bqp.solutionQuality) {
                globalMinFound = true;
                bestK = k;
                break;
            }
            if (cost < localMinCost) {
                bestK = k;
                localMinCost = cost;
                tieList[0] = k;
                numTies = 1;
            }
            else if (cost == localMinCost) {
                tieList[numTies] = k; numTies++;
            }
        }

        if (!globalMinFound && numTies > 1) {
            bestK = numTies * (double)generator() / ((double)generator.max() + 1);
            bestK = tieList[bestK];
        }
        for (int i = 0; i < bqp.nVars; i++) {
            if (taboo[i] > 0) {
                taboo[i] = taboo[i] - 1;
            }
        }
        if (bestK == -1) {
            continue;
        }
        solution[bestK] = 1 - solution[bestK];
        prevCost = localMinCost;
        for (int i = 0; i < bestK; i++) {
            double change = bqp.Q[i][bestK];
            changeInObjective[i] += (solution[i] != solution[bestK])? change : -change; 
        }
        for (int i = bestK + 1; i < bqp.nVars; i++) {
            double change = bqp.Q[bestK][i];
            changeInObjective[i] += (solution[i] != solution[bestK])? change : -change;
        }
        changeInObjective[bestK] = -changeInObjective[bestK];
        taboo[bestK] = tabooTenure;
        if (globalMinFound) {
            localSearchInternal(solution, cost, changeInObjective);
            solution = bqp.solution;
            prevCost = bqp.solutionQuality;
            iter += bqp.nIterations;
            bqp.nIterations = iter;

            if (callback != nullptr) {
                callback->func(callback, &bqp);
            }

            if (bqp.solutionQuality <= bqp.upperBound) {
                timeLimitInMilliSecs = 0;
            }
        }
    }
}

void TabuSearch::localSearchInternal(const vector<int> &starting, double startingObjective, vector<double> &changeInObjective) {
    bqp.solution = starting;
    bqp.solutionQuality = startingObjective;

    long long iter = 0;
    bool improved;

    do {
        improved = false;
        for (int i = 0; i < bqp.nVars; i++, iter++) {
            bqp.evalNum++; /*added to record more statistics.*/
            if (changeInObjective[i] < 0) {
                improved = true;
                bqp.solution[i] = 1 - bqp.solution[i];
                bqp.solutionQuality = bqp.solutionQuality + changeInObjective[i];
                changeInObjective[i] = -changeInObjective[i];
                for (int j = 0; j < bqp.nVars; j++) {
                    if (j != i) {
                        double change = bqp.Q[i][j] + bqp.Q[j][i];
                        changeInObjective[j] += (bqp.solution[j] != bqp.solution[i])? change : -change;
                    }
                }
            }
        }
    } while(improved);

    bqp.nIterations = iter;
}

void TabuSearch::selectVariables(int numSelection, vector<vector<double>> &C, vector<int> &I) {
    int i, ctr;
    vector<double> d(bqp.nVars);   // estimate used to calculate e
    for (i = 0; i < bqp.nVars; i++) {
        d[i] = C[i][i];
    }

    vector<double> e(bqp.nVars);   // used to assign probability of being selected as a free variable  
    vector<double> prob(bqp.nVars);
    vector<int> selected(bqp.nVars, 0);   
    double prevProb, sumE;

    for (ctr = 0; ctr < numSelection; ctr++) {
        double dmin = std::numeric_limits<double>::max();
        double dmax = -std::numeric_limits<double>::max();
        for (i = 0; i < bqp.nVars; i++) {
            if (selected[i] == 1) {
                continue;
            }
            if (d[i] > dmax) {
                dmax = d[i];
            }
            if (d[i] < dmin) {
                dmin = d[i];
            }
        }
        if (dmin == dmax) {
            for (i = 0; i < bqp.nVars; i++) {
                if (selected[i] == 1) {
                    continue;
                }
                e[i] = 1;
            }
        }
        else {
            for (i = 0; i < bqp.nVars; i++) {
                if(selected[i] == 1) {
                    continue;
                }
                if (d[i] <= 0 && dmin < 0) {
                    e[i] = 1 - d[i] / dmin;
                }
                else if (d[i] == dmin && dmin == 0) {
                    e[i] = 0;
                }
                else {
                    e[i] = 1 + LAMBDA * (d[i] / dmax);
                }
            }
        }
        for (i = 0, sumE = 0, prevProb = 0; i < bqp.nVars; i++) {
            if (selected[i] == 1) {
                continue;
            }
            prob[i] = prevProb + e[i];
            prevProb = prob[i];
            sumE = sumE + e[i];
        }
        if (sumE == 0) {
            sumE = 1;
            for (i = 0; i < bqp.nVars; i++) {
                if (selected[i] == 0) {
                    if (prob[i] == 0) {
                        prob[i] = 1;
                    }
                    else {
                        printf("ERROR: probability is non-zero and yet the sum is zero\n");
                    }
                }
            }
        }
        double selectedProb = (double)generator() / ((double)generator.max() + 1);
        int selectedVar = -1;
        for (int i = 0; i < bqp.nVars; i++) {
            if (selected[i] == 1) {
                continue;
            }
            if (selectedProb <= prob[i] / sumE) {
                selectedVar = i;
                break;
            }
        }
        if (selectedVar < 0 || selectedVar > bqp.nVars - 1) {
            printf("ERROR!!!\n");
        }
        I[ctr] = selectedVar;
        selected[selectedVar] = 1;
        for (int i = 0; i < bqp.nVars; i++) {
            if (selected[i] == 0) {
                d[i] = d[i] + C[i][selectedVar];    // update d for each unselected variable
            }
        }
    }
}

void TabuSearch::steepestAscent(int numSelection, vector<vector<double>> &C, vector<int> &I, vector<int> &solution) {
    int i, j, ctr;
    int idI, idJ, r, v = 0;
    vector<double> h1(bqp.nVars);
    vector<double> h2(bqp.nVars);
    vector<double> q1(bqp.nVars);
    vector<double> q2(bqp.nVars);
    vector<int> visited(bqp.nVars, 0);

    std::fill(solution.begin(), solution.end(), 0); // all vars outside of selected variables (I) stay fixed at 0

    for (i = 0; i < numSelection; i++) {
        idI = I[i];
        h1[idI] = C[idI][idI];
        h2[idI] = 0;
        for (j = 0; j < numSelection; j++) {
            idJ = I[j];
            if (idJ != idI) {
                h2[idI] = h2[idI] + (C[idI][idJ]);
            }
        }
        h2[idI] = h2[idI] * 1;
    }

    for (ctr = 0; ctr < numSelection; ctr++) {
        double V1 = -std::numeric_limits<double>::max();
        double V2 = -std::numeric_limits<double>::max();
        for (i = 0; i < numSelection; i++) {
            idI = I[i];
            if (visited[idI] == 1) {
                continue;
            }
            q1[idI] = 2 * h1[idI] + h2[idI];
            q2[idI] = h1[idI];
            if (q1[idI] > 0 || (q1[idI] == 0 && q2[idI] >= 0)) {
                r = 1;
            }
            else {
                r = 0;
                q1[idI] = -q1[idI];
                q2[idI] = -q2[idI];
            }
            if (q1[idI] > V1 ||(q1[idI] == V1 && q2[idI] > V2)) {
                V1 = q1[idI];
                V2 = q2[idI];
                j = idI;
                v = r;
            }
        }
        solution[j] = v;
        visited[j] = 1;
        for (i = 0; i < numSelection; i++) {
            idI = I[i];
            if (visited[idI] == 1) {
                continue;
            }
            h2[idI] = h2[idI] - 1 * (C[idI][j]);
            if (v == 1) {
                h1[idI] = h1[idI] + 1 * (C[idI][j]);
            }
        }
    }
}

void TabuSearch::computeC(vector<vector<double>> &C, const vector<int> &solution) {
    for (int i = 0; i < bqp.nVars; i++) {
        C[i][i] = -bqp.Q[i][i];
        for (int j = i + 1; j < bqp.nVars; j++) {
            if (solution[j] == 1) {
                C[i][i] += -(bqp.Q[i][j]);
            }
            C[i][j] = (solution[i] == solution[j])? -bqp.Q[i][j] : bqp.Q[i][j];
            C[j][i] = C[i][j];
        }
        C[i][i] = (solution[i] == 1)? -C[i][i] : C[i][i];
    }
}
