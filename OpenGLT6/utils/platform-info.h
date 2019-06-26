#ifndef _NABLA_UITLS_PLATFORM_INFO_H_
#define _NABLA_UITLS_PLATFORM_INFO_H_

#if defined (_WIN32)

#if defined (_WIN64)
#define PLATFORM_64BIT
#else
#define PLATFORM_32BIT
#endif // 32/64bit
#define PLATFORM_WINDOWS
#endif // windows


#if defined(__GNUC__)
#ifdef WIN32
#define PLATFORM_
#endif // WIN32

#ifdef __x86_64__ || __ppc64__
#define PLATFORM_64BIT
#else
#define PLATFORM_32BIT
#endif // __x86_64__ || __ppc64__
#endif

#ifdef unix
#define PLATFORM_UNIX
#endif // unix



#endif // !_NABLA_UITLS_PLATFORM_INFO_H_
