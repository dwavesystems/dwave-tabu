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
#ifndef _BQPUTIL_H_

#define _BQPUTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <vector>

/**
 * structure to represent a BQP
 * Q: the Q matrix
 * nVars: number of variables
 * solution: array fo size nVars where every entry is 0 or 1. current solution.
 * solutionQuality: objective function value at solution
 * nIterations: number of iterations required to arrive at this solution
 */
typedef struct {
	std::vector<std::vector<long> >Q;
	int nVars;
	std::vector<int> solution;
	long solutionQuality;
	unsigned long long nIterations;
	/*added to record more statistics.*/
	unsigned long long restartNum;
	unsigned long long iterNum;
	unsigned long long evalNum;
	long upperBound;
} BQP;


/**
 * gets the maximum abs(Q[i][j]) in a bqp
 * @param bqp
 * @return maximum Q[i][j]
 */
long bqpUtil_getMaxBQPCoeff(BQP *bqp);

/**
 * converts a generic bqp Q matrix into upper trianglular using:
 * Q[i][j] = Q[i][j] + Q[j][i] for all i < j
 * @param bqp
 * @return void
 */
void bqpUtil_convertBQPToUpperTriangular(BQP *bqp);

/**
 * Read BQP from file. The file must be in matrix format, that is
 * it must contain the whole BQP matrix unlike the other format in
 * which, only the non-zero entries of the matrix are specified
 * @param bqp: The BQP read fromt he file
 * @param fileName: BQP file name
 * @return void
 */
void bqpUtil_readMatrixTypeBQPFile(BQP *bqp, char* fileName);

/**
 * Read BQP from file. The file nust be in list format, that is
 * it must contain only the non-zero entries of the BQP matrix
 * unlike the other format in which the complete matrix is specified
 * @param bqp: The BQP read fromt he file
 * @param fileName: BQP file name
 * @return void
 */
void bqpUtil_readListTypeBQPFile(BQP *bqp, char* fileName);

/**
 * Print a BQP
 * @param bqp: BQP to be printed
 * @return void
 */
void bqpUtil_print(BQP *bqp);

/**
 * Compute the value by which the ovjective function is changed if
 * exactly one bit in the solution is flipped
 * @param bqp: the BQP
 * @param oldSolution: current solution
 * @param flippedBit: the bit that is flipped
 * @return change in objective
 */
long bqpUtil_getChangeInObjective(BQP *bqp, int *oldSolution, int flippedBit);

/**
 * Computes the value of objective function of a BQP for a given solution
 * @param bqp: the BQP
 * @param solution: given solution
 * @return value of objective function
 */
long bqpUtil_getObjective(BQP *bqp, int * solution);

/**
 * Computes the value of objective function of a BQP for a given solution,
 * given some old solution.
 * If an old solution and the objective function value for that old soltuion
 * is known, then we can calculate the objective for the new solution quicker
 * by building up on the old soltuion.
 * @param bqp: the BQP
 * @param solution: new solution
 * @param oldSolution: old solution
 * @param oldCost: value of objective function at old solution
 * @return value ot objective function at new solution
 */
long bqpUtil_getObjectiveIncremental(BQP *bqp, int *solution, int *oldSolution, long oldCost);

/**
 * Generates a random BQP.
 * @param bqp: the BQP generated
 * @param nVars: number of variables
 * @param maxCoeff: all entries in the Q matrix will be between 0 and maxCoeff
 * @return void
 */
void bqpUtil_getRandomBQP(BQP *bqp, int nVars, long maxCoeff);

/**
 * Initialize the current solution in a BQP to all zeros
 * @param bqp: the BQP
 * @return void
 */
void bqpUtil_initBQPSolution(BQP *bqp, const int *initSolution);

/**
 * Initialize the current solution in a BQP randomly
 * @param bqp: the BQP
 * @return void
 */
void bqpUtil_randomizeBQPSolution(BQP *bqp);

/**
 * Free BQP. De-allocate Q and solution matrices
 * @param bqp: the BQP
 * @return void
 */
void bqpUtil_free(BQP *bqp);

/**
 * Print the current solution of BQP
 * @param bqp: the BQP
 * @return void
 */
void bqpUtil_printSolution(BQP *bqp);


#endif
