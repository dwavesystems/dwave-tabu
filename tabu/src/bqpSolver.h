#ifndef _BQPSOLVER_H_

#define _BQPSOLVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "bqpUtil.h"

#define LAMBDA 5000
#define ALPHA 0.4

#define LARGE_NUMBER 999999999


typedef struct bqpSolver_Callback {
  void (*func)(const struct bqpSolver_Callback *callback, BQP *bqp);
  void *context;
} bqpSolver_Callback;

/**
 * Solves a BQP using simple taboo search heuristic
 * \param bqp: BQP problem to be solved
 * \param starting: A starting solution
 * \param ZCoeff: A parameter that defines the number of iterations
 * \param timLimitInMilliSecs: time limit in milli seconds
 * \return best solution found
 */
long bqpSolver_tabooSearch(BQP *bqp, int *starting, long startingObjective, int tt, long long ZCoeff, long long timeLimitInMilliSecs, const bqpSolver_Callback *callback);

/**
 * Solves a BQP using basic local searching
 * \param bqp: BQP problem to be solved
 * \param starting: A starting solution
 * \param startingObjective: The objective function value for the starting solution
 * \param changeInObjective: Partial derivative values for the starting solution
 * \return best solution found
 */
long bqpSolver_localSearchInternal(BQP *bqp, int *starting, long startingObjective, long *changeInObjective);

/**
 * Solves a BQP using basic local searching (wrapper for localSearchInternal)
 * A wrapper to the bqpSolver_localSearchInternal() function
 * \param bqp: BQP problem to be solved
 * \param starting: A starting solution
 * \return best solution found
 */
long bqpSolver_localSearch(BQP *bqp, int *starting);

/**
 * Helper function to bqpSolver_multiStartTabooSearch() function
 * Selects variables to change and build up new solution
 * \param bqp: BQP problem to be solved
 * \param n: number of variables required to be selected
 * \param C: C matrix (refer paper for multi start taboo search by Paulubekis)
 * \param I: storage for selected variables
 * \return 
 */
void bqpSolver_selectVariables(BQP *bqp, int n, long **C, int *I);

/**
 * Helper function to bqpSolver_multiStartTabooSearch() function
 * Uses steepest ascent to construct a new solution
 * \param solution: Old solution
 * \param bqp: BQP problem to be solved
 * \param C: C matrix (refer paper for multi start taboo search by Paulubekis)
 * \param I: selected variables
 * \param n: number of variables required to be selected
 * \return 
 */
void bqpSolver_steepestAscent(int *solution, BQP *bqp, long **C, int *I, int n);

/**
 * Compute the C matrix (refer to the taboo search heuristic in the paper by palubekis
 * \param C: the matrix computed
 * \param bqp: the BQP
 * \param solution: current solution
 * \return void
 */
void bqpSolver_computeC(long **C, BQP *bqp, int *solution);

/**
 * Simple taboo search solver with multi starts
 * \param bqp: BQP problem to be solved
 * \param timLimitInMilliSecs: time limit in milli seconds
 * \param numStarts: number of re starts
 * \return best solution found
 */
long bqpSolver_multiStartTabooSearch(BQP *bqp, long long timeLimitInMilliSecs, int numStarts, int tabuTenure, const int *initSolution, const bqpSolver_Callback *callback);

/**
 * Exhaustive solver (takes too long for problems with more than 20 variables)
 * \param bqp: BQP problem to be solved
 * \return 
 */
int bqpSolver_naiveSearch(BQP *bqp);

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
long bqpSolver_restrictedLocalSearchInternal(BQP *bqp, int *starting, int *restricted, long startingObjective, long *changeInObjective);

/**
 * Another version to bqpSolver_localSearch() function
 * A wrapper to the bqpSolver_restrictedLocalSearchInternal() function
 * \param bqp: BQP problem to be solved
 * \param starting: A starting solution
 * \param restricted: Tells if a variable is restricted or not
 * \return best solution found
 */
long bqpSolver_restrictedLocalSearch(BQP *bqp, int *starting, int *restricted);




#endif
