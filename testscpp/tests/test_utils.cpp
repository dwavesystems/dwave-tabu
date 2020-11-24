// Copyright 2020 D-Wave Systems Inc.
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.

#include "../Catch2/single_include/catch2/catch.hpp"

#include <vector>

#include "bqp.cpp"

using std::vector;

TEST_CASE("Test BQP::initialize()") {
    vector<vector<double> > Q {{2,1,1},
                               {1,2,1},
                               {1,1,2}};

    BQP bqp = BQP(Q);

    vector<int> solution = {1, 1, 1};
    bqp.initialize(solution);

    // Check that Q matrix was converted to upper triangular
    for (int i = 0; i < bqp.nVars; i++) {
        for (int j = i + 1; j < bqp.nVars; j++) {
            REQUIRE(bqp.Q[i][j] == Q[i][j] + Q[j][i]);
            REQUIRE(bqp.Q[j][i] == 0);
        }
    }
    
    // Check that solution was set
    REQUIRE(bqp.solution == solution);
    REQUIRE(bqp.solutionQuality == 12);
    REQUIRE(bqp.nIterations == 1);
}

TEST_CASE("Testing BQP::getObjective() and BQP::getChangeInObjective()") {
    vector<vector<double>> Q = {{1, -1},
                                {-1, 1}};    // equality problem
    BQP bqp = BQP(Q);

    vector<int> solution = {0, 0};
    REQUIRE(bqp.getObjective(solution) == 0);

    vector<int> new_solution = {0, 1};
    REQUIRE(bqp.getObjective(new_solution) == 1);

    REQUIRE(bqp.getChangeInObjective(solution, 1) == 1);
    REQUIRE(bqp.getChangeInObjective(new_solution, 1) == -1);
}

TEST_CASE("Testing BQP::getMaxBQPCoeff()") {
    vector<vector<double> > Q {{2, 3, -1},
                               {3, -2, 1},
                               {-1, 1, 2}};

    BQP bqp = BQP(Q);
    REQUIRE(bqp.getMaxBQPCoeff() == 3);  
}
