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

#ifndef __COMMON_H__
#define __COMMON_H__

#include <map>
#include <sstream>
#include <cstdlib>

class Exception: public std::exception
{
    public:
        Exception(std::string error) : error_msg(error) {}
        ~Exception() throw() {}

        virtual const char* what() const throw()
        {
            return error_msg.c_str();
        }
    protected:
        std::string error_msg;
};

#endif
