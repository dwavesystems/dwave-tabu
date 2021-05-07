# Copyright 2019 D-Wave Systems Inc.
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

"""Test the (private) TabuSearch python interface."""

import time
import unittest
from concurrent.futures import ThreadPoolExecutor, wait

import dimod
import tabu
import numpy as np
from hybrid.testing import RunTimeAssertionMixin


class TestTabuSearch(unittest.TestCase, RunTimeAssertionMixin):

    def test_trivial(self):
        qubo = [[1.0]]
        init = [1]
        tenure = len(init) - 1
        timeout = 1
        restarts = 100

        search = tabu.TabuSearch(qubo, init, tenure, timeout, restarts)

        solution = list(search.bestSolution())
        energy = search.bestEnergy()

        self.assertEqual(solution, [0])
        self.assertEqual(energy, 0.0)

    def test_correctness(self):
        qubo = [[-1.2, 1.1], [1.1, -1.2]]
        init = [1, 1]
        tenure = len(init) - 1
        timeout = 20
        restarts = 100

        search = tabu.TabuSearch(qubo, init, tenure, timeout, restarts)

        solution = list(search.bestSolution())
        energy = search.bestEnergy()

        self.assertEqual(solution, [0, 1])
        self.assertEqual(energy, -1.2)

    def test_concurrency(self):

        def search(timeout, restarts=int(1e6)):
            return tabu.TabuSearch([[1.0]], [1], 0, timeout, restarts).bestEnergy()

        with ThreadPoolExecutor(max_workers=3) as executor:

            # ~ 500 ms (but be gracious on slow CI VMs)
            with self.assertRuntimeWithin(400, 1600):
                wait([executor.submit(search, timeout=500) for _ in range(3)])

            # ~ 1000 ms (but be gracious on slow CI VMs)
            with self.assertRuntimeWithin(900, 2100):
                wait([executor.submit(search, timeout=500) for _ in range(4)])

    def test_float(self):
        n = 20
        init = [1] * n
        tenure = len(init) - 1
        timeout = 20
        restarts = 100

        bqm = dimod.generators.random.uniform(n, 'BINARY', low=-100, high=100, seed=123)
        Q, _ = tabu.TabuSampler._bqm_to_tabu_qubo(bqm)
        search = tabu.TabuSearch(Q, init, tenure, timeout, restarts)
        self.assertAlmostEqual(search.bestEnergy(), -1465.9867898)

        bqm = dimod.generators.random.uniform(n, 'BINARY', low=-1, high=1, seed=123)
        Q, _ = tabu.TabuSampler._bqm_to_tabu_qubo(bqm)
        search = tabu.TabuSearch(Q, init, tenure, timeout, restarts)
        self.assertAlmostEqual(search.bestEnergy(), -14.65986790)

    def test_exceptions(self):
        qubo = [[-1.2, 1.1], [1.1, -1.2]]
        timeout = 10
        restarts = 100

        # Wrong length for init_solution
        with self.assertRaises(RuntimeError):
            init = [1, 1, 1]
            tenure = len(init) - 1
            search = tabu.TabuSearch(qubo, init, tenure, timeout, restarts)

        # Tenure out of bounds
        with self.assertRaises(RuntimeError):
            init = [1, 1]
            tenure = 3
            search = tabu.TabuSearch(qubo, init, tenure, timeout, restarts)
