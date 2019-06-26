#ifndef _UTILS_PLATFORM_GCC_H_
#define _UTILS_PLATFORM_GCC_H_

#ifdef __GNUC__

#define FORCE_INLINE __attribute__((always_inline))

// no equivalence in gcc
#define NO_VIRTUAL_TABLE

#endif // __GNUC__


#endif // !_UTILS_PLATFORM_GCC_H_
