# distutils: language = c++
# distutils: include_dirs = tabu/src/
# distutils: sources = tabu/src/tabu_search.cc

# Copyright 2020 D-Wave Systems Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from libcpp.vector cimport vector
cimport tabu

cdef class TabuSearch:
    """Wraps the class `TabuSearch` from `tabu_search.cc`."""
    cdef tabu.TabuSearch *c_tabu

    def __cinit__(self, vector[vector[double]] Q, vector[int] initSol, int tenure, long int timeout):
        self.c_tabu = new tabu.TabuSearch(Q, initSol, tenure, timeout)

    def __dealloc__(self):
        del self.c_tabu

    def bestEnergy(self):
        return self.c_tabu.bestEnergy()

    def bestSolution(self):
        return self.c_tabu.bestSolution()