.. image:: https://badge.fury.io/py/dwave-tabu.svg
    :target: https://badge.fury.io/py/dwave-tabu
    :alt: Last version on PyPI

.. image:: https://circleci.com/gh/dwavesystems/dwave-tabu.svg?style=svg
    :target: https://circleci.com/gh/dwavesystems/dwave-tabu
    :alt: Linux/Mac build status

.. image:: https://ci.appveyor.com/api/projects/status/79notdhalmnbbh1v/branch/master?svg=true
    :target: https://ci.appveyor.com/project/dwave-adtt/dwave-tabu/branch/master
    :alt: Windows build status

.. image:: https://readthedocs.com/projects/d-wave-systems-dwave-tabu/badge/?version=latest
    :target: https://docs.ocean.dwavesys.com/projects/d-wave-systems-dwave-tabu/en/latest/?badge=latest
    :alt: Documentation Status

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

Installation or Building
========================

.. installation-start-marker

Install from a wheel on PyPI::

    pip install dwave-tabu

Alternatively, you can build the library with setuptools. This build requires that
your system has a C++ compiler toolchain installed, as well as `SWIG <http://www.swig.org/>`_.

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
