//  Copyright 2018 D-Wave Systems Inc.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include "utils.h"

#include "common.h"

#if defined(_WIN32) || defined(_WIN64)

#include <Windows.h>

long long realtime_clock() {
    LARGE_INTEGER frequency;
    LARGE_INTEGER now;

    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&now);

    return (long long)(1000.0 * now.QuadPart / frequency.QuadPart);
}

#else

long long realtime_clock() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

#endif
