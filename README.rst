==========
dwave-tabu
==========

.. index-start-marker

An implementation of the `MST2 multistart tabu search algorithm
<https://link.springer.com/article/10.1023/B:ANOR.0000039522.58036.68>`_
for quadratic unconstrained binary optimization (QUBO) problems
with a `dimod <https://dimod.readthedocs.io/en/latest/>`_ Python wrapper.

.. index-end-marker

Installation or Building
========================

.. installation-start-marker

**Package not yet available on PyPI.** Install in developer (edit) mode::

    pip install -e git+https://github.com/dwavesystems/dwave-tabu.git#egg=dwave-tabu

Alternatively, you can build the library with setuptools. This build requires that
your system has a C compiler toolchain installed.

.. code-block:: bash

    pip install -r requirements.txt
    python setup.py install

.. installation-end-marker

Example
=======

.. example-start-marker

This example solves a two-variable Ising model.

>>> from tabu import TabuSampler
>>> response = TabuSampler().sample_ising({'a': -0.5, 'b': 1.0}, {('a', 'b'): -1})

.. example-end-marker
