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

try:
    perf_counter = time.perf_counter
except AttributeError:  # pragma: no cover
    # python 2
    perf_counter = time.time

class RunTimeAssertionMixin(object):

    class assertRuntimeWithin(object):

        def __init__(self, low, high):
            """Min/max runtime in milliseconds."""
            self.limits = (low, high)

        def __enter__(self):
            self.tick = perf_counter()
            return self

        def __exit__(self, exc_type, exc_value, traceback):
            self.dt = (perf_counter() - self.tick) * 1000.0
            self.test()

        def test(self):
            low, high = self.limits
            if low is not None and self.dt < low:
                raise AssertionError("Min runtime unreached: %g ms < %g ms" % (self.dt, low))
            if high is not None and self.dt > high:
                raise AssertionError("Max runtime exceeded: %g ms > %g ms" % (self.dt, high))

    class assertMinRuntime(assertRuntimeWithin):

        def __init__(self, t):
            """Min runtime in milliseconds."""
            self.limits = (t, None)

    class assertMaxRuntime(assertRuntimeWithin):

        def __init__(self, t):
            """Max runtime in milliseconds."""
            self.limits = (None, t)


class TestTabuSearch(unittest.TestCase, RunTimeAssertionMixin):

    def test_trivial(self):
        qubo = np.array([[1.0]])
        init = [1]
        tenure = len(init) - 1
        timeout = 1

        search = tabu.TabuSearch(qubo, init, tenure, timeout)

        solution = list(search.bestSolution())
        energy = search.bestEnergy()

        self.assertEqual(solution, [0])
        self.assertEqual(energy, 0.0)

    def test_correctness(self):
        qubo = np.array([[-1.2, 1.1], [1.1, -1.2]])
        init = [1, 1]
        tenure = len(init) - 1
        timeout = 20

        search = tabu.TabuSearch(qubo, init, tenure, timeout)

        solution = list(search.bestSolution())
        energy = search.bestEnergy()

        self.assertEqual(solution, [0, 1])
        self.assertEqual(energy, -1.2)

    def test_concurrency(self):

        def search(timeout):
            return tabu.TabuSearch(np.array([[1.0]]), [1], 0, timeout).bestEnergy()

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

        bqm = dimod.generators.random.uniform(n, 'BINARY', low=-100, high=100, seed=123)
        Q, _ = tabu.TabuSampler._bqm_to_tabu_qubo(bqm)
        search = tabu.TabuSearch(Q, init, tenure, timeout)
        self.assertAlmostEqual(search.bestEnergy(), -1465.9867898)

        bqm = dimod.generators.random.uniform(n, 'BINARY', low=-1, high=1, seed=123)
        Q, _ = tabu.TabuSampler._bqm_to_tabu_qubo(bqm)
        search = tabu.TabuSearch(Q, init, tenure, timeout)
        self.assertAlmostEqual(search.bestEnergy(), -14.65986790)

    def test_exceptions(self):
        qubo = np.array([[-1.2, 1.1], [1.1, -1.2]])

        # Wrong length for init_solution
        with self.assertRaises(RuntimeError):
            init = [1, 1, 1]
            tenure = len(init) - 1
            search = tabu.TabuSearch(qubo, init, tenure, 10)

        # Tenure out of bounds
        with self.assertRaises(RuntimeError):
            init = [1, 1]
            tenure = 3
            search = tabu.TabuSearch(qubo, init, tenure, 10)
