from distutils.core import setup, Extension


setup(
    name='dwave-tabu',
    version='0.0.1',
    ext_modules=[
        Extension(
            name='tabu._tabu_search',
            sources=[
                'tabu/tabu_search.i',
                'tabu/src/tabu_search.cc',
                'tabu/src/bqpSolver.cpp',
                'tabu/src/bqpUtil.cpp'
            ],
            include_dirs=['tabu/src'],
            swig_opts=['-c++']
        )
    ],
)
