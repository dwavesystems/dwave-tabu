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

#include <stdlib.h>
#include <vector>

#include "bqpUtil.h"

using std::vector;

double bqpUtil_getMaxBQPCoeff(BQP *bqp) {
    int i, j;
    double M = bqp->Q[0][0];
    for(i = 0; i < bqp->nVars; i++) {
        for(j = 0; j < bqp->nVars; j++) {
            if(M < abs(bqp->Q[i][j])) {
                M = abs(bqp->Q[i][j]);
            }
        }
    }
    return M;
}

void bqpUtil_convertBQPToUpperTriangular(BQP *bqp) {
    int i, j;
    for(i = 0; i < bqp->nVars; i++) {
        for(j = i + 1; j < bqp->nVars; j++) {
            bqp->Q[i][j] = bqp->Q[i][j] + bqp->Q[j][i];
            bqp->Q[j][i] = 0;
        }
    }
}

void bqpUtil_print(BQP *bqp) {
    int i, j;
    printf("BQP: Number of variables: %d\nCoefficient matrix:\n", bqp->nVars);
    printf("{\n");
    for(i = 0; i < bqp->nVars; i++) {
        printf("{");
        for(j = 0; j < bqp->nVars; j++) {
            printf("%6ld,", bqp->Q[i][j]);
        }
        printf("},\n");
    }
    printf("}\n");
}

double bqpUtil_getChangeInObjective(BQP *bqp, int *oldSolution, int flippedBit) {
    int i;
    double change = 0, inc;
    change += (oldSolution[flippedBit] == 1)? (-1 * bqp->Q[flippedBit][flippedBit]) : bqp->Q[flippedBit][flippedBit];
    for(i = bqp->nVars; i--;) {
        if(!(oldSolution[i] ^ 1) && i ^ flippedBit) {
            inc = bqp->Q[flippedBit][i] + bqp->Q[i][flippedBit];
            change += (oldSolution[flippedBit] ^ 1) ?  inc : -inc;
        }
    }
    return change;
}

double bqpUtil_getObjective(BQP *bqp, int * solution) {
    int i;
    double cost = 0;
    vector<int> u_zeroSol(bqp->nVars);
    int *zeroSolution = vector_data<int>(u_zeroSol);
    for(i = bqp->nVars; i--;) {
        zeroSolution[i] = 0;
    }
    for(i = bqp->nVars; i--;) {
        if(solution[i] == 1) {
            cost += bqpUtil_getChangeInObjective(bqp, zeroSolution, i);
            zeroSolution[i] = 1;
        }
    }
    return cost;
}

double bqpUtil_getObjectiveIncremental(BQP *bqp, int *solution, int *oldSolution, double oldCost) {
    int i;
    double cost = oldCost;
    vector<int> u_old(bqp->nVars);
    int *oldSolCopy = vector_data<int>(u_old);
    for(i = 0; i < bqp->nVars; i++) {
        oldSolCopy[i] = oldSolution[i];
    }
    for(i = bqp->nVars; i--;) {
        if(solution[i] != oldSolCopy[i]) {
            cost += bqpUtil_getChangeInObjective(bqp, oldSolCopy, i);
            oldSolCopy[i] = solution[i];
        }
    }
    return cost;
}

void bqpUtil_initBQPSolution(BQP *bqp, const int *initSolution) {
    int i;

    if (initSolution == NULL) {
      for(i = 0; i < bqp->nVars; i++) {
          bqp->solution[i] = 0;
      }
    } else {
      memcpy(vector_data<int>(bqp->solution), initSolution, bqp->nVars * sizeof(int));
    }
    bqp->solutionQuality = bqpUtil_getObjective(bqp, vector_data<int>(bqp->solution));
    bqp->nIterations = 1;
}

void bqpUtil_randomizeBQPSolution(BQP *bqp) {
    int i;
    for(i = 0; i < bqp->nVars; i++) {
        if((rand() / (float)RAND_MAX) < 0.5) {
            bqp->solution[i] = 0;
        }
        else {
            bqp->solution[i] = 1;
        }
    }
    bqp->nIterations = 1;
    bqp->solutionQuality = bqpUtil_getObjective(bqp, vector_data<int>(bqp->solution));
}

void bqpUtil_printSolution(BQP *bqp) {
    int i;
    printf("Objective function value: %ld\n", bqpUtil_getObjective(bqp, vector_data<int>(bqp->solution)));
    printf("Variable assignment:\n");
    for(i = 0; i < bqp->nVars; i++) {
        printf("%d ", bqp->solution[i]);
    }
    printf("\n");
}
