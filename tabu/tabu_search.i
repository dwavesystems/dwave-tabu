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
%module tabu_search

%{
#include "src/tabu_search.h"
#include "src/common.h"
%}

%include "stdint.i"
%include "std_vector.i"

namespace std {
    %template(IntVector)  vector<int>;
    %template(DoubleVector) vector<double>;
    %template(IntArray) vector< vector<int> >;
    %template(DoubleArray) vector< vector<double> >;
}

%define TABU_DOCSTRING
"Tabu Search

handler = TabuSearch(q, init_solution, tenure, scaleFactor, timeout)

    Args:
        q: QUBO as a list of list, or numpy matrix of double (float64) values.
           q must be symmetric.
        init_solution: List of 0/1 values, which defines the initial state of each variable.
        tenure: Tabu tenure. min(20, num_vars / 4) seems to be a good choice.
        scaleFactor: Scaling factor for elements of q. The elements of q are stored as long ints using
                     internal_q = long int (q * scaleFactor).
        timeout: Total running time in milliseconds.

energy = handler.bestEnergy()

solution = handler.bestSolution()"

%enddef

%feature("autodoc", TABU_DOCSTRING) TabuSearch;

%exception {
    try {
        $action
    }
    catch (Exception e) {
            PyErr_SetString(PyExc_RuntimeError, e.what());
            return NULL;
    }
}

%include "src/tabu_search.h"
%include "src/common.h"
