.. image:: https://circleci.com/gh/dwavesystems/dwave-tabu.svg?style=svg
    :target: https://circleci.com/gh/dwavesystems/dwave-tabu
    :alt: Linux/MacOS/Windows build status

.. image:: https://codecov.io/gh/dwavesystems/dwave-tabu/branch/master/graph/badge.svg?token=uoMkg6WvKy
    :target: https://codecov.io/gh/dwavesystems/dwave-tabu
    :alt: Code coverage

.. image:: https://readthedocs.com/projects/d-wave-systems-dwave-tabu/badge/?version=latest
    :target: https://docs.ocean.dwavesys.com/projects/d-wave-systems-dwave-tabu/en/latest/?badge=latest
    :alt: Documentation Status

.. image:: https://badge.fury.io/py/dwave-tabu.svg
    :target: https://badge.fury.io/py/dwave-tabu
    :alt: Latest version on PyPI

.. image:: https://img.shields.io/pypi/pyversions/dwave-tabu.svg?style=flat
    :target: https://pypi.org/project/dwave-tabu/
    :alt: PyPI - Python Version


==========
dwave-tabu
==========

.. index-start-marker

A C/C++ implementation of the `MST2 multistart tabu search algorithm
<https://link.springer.com/article/10.1023/B:ANOR.0000039522.58036.68>`_
for quadratic unconstrained binary optimization (QUBO) problems with a
`dimod sampler <https://docs.ocean.dwavesys.com/en/stable/docs_dimod/reference/sampler_composites/api.html#dimod.Sampler>`_
Python interface.

.. index-end-marker


Installation
============

.. installation-start-marker

Install from a wheel on PyPI::

    pip install dwave-tabu

or install from source:

.. code-block:: bash

    pip install git+https://github.com/dwavesystems/dwave-tabu.git#egg=dwave-tabu

Note: installation from source involves a "cythonization" step. To install
project requirements automatically, make sure to use a PEP-517 compliant pip,
e.g. ``pip>=10.0``.

To build from source:

.. code-block:: bash

    pip install -r requirements.txt
    python setup.py build_ext --inplace
    python setup.py install

.. installation-end-marker


Example
=======

.. example-start-marker

This example solves a two-variable Ising model.

>>> from tabu import TabuSampler
>>> response = TabuSampler().sample_ising({'a': -0.5, 'b': 1.0}, {('a', 'b'): -1})

.. example-end-marker


License
=======

Released under the Apache License 2.0. See `<LICENSE>`_ file.

Contributing
============

Ocean's `contributing guide <https://docs.ocean.dwavesys.com/en/stable/contributing.html>`_
has guidelines for contributing to Ocean packages.
