#pragma once

// my_cpp_98_compat.hpp
#include <cassert>

#ifndef constexpr
#define constexpr
#endif

#ifndef noexcept
#define noexcept
#endif

#ifndef _MSC_VER
#ifndef TRACE
#define TRACE printf
#endif
#else
#ifndef TRACE 
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define NOMINMAX
#include <atlbase.h>

#define TRACE ATLTRACE
#endif
#endif

#ifndef ASSERT
#define ASSERT assert
#endif

#ifndef nullptr
#define nullptr 0
#endif

static inline unsigned int nextPowerOf2(unsigned int n) {
    unsigned count = 0;

    // First n in the below condition
    // is for the case where n is 0
    if (n && !(n & (n - 1))) return n;

    while (n != 0) {
        n >>= 1;
        count += 1;
    }

    return 1 << count;
}

#ifndef int64_t
typedef  __int64 int64_t;
#endif

#ifndef uint64_t
typedef unsigned __int64 uint64_t;
#endif

#ifndef ptrdiff_type
#ifdef _MSC_VER 
	#if _MSC_VER <= 1200
	#define ptrdiff_type int
#endif
#else

#define ptrdiff_type std::ptrdiff_t
#endif
#endif
