# Copyright 2018 D-Wave Systems Inc.
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

"""Test the TabuSampler python interface."""

import unittest

import dimod

import tabu


class TestTabuSampler(unittest.TestCase):
    def test_instantiation(self):
        sampler = tabu.TabuSampler()
        dimod.testing.assert_sampler_api(sampler)

    def test_sample_basic(self):
        sampler = tabu.TabuSampler()

        bqm = dimod.BinaryQuadraticModel.from_ising({}, {'ab': -1, 'bc': +1, 'ac': +1})

        resp = sampler.sample(bqm)

        dimod.testing.assert_response_energies(resp, bqm)

    def test_sample_num_reads(self):
        bqm = dimod.BinaryQuadraticModel.from_ising({}, {'ab': -1, 'bc': +1, 'ac': +1})

        resp = tabu.TabuSampler().sample(bqm, num_reads=57)
        dimod.testing.assert_response_energies(resp, bqm)
        self.assertEqual(sum(resp.record.num_occurrences), 57)

    def test_disconnected_problem(self):
        h = {}
        J = {
            # K_3
            (0, 1): -1,
            (1, 2): -1,
            (0, 2): -1,

            # disonnected K_3
            (3, 4): -1,
            (4, 5): -1,
            (3, 5): -1,
        }

        bqm = dimod.BinaryQuadraticModel.from_ising(h, J)
        resp = tabu.TabuSampler().sample(bqm)
        dimod.testing.assert_response_energies(resp, bqm)

    def test_empty(self):
        resp = tabu.TabuSampler().sample(dimod.BinaryQuadraticModel.empty(dimod.SPIN))
        dimod.testing.assert_response_energies(resp, dimod.BinaryQuadraticModel.empty(dimod.SPIN))

        resp = tabu.TabuSampler().sample(dimod.BinaryQuadraticModel.empty(dimod.BINARY))
        dimod.testing.assert_response_energies(resp, dimod.BinaryQuadraticModel.empty(dimod.BINARY))

        resp = tabu.TabuSampler().sample_qubo({})
        dimod.testing.assert_response_energies(resp, dimod.BinaryQuadraticModel.empty(dimod.BINARY))

        resp = tabu.TabuSampler().sample_ising({}, {})
        dimod.testing.assert_response_energies(resp, dimod.BinaryQuadraticModel.empty(dimod.SPIN))

    def test_linear_problem(self):
        bqm = dimod.BinaryQuadraticModel.from_ising({v: -1 for v in range(100)}, {})
        resp = tabu.TabuSampler().sample(bqm)
        dimod.testing.assert_response_energies(resp, bqm)

    def test_init_solution_smoketest(self):
        bqm = dimod.BinaryQuadraticModel.from_ising({}, {'ab': -1, 'bc': +1, 'ac': +1})
        resp = tabu.TabuSampler().sample(bqm, init_solution=tabu.TabuSampler().sample(bqm))
        dimod.testing.assert_response_energies(resp, bqm)

    def test_init_solution(self):
        bqm = dimod.BinaryQuadraticModel.from_ising({}, {'ab': -1, 'bc': +1, 'ac': +1})
        init = dimod.SampleSet.from_samples({'a': 0, 'b': 0, 'c': 0}, vartype=dimod.BINARY, energy=0)
        resp = tabu.TabuSampler().sample(bqm, init_solution=init)
        dimod.testing.assert_response_energies(resp, bqm)
