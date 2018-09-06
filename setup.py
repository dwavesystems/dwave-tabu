import os
from io import open
from distutils.core import setup, Extension


# Load package info, without importing the package
basedir = os.path.dirname(os.path.abspath(__file__))
package_info_path = os.path.join(basedir, "tabu", "package_info.py")
package_info = {}
try:
    with open(package_info_path, encoding='utf-8') as f:
        exec(f.read(), package_info)
except SyntaxError:
    execfile(package_info_path, package_info)


packages = ['tabu']

py_modules = ['tabu.tabu_search']

ext_modules = [
    Extension(
        name='tabu._tabu_search',
        sources=[
            os.path.join('tabu', 'tabu_search.i'),
            os.path.join('tabu', 'src', 'tabu_search.cc'),
            os.path.join('tabu', 'src', 'bqpSolver.cpp'),
            os.path.join('tabu', 'src', 'bqpUtil.cpp')
        ],
        include_dirs=[
            os.path.join('tabu', 'src')
        ],
        swig_opts=['-c++']
    )
]

install_requires = ['numpy>=1.14', 'dimod>=0.7']

extras_require = {
    'test': ['coverage'],
}

setup(
    name=package_info['__packagename__'],
    version=package_info['__version__'],
    author=package_info['__author__'],
    author_email=package_info['__authoremail__'],
    description=package_info['__description__'],
    long_description=open('README.rst', encoding='utf-8').read(),
    url=package_info['__url__'],
    license=package_info['__license__'],
    ext_modules=ext_modules,
    py_modules=py_modules,
    packages=packages,
    install_requires=install_requires,
    extras_require=extras_require,
)
