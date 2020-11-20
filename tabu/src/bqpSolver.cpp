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

#include "bqpSolver.h"

#include <limits>

using std::vector;

double bqpSolver_tabooSearch(
    BQP *bqp,
    const vector<int> &starting,
    double startingObjective,
    int tt,
    long long ZCoeff,
    long long timeLimitInMilliSecs,
    const bqpSolver_Callback *callback
) {
    bqp->restartNum++;/*added to record more statistics.*/
    long long startTime = realtime_clock();
    int i, k;
    int globalMinFound;
    long /*objectiveChangeCtr = 0,*/ localSearchCtr = 0;
    long long iter = 0;
    int bestK;
    double localMinCost, cost = 0;
    double prevCost;
    vector<int> taboo(bqp->nVars);
    vector<int> solution(bqp->nVars);
    int tabooTenure = (20 < (int)(bqp->nVars / 4.0))? 20 : (int)(bqp->nVars / 4.0); // num of solutions we keep track of
    long long maxIter = (500000 > ZCoeff * (long long)bqp->nVars)? 500000 : ZCoeff * (long long)bqp->nVars;
    vector<double> changeInObjective(bqp->nVars);
    double change;

    vector<int> tieList(bqp->nVars);
    int numTies = 0;

    if (tt > 0) { 
      tabooTenure = tt;
    }

    for (i = 0; i < bqp->nVars; i++) {
        taboo[i] = 0;
        solution[i] = starting[i];
        bqp->solution[i] = starting[i];
        changeInObjective[i] = bqpUtil_getChangeInObjective(bqp, starting, i);
    }
    bqp->solutionQuality = startingObjective;
    prevCost = bqp->solutionQuality;

    while (iter < maxIter && (realtime_clock() - startTime) < timeLimitInMilliSecs) {
        bqp->iterNum++; /*added to record more statistics.*/
        localMinCost = std::numeric_limits<double>::max();
        bestK = -1;
        globalMinFound = 0;
        numTies = 0;
        bqp->evalNum += bqp->nVars; /*added to record more statistics.*/

        for (k = 0; k < bqp->nVars; k++) {
            if (taboo[k] != 0) {
                // variable at k was not recently flipped
                continue;
            }
            iter++;
            cost = prevCost + changeInObjective[k];
            if (cost < bqp->solutionQuality) {
                // A better solution was found. Local search procedure will be invoked later.
                bestK = k;
                globalMinFound = 1;
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
            bestK = numTies * (double)(rand())/((double)(RAND_MAX)+1);
            bestK = tieList[bestK];
        }
        for (i = 0; i < bqp->nVars; i++) {
            if(taboo[i] > 0) {
                taboo[i] = taboo[i] - 1;
            }
        }
        if (bestK == -1) {
            continue;
        }
        solution[bestK] = 1 - solution[bestK];
        prevCost = localMinCost;
        for (i = 0; i < bestK; i++) {
            change = bqp->Q[i][bestK];
            if (change) {
                changeInObjective[i] += (solution[i] ^ solution[bestK])? change : -change;
            }
        }
        for (i = bestK + 1; i < bqp->nVars; i++) {
            change = bqp->Q[bestK][i];
            if (change) {
                changeInObjective[i] += (solution[i] ^ solution[bestK])? change : -change;
            }
        }
        changeInObjective[bestK] = -changeInObjective[bestK];
        taboo[bestK] = tabooTenure;
        if (globalMinFound == 1) {
            localSearchCtr++;
            bqpSolver_localSearchInternal(bqp, solution, cost, changeInObjective);
            solution = bqp->solution;
            prevCost = bqp->solutionQuality;
            iter += bqp->nIterations;
            bqp->nIterations = iter;

            if (callback != nullptr) {
              callback->func(callback, bqp);
            }

            if (bqp->solutionQuality <= bqp->upperBound) timeLimitInMilliSecs = 0;
        }
    }
    return bqp->solutionQuality;
}

double bqpSolver_localSearchInternal(BQP *bqp, const vector<int> &starting, double startingObjective, vector<double> &changeInObjective) {
    int i, j;
    long long iter = 0;
    double change;
    bool improved;
    bqp->solution = starting;
    bqp->solutionQuality = startingObjective;
    do {
        improved = false;
        for (i = 0; i < bqp->nVars; i++, iter++) {
            bqp->evalNum++; /*added to record more statistics.*/
            if (changeInObjective[i] < 0) {
                improved = true;
                bqp->solution[i] = 1 - bqp->solution[i];
                bqp->solutionQuality = bqp->solutionQuality + changeInObjective[i];
                changeInObjective[i] = -changeInObjective[i];
                for (j = 0; j < bqp->nVars; j++) {
                    if (j != i) {
                        change = bqp->Q[i][j] + bqp->Q[j][i];
                        changeInObjective[j] += (bqp->solution[j] ^ bqp->solution[i])? change : -change;
                    }
                }
            }
        }
    } while(improved);
    bqp->nIterations = iter;
    return bqp->solutionQuality;
}

// not currently used
double bqpSolver_localSearch(BQP *bqp, const vector<int> &starting) {
    vector<double> changeInObjective(bqp->nVars);
    for (int i = 0; i < bqp->nVars; i++) {
        changeInObjective[i] = bqpUtil_getChangeInObjective(bqp, starting, i);
    }
    bqpSolver_localSearchInternal(bqp, starting, bqpUtil_getObjective(bqp, starting), changeInObjective);
    return bqp->solutionQuality;
}

void bqpSolver_selectVariables(BQP *bqp, int numSelection, vector<vector<double> > &C, vector<int> &I) {
    int i, ctr;
    vector<double> d(bqp->nVars);   // estimate used to calculate e
    for (i = 0; i < bqp->nVars; i++) {
        d[i] = C[i][i];
    }

    vector<double> e(bqp->nVars);   // used to assign probability of being selected as a free variable
    
    vector<double> prob(bqp->nVars);
    vector<int> selected(bqp->nVars, 0);   
    double prevProb, sumE;

    for (ctr = 0; ctr < numSelection; ctr++) {
        double dmin = std::numeric_limits<double>::max();
        double dmax = -std::numeric_limits<double>::max();
        for (i = 0; i < bqp->nVars; i++) {
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
            for (i = 0; i < bqp->nVars; i++) {
                if (selected[i] == 1) {
                    continue;
                }
                e[i] = 1;
            }
        }
        else {
            for (i = 0; i < bqp->nVars; i++) {
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
        for (i = 0, sumE = 0, prevProb = 0; i < bqp->nVars; i++) {
            if (selected[i] == 1) {
                continue;
            }
            prob[i] = prevProb + e[i];
            prevProb = prob[i];
            sumE = sumE + e[i];
        }
        if (sumE == 0) {
            sumE = 1;
            for (int i = 0; i < bqp->nVars; i++) {
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
        double selectedProb = (double)rand() / ((double)RAND_MAX + 1);
        int selectedVar = -1;
        for (int i = 0; i < bqp->nVars; i++) {
            if (selected[i] == 1) {
                continue;
            }
            if (selectedProb <= prob[i] / sumE) {
                selectedVar = i;
                break;
            }
        }
        if (selectedVar < 0 || selectedVar > bqp->nVars - 1) {
            printf("ERROR!!!\n");
        }
        I[ctr] = selectedVar;
        selected[selectedVar] = 1;
        for (int i = 0; i < bqp->nVars; i++) {
            if (selected[i] == 0) {
                d[i] = d[i] + C[i][selectedVar];    // update d for each unselected variable
            }
        }
    }
}

void bqpSolver_steepestAscent(vector<int> &solution, BQP *bqp, vector<vector<double> > &C, vector<int> &I, int n) {
    int i, j, ctr;
    int idI, idJ, r, v = 0;
    vector<double> h1(bqp->nVars);
    vector<double> h2(bqp->nVars);
    vector<double> q1(bqp->nVars);
    vector<double> q2(bqp->nVars);
    vector<int> visited(bqp->nVars, 0);
    double V1, V2;

    std::fill(solution.begin(), solution.end(), 0); // all vars outside of selected variables (I) stay fixed at 0

    for (i = 0; i < n; i++) {
        idI = I[i];
        h1[idI] = C[idI][idI];
        h2[idI] = 0;
        for (j = 0; j < n; j++) {
            idJ = I[j];
            if (idJ != idI) {
                h2[idI] = h2[idI] + (C[idI][idJ]);
            }
        }
        h2[idI] = h2[idI] * 1;
    }

    for (ctr = 0; ctr < n; ctr++) {
        V1 = -std::numeric_limits<double>::max();
        V2 = -std::numeric_limits<double>::max();
        for (i = 0; i < n; i++) {
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
        for (i = 0; i < n; i++) {
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

void bqpSolver_computeC(vector<vector<double> > &C, BQP *bqp, const vector<int> &solution) {
    for (int i = 0; i < bqp->nVars; i++) {
        C[i][i] = -bqp->Q[i][i];
        for (int j = i + 1; j < bqp->nVars; j++) {
            if (solution[j] == 1) {
                C[i][i] += -(bqp->Q[i][j]);
            }
            C[i][j] = (solution[i] == solution[j])? -bqp->Q[i][j] : bqp->Q[i][j];
            C[j][i] = C[i][j];
        }
        C[i][i] = (solution[i] == 1)? -C[i][i] : C[i][i];
    }
}

double bqpSolver_multiStartTabooSearch(
    BQP *bqp,
    long long timeLimitInMilliSecs,
    int numStarts,
    int tabuTenure,
    const vector<int> &initSolution,
    const bqpSolver_Callback *callback
) {
    long long startTime = realtime_clock();
    int i;
    long iter;
    vector<vector<double> > C(bqp->nVars);
    vector<int> I(bqp->nVars); // will store set of variables to apply steepest ascent to
    vector<int> solution(bqp->nVars);
    vector<int> bestSolution(bqp->nVars);
    double bestSolutionQuality;
    int numSelection;
    int Z1Coeff = (bqp->nVars <= 500)? 10000 : 25000;
    int Z2Coeff = (bqp->nVars <= 500)? 2500 : 10000;
    for (i = 0; i < bqp->nVars; i++) {
        C[i].resize(bqp->nVars);
    }

    bqpUtil_convertBQPToUpperTriangular(bqp);
    bqpUtil_initBQPSolution(bqp, initSolution);

    bqpSolver_tabooSearch(bqp, bqp->solution, bqp->solutionQuality, tabuTenure, Z1Coeff, timeLimitInMilliSecs, callback);

    bestSolutionQuality = bqp->solutionQuality;
    for (i = 0; i < bqp->nVars; i++) {
        bestSolution[i] = bqp->solution[i];
    }

    for (iter = 0; iter < numStarts && ((realtime_clock() - startTime) < timeLimitInMilliSecs); iter++) {    
        // Compute coefficients from current solution (used later to get solution from steepestAscent())
        bqpSolver_computeC(C, bqp, bqp->solution);

        // Select a group of variables (I) and apply steepest ascent to it
        numSelection = (10 > (int)(ALPHA * bqp->nVars))? 10 : (int)(ALPHA * bqp->nVars);
        if (numSelection > bqp->nVars) {
            numSelection = bqp->nVars;
        }
        bqpSolver_selectVariables(bqp, numSelection, C, I);   

        // Construct new initial solution to apply taboo search to 
        bqpSolver_steepestAscent(solution, bqp, C, I, numSelection);    

        for (i = 0; i < numSelection; i++) {
            if (solution[I[i]] == 1) {
                bqp->solution[I[i]] = 1 - bqp->solution[I[i]];  // flipping variable
            }
        }
        bqp->solutionQuality = bqpUtil_getObjective(bqp, bqp->solution);

        // Run taboo search and update solution again
        bqpSolver_tabooSearch(bqp, bqp->solution, bqp->solutionQuality, tabuTenure, Z2Coeff, timeLimitInMilliSecs - (realtime_clock() - startTime), callback);
        
        if (bestSolutionQuality > bqp->solutionQuality) {
            bestSolutionQuality = bqp->solutionQuality;
            for (i = 0; i < bqp->nVars; i++) {
                bestSolution[i] = bqp->solution[i];
            }
        }

        if (callback != NULL) {
            callback->func(callback, bqp);
        }
    }
    
    bqp->solutionQuality = bestSolutionQuality;
    bqp->solution = bestSolution;

    return bqp->solutionQuality;
}

// not currently used
int bqpSolver_naiveSearch(BQP *bqp) {
    int i;
    unsigned long long num, grayNum;
    double cost, prevCost, minCost;
    int flippedBit;
    vector<int> solution(bqp->nVars);
    vector<int> prevSolution(bqp->nVars);
    if(bqp->nVars > 64) {
        bqpUtil_randomizeBQPSolution(bqp);
        return bqp->solutionQuality;
    }
    for(i = 0; i < bqp->nVars; i++) {
        prevSolution[i] = 0;
        bqp->solution[i] = 0;
    }
    bqp->nIterations = (long)pow(2.0, bqp->nVars);
    prevCost = 0;
    minCost = 0;
    for(num = 1; num < bqp->nIterations; num++) {
        grayNum = num ^ (num >> 1);
        for(i = 0; i < bqp->nVars; i++) {
            solution[i] = 0;
        }
        for(i = 0; grayNum != 0 && i < bqp->nVars; i++) {
            solution[i] = (int)(grayNum & 1);
            grayNum = grayNum >> 1;
        }
        flippedBit = -1;
        for(i = 0; i < bqp->nVars; i++) {
            if(solution[i] != prevSolution[i]) {
                flippedBit = i;
                break;
            }
        }
        cost = prevCost + bqpUtil_getChangeInObjective(bqp, prevSolution, flippedBit);
        prevCost = cost;
        prevSolution[flippedBit] = solution[flippedBit];

        if(minCost >= cost) {
            minCost = cost;
            for(i = 0; i < bqp->nVars; i++) {
                bqp->solution[i] = solution[i];
            }
            bqp->solutionQuality = minCost;
        }
    }
    return bqp->solutionQuality;
}

// not currently used
double bqpSolver_restrictedLocalSearchInternal(BQP *bqp, const vector<int> &starting, const vector<int> &restricted, double startingObjective, vector<double> &changeInObjective) {
    int i, j;
    long long iter = 0;
    double objective, change;
    int improved;
    for(i = 0; i < bqp->nVars; i++) {
        bqp->solution[i] = starting[i];
    }
    bqp->solutionQuality = startingObjective;
    do {
        improved = 0;
        for(i = 0; i < bqp->nVars; i++, iter++) {
            if(restricted[i] == 1) {
                continue;
            }
            objective = bqp->solutionQuality + changeInObjective[i];
            if(objective < bqp->solutionQuality) {
                bqp->solution[i] = 1 - bqp->solution[i];
                bqp->solutionQuality = objective;
                improved = 1;
                changeInObjective[i] = -changeInObjective[i];
                for(j = 0; j < bqp->nVars; j++) {
                    change = bqp->Q[i][j] + bqp->Q[j][i];
                    if(change && (j ^ i)) {
                        changeInObjective[j] += (bqp->solution[j] ^ bqp->solution[i])? change : -change;
                    }
                }
            }
        }
    } while(improved);
    bqp->nIterations = iter;
    return bqp->solutionQuality;
}

// not currently used
double bqpSolver_restrictedLocalSearch(BQP *bqp, const vector<int> &starting, const vector<int> &restricted) {
    vector<double> changeInObjective(bqp->nVars);
    for (int i = 0; i < bqp->nVars; i++) {
        changeInObjective[i] = bqpUtil_getChangeInObjective(bqp, starting, i);
    }
    bqpSolver_restrictedLocalSearchInternal(bqp, starting, restricted, bqpUtil_getObjective(bqp, starting), changeInObjective);
    return bqp->solutionQuality;
}
