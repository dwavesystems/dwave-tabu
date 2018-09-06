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
