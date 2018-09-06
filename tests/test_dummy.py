import unittest

import tabu


class TestDummy(unittest.TestCase):
    """Simplest possible test for setting up CI"""
    def test_simple(self):
        tabu.TabuSampler().sample_ising({'a': -0.5, 'b': 1.0}, {('a', 'b'): -1}).data()
