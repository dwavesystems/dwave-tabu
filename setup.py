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

import os
from io import open
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext


# Load package info, without importing the package
basedir = os.path.dirname(os.path.abspath(__file__))
package_info_path = os.path.join(basedir, "tabu", "package_info.py")
package_info = {}
try:
    with open(package_info_path, encoding='utf-8') as f:
        exec(f.read(), package_info)
except SyntaxError:
    execfile(package_info_path, package_info)

extra_compile_args = {
    'msvc': ['/std:c++14'],
    'unix': ['-std=c++11'],
}


class build_ext_compiler_check(build_ext):
    def build_extensions(self):
        compiler = self.compiler.compiler_type

        compile_args = extra_compile_args[compiler]
        for ext in self.extensions:
            ext.extra_compile_args = compile_args

        build_ext.build_extensions(self)


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

install_requires = ['numpy>=1.14', 'dimod>=0.7.9']

extras_require = {
    'test': ['coverage'],
}

setup(
    name=package_info['__packagename__'],
    version=package_info['__version__'],
    author=package_info['__author__'],
    author_email=package_info['__authoremail__'],
    description=package_info['__description__'],
    long_description=open(os.path.join(basedir, 'README.rst'), encoding='utf-8').read(),
    url=package_info['__url__'],
    license=package_info['__license__'],
    ext_modules=ext_modules,
    cmdclass={'build_ext': build_ext_compiler_check},
    py_modules=py_modules,
    packages=packages,
    install_requires=install_requires,
    extras_require=extras_require,
)
