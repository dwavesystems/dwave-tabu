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

TabuSearch::TabuSearch(vector<vector<double>> Q, vector<int> initSol, int tenure, long int timeout) : bqp(Q)
{
    size_t nvars = Q.size();
    if (initSol.size() != nvars)
        throw Exception("length of init_solution doesn't match the size of Q");

    if (tenure < 0 || tenure > (nvars - 1))
        throw Exception("tenure must be in the range [0, num_vars - 1]");

    // Solve and update bqp
    BQPSolver solver = BQPSolver(bqp);
    bqp = solver.multiStartTabuSearch(timeout, 1000000, tenure, initSol, nullptr);
}

double TabuSearch::bestEnergy()
{
    return bqp.getObjective(bqp.solution);
}

vector<int> TabuSearch::bestSolution()
{
    return bqp.solution;
}
