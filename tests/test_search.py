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
        qubo = [[1]]
        init = [1]
        tenure = len(init) - 1
        scale = 1
        timeout = 1

        search = tabu.TabuSearch(qubo, init, tenure, scale, timeout)

        solution = list(search.bestSolution())
        energy = search.bestEnergy()

        self.assertEqual(solution, [0])
        self.assertEqual(energy, 0.0)

    def test_correctness(self):
        qubo = [[2, 1, 1], [1, 2, 1], [1, 1, 2]]
        init = [1, 1, 1]
        tenure = len(init) - 1
        scale = 1
        timeout = 20

        search = tabu.TabuSearch(qubo, init, tenure, scale, timeout)

        solution = list(search.bestSolution())
        energy = search.bestEnergy()

        self.assertEqual(solution, [0, 0, 0])
        self.assertEqual(energy, 0.0)

    def test_concurrency(self):

        def search(timeout):
            return tabu.TabuSearch([[1]], [1], 0, 1, timeout).bestEnergy()

        with ThreadPoolExecutor(max_workers=3) as executor:

            # ~ 500 ms
            with self.assertRuntimeWithin(400, 600):
                wait([executor.submit(search, timeout=500) for _ in range(3)])

            # ~ 1000 ms
            with self.assertRuntimeWithin(900, 1100):
                wait([executor.submit(search, timeout=500) for _ in range(4)])
