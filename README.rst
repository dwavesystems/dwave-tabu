==========
dwave-tabu
==========

.. index-start-marker

An implementation of the `tabu algorithm <https://en.wikipedia.org/wiki/Tabu_search>`_ for
quadratic unconstrained binary optimization (QUBO) problems with a dimod Python wrapper.

.. index-end-marker

Installation or Building
========================

.. installation-start-marker

A wheel might be available for your system on PyPI. Source distributions are provided as well.

.. code-block:: python

    pip install dwave-tabu


Alternatively, you can build the library with setuptools.

.. code-block:: bash

    pip install -r python/requirements.txt
    python setup.py install

.. installation-end-marker

Example
=======

.. example-start-marker

.. code-block:: python

  from __future__ import print_function
  from tabu import TabuSearch

  # Create the problem
  q = [[-1, 2, 1], [2, -3, -4.5], [1, -4.5, 3.25]]
  init_solution = [0, 0, 1]
  tenure = 1
  scale_factor = 4
  timeout = 100  # millisecond

  # Run the solver
  r = TabuSearch(q, init_solution, tenure, scale_factor, timeout)

  # Print the results
  print("qubo =", q)
  print("best energy =", r.bestEnergy())
  print("best sample =", r.bestSolution())

.. example-end-marker
