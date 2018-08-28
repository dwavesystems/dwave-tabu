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

#endif // __COMMON_H__