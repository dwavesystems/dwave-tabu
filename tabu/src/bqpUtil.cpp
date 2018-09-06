// Copyright 2018 D-Wave Systems Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// ===========================================================================
#include <stdlib.h>
#include <vector>

#include "bqpUtil.h"

using std::vector;

long bqpUtil_getMaxBQPCoeff(BQP *bqp) {
    int i, j;
    long M = bqp->Q[0][0];
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

// void bqpUtil_readMatrixTypeBQPFile(BQP *bqp, char* fileName) {
//     int i, junk;
//     char line[1000];
//     int row = 0, col = 0;
//
//     FILE *fptr = fopen(fileName, "r");
//     if(fptr == NULL) {
//         printf("ERROR openning file");
//         return;
//     }
//     fgets(line, 1000, fptr);
//     fscanf(fptr, "  %d %d", &(bqp->nVars), &junk);
//     bqp->Q = (long **)malloc(sizeof(long *) * bqp->nVars);
//     bqp->solution = (int *)malloc(sizeof(int) * bqp->nVars);
//     for(i = 0; i < bqp->nVars; i++) {
//       bqp->Q[i] = (long *)malloc(sizeof(long) * bqp->nVars);
//         bqp->solution[i] = 0;
//     }
//     bqp->solutionQuality = 0;
//     bqp->nIterations = 1;
//
//     while(!feof(fptr)) {
//       fscanf(fptr, "%ld", &(bqp->Q[row][col]));
//         col++;
//       if(col >= bqp->nVars) {
//         col = 0;
//         row++;
//       }
//       if(row >= bqp->nVars) {
//         break;
//       }
//     }
//     fclose(fptr);
// }

// void bqpUtil_readListTypeBQPFile(BQP *bqp, char* fileName) {
//     int i, j, numOfProblems, numCoeffs, id1, id2, coeff;
//     char line[1000];
//     FILE *fptr = fopen(fileName, "r");
//     if(fptr == NULL) {
//         printf("ERROR openning file");
//         return;
//     }
//     fgets(line, 1000, fptr);
//     sscanf(line, " %d", &numOfProblems);
//     if(numOfProblems < 1) {
//         fclose(fptr);
//         return;
//     }
//     fgets(line, 1000, fptr);
//     sscanf(line, "  %d %d", &(bqp->nVars), &numCoeffs);
//     bqp->Q = (long **)malloc(sizeof(long *) * bqp->nVars);
//     bqp->solution = (int *)malloc(sizeof(int) * bqp->nVars);
//     for(i = 0; i < bqp->nVars; i++) {
//       bqp->Q[i] = (long *)malloc(sizeof(long) * bqp->nVars);
//         bqp->solution[i] = 0;
//         for(j = 0; j < bqp->nVars; j++) {
//             bqp->Q[i][j] = 0;
//         }
//     }
//     bqp->solutionQuality = 0;
//     bqp->nIterations = 1;
//
//     for(i = 0; i < numCoeffs && !feof(fptr); i++) {
//         fgets(line, 1000, fptr);
//         sscanf(line, " %d %d %d", &id1, &id2, &coeff);
//         bqp->Q[id1 - 1][id2 - 1] = -coeff;
//         bqp->Q[id2 - 1][id1 - 1] = -coeff;
//     }
//     fclose(fptr);
// }

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

long bqpUtil_getChangeInObjective(BQP *bqp, int *oldSolution, int flippedBit) {
    int i;
    long change = 0;
    change += (oldSolution[flippedBit] == 1)? (-1 * bqp->Q[flippedBit][flippedBit]) : bqp->Q[flippedBit][flippedBit];
    for(i = bqp->nVars; i--;) {
        if(!(oldSolution[i] ^ 1) && i ^ flippedBit) {
            change += (oldSolution[flippedBit] ^ 1)? (bqp->Q[flippedBit][i] + bqp->Q[i][flippedBit]) :
                                                                                                    -(bqp->Q[flippedBit][i] + bqp->Q[i][flippedBit]);
        }
    }
    return change;
}

long bqpUtil_getObjective(BQP *bqp, int * solution) {
    int i;
    long cost = 0;
    vector<int> u_zeroSol(bqp->nVars);
    int *zeroSolution = u_zeroSol.data();
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

long bqpUtil_getObjectiveIncremental(BQP *bqp, int *solution, int *oldSolution, long oldCost) {
    int i;
    long cost = oldCost;
    vector<int> u_old(bqp->nVars);
    int *oldSolCopy = u_old.data();
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

// void bqpUtil_getRandomBQP(BQP *bqp, int nVars, long maxCoeff) {
//     int i, j;
//     if(bqp == NULL) {
//         bqp = (BQP *)malloc(sizeof(BQP));
//     }
//     bqp->Q = (long **)malloc(sizeof(long *) * nVars);
//     for(i = 0; i < nVars; i++) {
//         bqp->Q[i] = (long *)malloc(sizeof(long) * nVars);
//     }
//     bqp->solution = (int *)malloc(sizeof(int) * nVars);
//     for(i = 0; i < nVars; i++) {
//         for(j = i; j < nVars; j++) {
//             bqp->Q[i][j] = 0;
//             if(rand() < RAND_MAX / 2.0) {
//                 bqp->Q[i][j] = (long)(rand() % maxCoeff);
//             }
//             if(rand() < RAND_MAX / 2.0) {
//                 bqp->Q[i][j] = -bqp->Q[i][j];
//             }
//             bqp->Q[j][i] = bqp->Q[i][j];
//         }
//         bqp->solution[i] = 0;
//     }
//     bqp->nVars = nVars;
//     bqp->solutionQuality = 0;
//     bqp->nIterations = 0;
//     /*added to record more statistics.*/
//     bqp->restartNum = 0;
//     bqp->iterNum = 0;
//     bqp->evalNum = 0;
// }

void bqpUtil_initBQPSolution(BQP *bqp, const int *initSolution) {
    int i;

    if (initSolution == NULL) {
      for(i = 0; i < bqp->nVars; i++) {
          bqp->solution[i] = 0;
      }
    } else {
      memcpy(bqp->solution.data(), initSolution, bqp->nVars * sizeof(int));
    }
    bqp->solutionQuality = bqpUtil_getObjective(bqp, bqp->solution.data());
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
    bqp->solutionQuality = bqpUtil_getObjective(bqp, bqp->solution.data());
}

void bqpUtil_printSolution(BQP *bqp) {
    int i;
    printf("Objective function value: %ld\n", bqpUtil_getObjective(bqp, bqp->solution.data()));
    printf("Variable assignment:\n");
    for(i = 0; i < bqp->nVars; i++) {
        printf("%d ", bqp->solution[i]);
    }
    printf("\n");
}



