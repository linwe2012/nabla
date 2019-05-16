#ifndef _UTILS_PLATFORM_MSC_H_
#define _UTILS_PLATFORM_MSC_H_

#ifdef _MSC_VER

#define FORCE_INLINE __forceinline

#define NO_VIRTUAL_TABLE __declspec(novtable)

#endif // _MSC_VER

#endif // !_UTILS_PLATFORM_GCC_H_
