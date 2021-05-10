# distutils: language = c++
# distutils: include_dirs = tabu/src/
# distutils: sources = tabu/src/tabu_search.cpp

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
from libc.time cimport time
import numpy as np

cimport tabu


cdef class TabuSearch:
    """Wraps the class `TabuSearch` from `src/tabu_search.cpp`."""

    cdef tabu.TabuSearch *c_tabu

    def __cinit__(self,
                  object Q,
                  object initSol,
                  int tenure,
                  int timeout,
                  int numRestarts,
                  object seed=None,
                  object energyThreshold=None):
        cdef unsigned int _seed = time(NULL) if seed is None else seed
        cdef double _energyThreshold = -np.inf if energyThreshold is None else energyThreshold

        cdef double[:,:] qubo = np.asarray(Q, dtype=np.double)
        cdef vector[vector[double]] Qvec
        Qvec.resize(qubo.shape[0])
        cdef Py_ssize_t i, j
        for i in range(qubo.shape[0]):
            for j in range(qubo.shape[1]):
                Qvec[i].push_back(qubo[i, j])

        cdef int[:] initial = np.asarray(initSol, dtype=np.intc)
        cdef vector[int] initVec
        for i in range(len(initial)):
            initVec.push_back(initial[i])

        with nogil:
            self.c_tabu = new tabu.TabuSearch(
                Qvec, initVec, tenure, timeout, numRestarts, _seed, _energyThreshold)

    def __dealloc__(self):
        del self.c_tabu

    def bestEnergy(self):
        return self.c_tabu.bestEnergy()

    def bestSolution(self):
        return self.c_tabu.bestSolution()

    def numRestarts(self):
        return self.c_tabu.numRestarts()
