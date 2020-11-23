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

#include "bqpUtil.h"

#include <stdlib.h>

using std::vector;

double bqpUtil_getMaxBQPCoeff(const BQP* bqp) {
    double M = bqp->Q[0][0];
    for (int i = 0; i < bqp->nVars; i++) {
        for (int j = 0; j < bqp->nVars; j++) {
            if (M < abs(bqp->Q[i][j])) {
                M = abs(bqp->Q[i][j]);
            }
        }
    }
    return M;
}

void bqpUtil_convertBQPToUpperTriangular(BQP *bqp) {
    for (int i = 0; i < bqp->nVars; i++) {
        for (int j = i + 1; j < bqp->nVars; j++) {
            bqp->Q[i][j] = bqp->Q[i][j] + bqp->Q[j][i];
            bqp->Q[j][i] = 0;
        }
    }
}

void bqpUtil_print(const BQP *bqp) {
    printf("BQP: Number of variables: %d\nCoefficient matrix:\n", bqp->nVars);
    printf("{\n");
    for (int i = 0; i < bqp->nVars; i++) {
        printf("{");
        for (int j = 0; j < bqp->nVars; j++) {
            printf("%6f,", bqp->Q[i][j]);
        }
        printf("},\n");
    }
    printf("}\n");
}

double bqpUtil_getChangeInObjective(const BQP *bqp, const vector<int> &oldSolution, int flippedBit) {
    bool changed = oldSolution[flippedBit] != 1;

    double inc = bqp->Q[flippedBit][flippedBit];
    double change = (changed)? inc : -inc;  

    for (int i = bqp->nVars; i--;) {
        // looking for all the other variables that are already flipped
        if ((oldSolution[i] == 1) && (i != flippedBit)) {
            inc = bqp->Q[flippedBit][i] + bqp->Q[i][flippedBit];
            change += (changed)?  inc : -inc;
        }
    }
    return change;
}

double bqpUtil_getObjective(const BQP *bqp, const vector<int> &solution) {
    double cost = 0;
    vector<int> zeroSolution(bqp->nVars, 0);

    for (int i = bqp->nVars; i--;) {
        if (solution[i] == 1) {
            cost += bqpUtil_getChangeInObjective(bqp, zeroSolution, i);
            zeroSolution[i] = 1;
        }
    }
    return cost;
}

double bqpUtil_getObjectiveIncremental(const BQP *bqp, const vector<int> &solution, const vector<int> &oldSolution, double oldCost) {
    double cost = oldCost;
    vector<int> oldSolCopy(oldSolution.begin(), oldSolution.end());

    for (int i = bqp->nVars; i--;) {
        if (solution[i] != oldSolCopy[i]) {
            cost += bqpUtil_getChangeInObjective(bqp, oldSolCopy, i);
            oldSolCopy[i] = solution[i];
        }
    }
    return cost;
}

void bqpUtil_initBQPSolution(BQP *bqp, const vector<int> &initSolution) {
    bqp->solution = initSolution;
    bqp->solutionQuality = bqpUtil_getObjective(bqp, bqp->solution);
    bqp->nIterations = 1;
}

void bqpUtil_randomizeBQPSolution(BQP *bqp) {
    for (int i = 0; i < bqp->nVars; i++) {
        if ((rand() / (double)RAND_MAX) < 0.5) {
            bqp->solution[i] = 0;
        }
        else {
            bqp->solution[i] = 1;
        }
    }
    bqp->nIterations = 1;
    bqp->solutionQuality = bqpUtil_getObjective(bqp, bqp->solution);
}

void bqpUtil_printSolution(const BQP *bqp) {
    printf("Objective function value: %f\n", bqpUtil_getObjective(bqp, bqp->solution));
    printf("Variable assignment:\n");
    for(int i = 0; i < bqp->nVars; i++) {
        printf("%d ", bqp->solution[i]);
    }
    printf("\n");
}


#if defined(_WIN32) || defined(_WIN64)

#include <Windows.h>

long long realtime_clock() {
    LARGE_INTEGER frequency;
    LARGE_INTEGER now;

    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&now);

    return (long long)(1000.0 * now.QuadPart / frequency.QuadPart);
}

#else

long long realtime_clock() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

#endif
