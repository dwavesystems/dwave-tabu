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

#ifndef _BQP_H_

#define _BQP_H_

#include <vector>

class BQP 
{
    public:
        BQP(std::vector<std::vector<double>> Q);

        /**
         * Calls toUpperTriangular() and sets the solution
         * @return void
         */
        void initialize(const std::vector<int> &initSolution);

        /**
         * Converts a generic bqp Q matrix into upper triangular using:
         * Q[i][j] = Q[i][j] + Q[j][i] for all i < j
         * @return void
         */
        void toUpperTriangular();

        /**
         * Computes the value by which the objective function is changed if
         * exactly one bit in the solution is flipped
         * @param oldSolution: Current solution
         * @param flippedBit: The bit that is flipped
         * @return Change in objective
         */
        double getChangeInObjective(const std::vector<int> &oldSolution, int flippedBit);

        /**
         * Computes the value of objective function for a given solution
         * @param solution: Given solution
         * @return Value of objective function
         */
        double getObjective(const std::vector<int> &solution);
        
        /**
         * Gets the maximum abs(Q[i][j])
         * @return Maximum Q[i][j]
         */
        double getMaxBQPCoeff();

        /**
         * Prints Q matrix
         * @return void
         */
        void printQ();

        /**
         * Prints the current solution
         * @return void
         */
        void printSolution();

        std::vector<std::vector<double> > Q;    // Q matrix
        int nVars;                              // Number of problem variables
        std::vector<int> solution;              // Current solution, vector of size nVars where every entry is 0 or 1
        double solutionQuality;                 // Objective function value at solution
        unsigned long long nIterations;         // Number of iterations required to arrive at solution

        /*added to record more statistics.*/
        unsigned long long restartNum;  // Number of times simpleTabuSearch runs
        unsigned long long iterNum;     // Number of times loop within simpleTabuSearch runs
        unsigned long long evalNum;
        double upperBound;
};

#endif
