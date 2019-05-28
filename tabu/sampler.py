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

"""A dimod sampler_ that uses the MST2 multistart tabu search algorithm."""

from __future__ import division

import random
import warnings

import numpy
import dimod

from tabu import TabuSearch


class TabuSampler(dimod.Sampler):
    """A tabu-search sampler.

    Examples:
        This example solves a two-variable Ising model.

        >>> from tabu import TabuSampler
        >>> samples = TabuSampler().sample_ising({'a': -0.5, 'b': 1.0}, {'ab': -1})
        >>> list(samples.data()) # doctest: +SKIP
        [Sample(sample={'a': -1, 'b': -1}, energy=-1.5, num_occurrences=1)]
        >>> samples.first.energy
        -1.5

    """

    properties = None
    parameters = None

    def __init__(self):
        self.parameters = {'tenure': [],
                           'scale_factor': [],
                           'timeout': [],
                           'num_reads': [],
                           'init_solution': []}
        self.properties = {}

    def sample(self, bqm, initial_states=None, tenure=None, scale_factor=1,
               timeout=20, num_reads=1, **kwargs):
        """Run Tabu search on a given binary quadratic model.

        Args:
            bqm (:class:`~dimod.BinaryQuadraticModel`):
                The binary quadratic model (BQM) to be sampled.

            initial_states (:class:`~dimod.SampleSet`, optional, default=None):
                Single sample that sets an initial state for all the problem variables.
                Default is a random initial state.

            init_solution (:class:`~dimod.SampleSet`, optional):
                Deprecated. Alias for `initial_states`.

            tenure (int, optional):
                Tabu tenure, which is the length of the tabu list, or number of recently
                explored solutions kept in memory.
                Default is a quarter of the number of problem variables up to
                a maximum value of 20.

            scale_factor (number, optional):
                Scaling factor for linear and quadratic biases in the BQM. Internally, the BQM is
                converted to a QUBO matrix, and elements are stored as long ints
                using ``internal_q = long int (q * scale_factor)``.

            timeout (int, optional):
                Total running time in milliseconds.

            num_reads (int, optional): Number of reads. Each run of the tabu algorithm
                generates a sample.

        Returns:
            :obj:`~dimod.SampleSet`: A `dimod` :obj:`.~dimod.SampleSet` object.

        Examples:
            This example provides samples for a two-variable QUBO model.

            >>> from tabu import TabuSampler
            >>> import dimod
            >>> sampler = TabuSampler()
            >>> Q = {(0, 0): -1, (1, 1): -1, (0, 1): 2}
            >>> bqm = dimod.BinaryQuadraticModel.from_qubo(Q, offset=0.0)
            >>> samples = sampler.sample(bqm)
            >>> samples.record[0].energy
            -1.0
        """

        if 'init_solution' in kwargs:
            warnings.warn(
                "`init_solution` is deprecated in favor of `initial_states`.",
                DeprecationWarning)

        init_solution = kwargs.pop('init_solution', initial_states)

        # input checking and defaults calculation
        # TODO: one "read" per sample in init_solution sampleset
        if init_solution is not None:
            if not isinstance(init_solution, dimod.SampleSet):
                raise TypeError("'init_solution' should be a 'dimod.SampleSet' instance")
            if len(init_solution.record) < 1:
                raise ValueError("'init_solution' should contain at least one sample")
            if len(init_solution.record[0].sample) != len(bqm):
                raise ValueError("'init_solution' sample dimension different from BQM")
            init_sample = self._bqm_sample_to_tabu_sample(
                init_solution.change_vartype(dimod.BINARY, inplace=False).record[0].sample, bqm.binary)
        else:
            init_sample = None

        if not bqm:
            return dimod.SampleSet.from_samples([], energy=0, vartype=bqm.vartype)

        if tenure is None:
            tenure = max(min(20, len(bqm) // 4), 0)
        if not isinstance(tenure, int):
            raise TypeError("'tenure' should be an integer in range [0, num_vars - 1]")
        if not 0 <= tenure < len(bqm):
            raise ValueError("'tenure' should be an integer in range [0, num_vars - 1]")

        if not isinstance(num_reads, int):
            raise TypeError("'num_reads' should be a positive integer")
        if num_reads < 1:
            raise ValueError("'num_reads' should be a positive integer")

        qubo = self._bqm_to_tabu_qubo(bqm.binary)

        # run Tabu search
        samples = []
        energies = []
        for _ in range(num_reads):
            if init_sample is None:
                init_sample = self._bqm_sample_to_tabu_sample(self._random_sample(bqm.binary), bqm.binary)
            r = TabuSearch(qubo, init_sample, tenure, scale_factor, timeout)
            sample = self._tabu_sample_to_bqm_sample(list(r.bestSolution()), bqm.binary)
            energy = bqm.binary.energy(sample)
            samples.append(sample)
            energies.append(energy)

        response = dimod.SampleSet.from_samples(
            samples, energy=energies, vartype=dimod.BINARY)
        response.change_vartype(bqm.vartype, inplace=True)
        return response

    def _bqm_to_tabu_qubo(self, bqm):
        # Note: normally, conversion would be: `ud + ud.T - numpy.diag(numpy.diag(ud))`,
        # but the Tabu solver we're using requires slightly different qubo matrix.
        varorder = sorted(list(bqm.adj.keys()))
        ud = 0.5 * bqm.to_numpy_matrix(varorder)
        symm = ud + ud.T
        qubo = symm.tolist()
        return qubo

    def _bqm_sample_to_tabu_sample(self, sample, bqm):
        assert len(sample) == len(bqm)
        _, values = zip(*sorted(self._sample_as_dict(sample).items()))
        return list(map(int, values))

    def _tabu_sample_to_bqm_sample(self, sample, bqm):
        varorder = sorted(list(bqm.adj.keys()))
        assert len(sample) == len(varorder)
        return dict(zip(varorder, sample))

    def _sample_as_dict(self, sample):
        """Convert list-like ``sample`` (list/dict/dimod.SampleView),
        ``list: var``, to ``map: idx -> var``.
        """
        if isinstance(sample, dict):
            return sample
        if isinstance(sample, (list, numpy.ndarray)):
            sample = enumerate(sample)
        return dict(sample)

    def _random_sample(self, bqm):
        values = list(bqm.vartype.value)
        return {i: random.choice(values) for i in bqm.variables}


if __name__ == "__main__":
    from pprint import pprint

    print("TabuSampler:")
    bqm = dimod.BinaryQuadraticModel(
        {'a': 0.0, 'b': -1.0, 'c': 0.5},
        {('a', 'b'): -1.0, ('b', 'c'): 1.5},
        offset=0.0, vartype=dimod.BINARY)
    response = TabuSampler().sample(bqm, num_reads=10)
    pprint(list(response.data()))

    print("ExactSolver:")
    response = dimod.ExactSolver().sample(bqm)
    pprint(list(response.data(sorted_by='energy')))
