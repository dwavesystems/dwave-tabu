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

#include "bqpUtil.cpp"

using std::vector;

TEST_CASE("Test bqpUtil_initBQPSolution") {
    vector<vector<double> > Q {{2,1,1},
                               {1,2,1},
                               {1,1,2}};

    BQP bqp;
    bqp.Q = Q;
    bqp.nVars = 3;

    vector<int> solution = {1, 1, 1};
    bqpUtil_initBQPSolution(&bqp, solution);
    
    REQUIRE(bqp.solution == solution);
    REQUIRE(bqp.nIterations == 1);
    REQUIRE(bqp.solutionQuality == 12);
}

TEST_CASE("Testing bqpUtil_convertBQPToUpperTriangular()") {
    vector<vector<double> > Q {{2.3, 1.1, -1.5},
                               {1.1, 2.0, 2.4},
                               {1.0, 1.4, 2.1}};

    BQP bqp;
    bqp.Q = Q;
    bqp.nVars = 3;

    auto orig_Q = bqp.Q;

    bqpUtil_convertBQPToUpperTriangular(&bqp);

    for (int i = 0; i < bqp.nVars; i++) {
        for (int j = i + 1; j < bqp.nVars; j++) {
            REQUIRE(bqp.Q[i][j] == orig_Q[i][j] + orig_Q[j][i]);
            REQUIRE(bqp.Q[j][i] == 0);
        }
    }
}

TEST_CASE("Testing bqpUtil_getObjective() and bqpUtil_getChangeInObjective()") {
    vector<vector<double>> Q = {{1, -2},
                                {0, 1}};    // equality problem
    BQP bqp;
    bqp.Q = Q;
    bqp.nVars = 2;

    vector<int> solution = {0, 0};
    REQUIRE(bqpUtil_getObjective(&bqp, solution) == 0);

    vector<int> new_solution = {0, 1};
    REQUIRE(bqpUtil_getObjective(&bqp, new_solution) == 1);

    REQUIRE(bqpUtil_getChangeInObjective(&bqp, solution, 1) == 1);
    REQUIRE(bqpUtil_getChangeInObjective(&bqp, new_solution, 1) == -1);
}

TEST_CASE("Testing bqpUtil_getMaxBQPCoeff()") {
    vector<vector<double> > Q {{2, 1, -1},
                               {3, -2, 1},
                               {-1, 1, 2}};

    BQP bqp;
    bqp.Q = Q;
    bqp.nVars = 3;

    REQUIRE(bqpUtil_getMaxBQPCoeff(&bqp) == 3);  
}
