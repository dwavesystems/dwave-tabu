//  Copyright 2020 D-Wave Systems Inc.
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

#include "bqp.h"

#include <math.h>
#include <limits>

#include "common.h"

using std::vector;

BQP::BQP(std::vector<std::vector<double>> Q) 
    : Q(Q), 
      nVars(Q.size()), 
      solutionQuality{0},
      nIterations{0},
      restartNum{0}, 
      iterNum{0},  
      evalNum{0}, 
      upperBound{-std::numeric_limits<double>::max()} {
    
    for (int i = 0; i < nVars; i++) {
        if (Q[i].size() != nVars) {
            throw Exception("Q must be a symmetric square matrix");
        }
        for (int j = i; j < nVars; j++) {
            if (Q[i][j] != Q[j][i]) {
                throw Exception("Q must be symmetric");
            }
        }
    }
}

void BQP::initialize(const vector<int> &initSolution) {
    toUpperTriangular();

    solution = initSolution;
    solutionQuality = getObjective(solution);
    nIterations = 1;
}

void BQP::toUpperTriangular() {
    for (int i = 0; i < nVars; i++) {
        for (int j = i + 1; j < nVars; j++) {
            Q[i][j] = Q[i][j] + Q[j][i];
            Q[j][i] = 0;
        }
    }
}

double BQP::getObjective(const vector<int> &solution) {
    double cost = 0;
    vector<int> zeroSolution(nVars, 0);

    for (int i = nVars; i--;) {
        if (solution[i] == 1) {
            cost += getChangeInObjective(zeroSolution, i);
            zeroSolution[i] = 1;
        }
    }
    return cost;
}

double BQP::getChangeInObjective(const vector<int> &oldSolution, int flippedBit) {
    // Add up all biases associated with the variable at flippedBit
    double change = Q[flippedBit][flippedBit];
    for (int i = 0; i < nVars; i++) {
        if ((oldSolution[i] == 1) && (i != flippedBit)) {
            change += Q[flippedBit][i] + Q[i][flippedBit];
        }
    }

    // Flipping to 0 = negative change, flipping to 1 = positive change
    return oldSolution[flippedBit] ? -change : change;
}

double BQP::getMaxBQPCoeff() {
    double M = Q[0][0];
    for (int i = 0; i < nVars; i++) {
        for (int j = 0; j < nVars; j++) {
            if (M < abs(Q[i][j])) {
                M = abs(Q[i][j]);
            }
        }
    }
    return M;
}

void BQP::printQ() {
    printf("BQP: Number of variables: %d\nCoefficient matrix:\n", nVars);
    printf("{\n");
    for (int i = 0; i < nVars; i++) {
        printf("{");
        for (int j = 0; j < nVars; j++) {
            printf("%6f,", Q[i][j]);
        }
        printf("},\n");
    }
    printf("}\n");
}

void BQP::printSolution() {
    printf("Objective function value: %f\n", getObjective(solution));
    printf("Variable assignment:\n");
    for(int i = 0; i < nVars; i++) {
        printf("%d ", solution[i]);
    }
    printf("\n");
}
