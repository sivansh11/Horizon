#ifndef DEBUG_H
#define DEBUG_H

#include <stdexcept>
#include <iostream>

#ifndef NDEBUG 
#define ASSERT(condition, message) \
    do { \
        if (! (condition)) { \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ \
                      << " line " << __LINE__ << ": " << message << std::endl; \
            std::terminate(); \
        } \
    } while (false)

#else
#define ASSERT(condition, message);

#endif

#endif