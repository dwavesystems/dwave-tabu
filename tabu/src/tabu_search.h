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

#ifndef __TABU_SEARCH_H__
#define __TABU_SEARCH_H__

#define LAMBDA 5000
#define ALPHA 0.4

#include <vector>
#include <random>

#include "bqp.h"

typedef struct bqpSolver_Callback {
  void (*func)(const struct bqpSolver_Callback *callback, BQP *bqp);
  void *context;
} bqpSolver_Callback;

class TabuSearch
{
    public:
        TabuSearch(std::vector<std::vector<double>> Q, 
                   const std::vector<int> initSol, 
                   int tenure, 
                   long int timeout, 
                   int numRestarts, 
                   unsigned int seed, 
                   double energyThreshold);
        double bestEnergy();
        std::vector<int> bestSolution();
        int numRestarts();

    private:
        /**
         * Simple tabu search solver with multi starts. Updates bqp with best solution found.
         * \param timeLimitInMilliSecs: Time limit in milliseconds
         * \param numStarts: Number of re starts
         * \param energyThreshold: Search terminates when energy lower than threshold is found
         * \param initSolution: Starting solution to start search from
         * \param callback: Optional callback function
         * \return
         */
        void multiStartTabuSearch(long long timeLimitInMilliSecs, 
                                  int numStarts, 
                                  double energyThreshold,
                                  const std::vector<int> &initSolution, 
                                  const bqpSolver_Callback *callback);

        /**
         * Solves and updates the BQP using simple tabu search heuristic
         * \param starting: A starting solution
         * \param startingObjective: The objective function value for the starting solution
         * \param ZCoeff: Parameter used to define the max number of iterations
         * \param timeLimitInMilliSecs: Time limit in milli seconds
         * \param useTimeLimit: If false, timeLimitInMilliSecs is ignored
         * \param energyThreshold: Search terminates when energy lower than threshold is found
         * \param callback: Optional callback function
         * \return
         */
        void simpleTabuSearch(const std::vector<int> &starting, 
                              double startingObjective, 
                              long long ZCoeff, 
                              long long timeLimitInMilliSecs, 
                              bool useTimeLimit, 
                              double energyThreshold,
                              const bqpSolver_Callback *callback);

        /**
         * Solves and updates the BQP using basic local searching
         * \param starting: A starting solution
         * \param startingObjective: The objective function value for the starting solution
         * \param changeInObjective: Partial derivative values for the starting solution
         * \return
         */
        void localSearchInternal(const std::vector<int> &starting, 
                                 double startingObjective, 
                                 std::vector<double> &changeInObjective);
        
        /**
         * Helper function to multiStartTabuSearch() function
         * Selects variables to change and build up new solution
         * \param numSelection: Number of variables required to be selected
         * \param C: C matrix (refer paper for multi start tabu search by Palubeckis)
         * \param I: Storage for selected variables
         * \return
         */
        void selectVariables(int numSelection, 
                             std::vector<std::vector<double>> &C, 
                             std::vector<int> &I);
        
        /**
         * Helper function to multiStartTabuSearch() function
         * Uses steepest ascent to construct a new solution
         * \param numSelection: Number of variables required to be selected
         * \param C: C matrix (refer paper for multi start tabu search by Palubeckis)
         * \param I: Selected variables
         * \param solution: Solution to be updated
         * \return
         */
        void steepestAscent(int numSelection, 
                            std::vector<std::vector<double>> &C, 
                            std::vector<int> &I, 
                            std::vector<int> &solution);

        /**
         * Compute the C matrix (refer to the tabu search heuristic in the paper by Palubeckis (p.262))
         * \param C: The matrix computed
         * \param solution: Current solution
         * \return
         */
        void computeC(std::vector<std::vector<double>> &C, const std::vector<int> &solution);

        /**
         * Stores the problem, the solution, and some statistics
         */
        BQP bqp;

        /**
         * Number of previous solutions to keep track of
         */
        int tabooTenure;

        /**
         * RNG
         */
        std::default_random_engine generator;
};

#endif
