# Copyright 2021 D-Wave Systems Inc.
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

import time

__all__ = ['tictoc']


class tictoc:
    """Timer as a context manager.

    Elapsed wall clock time in floating point seconds available in :attr:`.dt`.

    Adapted from D-Wave Hybrid's :class:`hybrid.profiling.tictoc`
    (https://github.com/dwavesystems/dwave-hybrid/blob/5844ce2461795a0b4dec58ec1dd4c1264dbd9e84/hybrid/profiling.py#L33-L87).
    """
    # NOTE: make sure to remove if/when we collect common utilities in a package
    # (e.g. dwave-common)

    dt: float = None

    def __enter__(self):
        self.tick = time.perf_counter()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.dt = time.perf_counter() - self.tick
