import os
from io import open
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext


class cythonizing_build_ext(build_ext):
    """Cython extensions adaptive build: cythonize if possible, include numpy
    headers, add custom compile/link flags."""

    # Custom extension compile/link flags
    extra_compile_args = {
        'msvc': ['/std:c++14'],
        'unix': ['-std=c++11'],
    }

    extra_link_args = {
        'msvc': [],
        'unix': [],
    }

    def build_extensions(self):
        # try using cython if available, fallback to using pre-cythonized
        # files (if they exist; if not, ultimately fail)
        try:
            from Cython.Build import cythonize

            # cythonization step will remove this flag needed by setuptools
            _needs_stub = [ext._needs_stub for ext in self.extensions]

            self.extensions = cythonize(self.extensions)

            # restore _needs_stub flags for setuptools
            for ext, ns in zip(self.extensions, _needs_stub):
                ext._needs_stub = ns

        except ImportError:
            # cython not available, assume cythonized cpp files exist
            for ext in self.extensions:
                ext.sources = [source.replace(".pyx", ".cpp") for source in ext.sources]

        # add compiler/linker flags
        compiler = self.compiler.compiler_type

        compile_args = self.extra_compile_args[compiler]
        for ext in self.extensions:
            ext.extra_compile_args = compile_args

        link_args = self.extra_compile_args[compiler]
        for ext in self.extensions:
            ext.extra_compile_args = link_args

        build_ext.build_extensions(self)

    def finalize_options(self):
        build_ext.finalize_options(self)

        # add numpy include files
        import numpy
        self.include_dirs.append(numpy.get_include())

# Load package info, without importing the package
basedir = os.path.dirname(os.path.abspath(__file__))
package_info_path = os.path.join(basedir, "tabu", "package_info.py")
package_info = {}
with open(package_info_path, encoding='utf-8') as f:
    exec(f.read(), package_info)

packages = ['tabu']

# Package requirements, minimal pinning
install_requires = ['numpy>=1.16.0', 'dimod>=0.9.0']

# Package extras requirements
extras_require = {
    'test': ['coverage'],
}

# Setup (extension build) requirements
setup_requires = ['numpy>=1.16.0']

# We distribute cythonized source, so cython is not required
# for install (build) from package, but allow manual override via env var
USE_CYTHON = os.getenv('USE_CYTHON', False)

if USE_CYTHON:
    setup_requires.append('cython>=0.29.12')

extensions = [
    Extension(
        name='tabu.tabu_search',
        sources=['./tabu/tabu_search.pyx', './tabu/src/utils.cpp', './tabu/src/bqp.cpp'],
        include_dirs=['./tabu/src/']), 
]

classifiers = [
    'License :: OSI Approved :: Apache Software License',
    'Operating System :: OS Independent',
    'Development Status :: 3 - Alpha',
    'Programming Language :: Python :: 3',
    'Programming Language :: Python :: 3.6',
    'Programming Language :: Python :: 3.7',
    'Programming Language :: Python :: 3.8',
    'Programming Language :: Python :: 3.9',
]

python_requires = '>=3.6'

setup(
    name=package_info['__packagename__'],
    version=package_info['__version__'],
    author=package_info['__author__'],
    author_email=package_info['__authoremail__'],
    description=package_info['__description__'],
    long_description=open('README.rst', encoding='utf-8').read(),
    url=package_info['__url__'],
    license=package_info['__license__'],
    classifiers=classifiers,
    cmdclass={'build_ext': cythonizing_build_ext},
    ext_modules=extensions,
    packages=packages,
    install_requires=install_requires,
    setup_requires=setup_requires,
    extras_require=extras_require,
    zip_safe=False,
    python_requires=python_requires,
)
