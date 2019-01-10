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

#ifndef _BQPSOLVER_H_

#define _BQPSOLVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <vector>
#include "bqpUtil.h"

#define LAMBDA 5000
#define ALPHA 0.4


typedef struct bqpSolver_Callback {
  void (*func)(const struct bqpSolver_Callback *callback, BQP *bqp);
  void *context;
} bqpSolver_Callback;

/**
 * Solves a BQP using simple tabu search heuristic
 * \param bqp: BQP problem to be solved
 * \param starting: A starting solution
 * \param ZCoeff: A parameter that defines the number of iterations
 * \param timLimitInMilliSecs: time limit in milli seconds
 * \return best solution found
 */
double bqpSolver_tabooSearch(BQP *bqp, std::vector<int> &starting, double startingObjective, int tt, long long ZCoeff, long long timeLimitInMilliSecs, const bqpSolver_Callback *callback);

/**
 * Solves a BQP using basic local searching
 * \param bqp: BQP problem to be solved
 * \param starting: A starting solution
 * \param startingObjective: The objective function value for the starting solution
 * \param changeInObjective: Partial derivative values for the starting solution
 * \return best solution found
 */
double bqpSolver_localSearchInternal(BQP *bqp, std::vector<int> &starting, double startingObjective, std::vector<double> &changeInObjective);

/**
 * Solves a BQP using basic local searching (wrapper for localSearchInternal)
 * A wrapper to the bqpSolver_localSearchInternal() function
 * \param bqp: BQP problem to be solved
 * \param starting: A starting solution
 * \return best solution found
 */
double bqpSolver_localSearch(BQP *bqp, std::vector<int> &starting);

/**
 * Helper function to bqpSolver_multiStartTabooSearch() function
 * Selects variables to change and build up new solution
 * \param bqp: BQP problem to be solved
 * \param n: number of variables required to be selected
 * \param C: C matrix (refer paper for multi start tabu search by Palubeckis)
 * \param I: storage for selected variables
 * \return 
 */
void bqpSolver_selectVariables(BQP *bqp, int n, std::vector< std::vector<double> > &C, std::vector<int> &I);

/**
 * Helper function to bqpSolver_multiStartTabooSearch() function
 * Uses steepest ascent to construct a new solution
 * \param solution: Old solution
 * \param bqp: BQP problem to be solved
 * \param C: C matrix (refer paper for multi start tabu search by Palubeckis)
 * \param I: selected variables
 * \param n: number of variables required to be selected
 * \return 
 */
void bqpSolver_steepestAscent(std::vector<int> &solution, BQP *bqp, std::vector< std::vector<double> > &C, std::vector<int> &I, int n);

/**
 * Compute the C matrix (refer to the tabu search heuristic in the paper by Palubeckis
 * \param C: the matrix computed
 * \param bqp: the BQP
 * \param solution: current solution
 * \return void
 */
void bqpSolver_computeC(std::vector< std::vector<double> > &C, BQP *bqp, std::vector<int> &solution);

/**
 * Simple tabu search solver with multi starts
 * \param bqp: BQP problem to be solved
 * \param timLimitInMilliSecs: time limit in milli seconds
 * \param numStarts: number of re starts
 * \return best solution found
 */
double bqpSolver_multiStartTabooSearch(BQP *bqp, long long timeLimitInMilliSecs, int numStarts, int tabuTenure, std::vector<int> const &initSolution, const bqpSolver_Callback *callback);

/**
 * Exhaustive solver (takes too long for problems with more than 20 variables)
 * \param bqp: BQP problem to be solved
 * \return 
 */
double bqpSolver_naiveSearch(BQP *bqp);

/**
 * Another version for bqpSolver_localSearchInternal() function
 * In this version some variables are restricted and you are not allowed to flip them
 * \param bqp: BQP problem to be solved
 * \param starting: A starting solution
 * \param restricted: Tells if a variable is restricted or not
 * \param startingObjective: The objective function value for the starting solution
 * \param changeInObjective: Partial derivative values for the starting solution
 * \return best solution found
 */
double bqpSolver_restrictedLocalSearchInternal(BQP *bqp, std::vector<int> &starting, std::vector<int> &restricted, double startingObjective, std::vector<double> &changeInObjective);

/**
 * Another version to bqpSolver_localSearch() function
 * A wrapper to the bqpSolver_restrictedLocalSearchInternal() function
 * \param bqp: BQP problem to be solved
 * \param starting: A starting solution
 * \param restricted: Tells if a variable is restricted or not
 * \return best solution found
 */
double bqpSolver_restrictedLocalSearch(BQP *bqp, std::vector<int> &starting, std::vector<int> &restricted);




#endif
