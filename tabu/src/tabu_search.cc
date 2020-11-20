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

#include "tabu_search.h"

#include <limits>

#include "common.h"
#include "bqpSolver.h"

using std::vector;
using std::size_t;

TabuSearch::TabuSearch(vector<vector<double> > Q, vector<int> initSol, int tenure, long int timeout)
{
    size_t nvars = Q.size();
    for (int i = 0; i < nvars; i++){
        if (Q[i].size() != nvars)
            throw Exception("Q must be a symmetric square matrix");
    }

    if (initSol.size() != nvars)
        throw Exception("length of init_solution doesn't match the size of Q");

    if (tenure < 0 || tenure > (nvars - 1))
        throw Exception("tenure must be in the range [0, num_vars - 1]");

    bqp.evalNum = 0;
    bqp.iterNum = 0;
    bqp.nIterations = 0;
    bqp.nVars = nvars;
    bqp.restartNum = 0;
    bqp.Q.resize(nvars);
    bqp.upperBound = -std::numeric_limits<double>::max();
    for (int i = 0; i < nvars; i++)
        bqp.Q[i].resize(nvars);

    for (int i = 0; i < nvars; i++){
        for (int j = i; j < nvars; j++){
            if (Q[i][j] != Q[j][i])
                throw Exception("Q must be symmetric");
            bqp.Q[i][j] = bqp.Q[j][i] = Q[i][j];
        }
    }
    bqp.solution.resize(nvars);
    bqp.solutionQuality = 0;
    bqpSolver_multiStartTabooSearch(&bqp, timeout, 1000000, tenure, initSol, nullptr);
}


double TabuSearch::bestEnergy()
{
    return bqpUtil_getObjective(&bqp, bqp.solution);
}

vector<int> TabuSearch::bestSolution()
{
    return bqp.solution;
}
