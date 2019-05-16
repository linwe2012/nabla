#ifndef _NABLA_UTILS_LOGGER_H_
#define _NABLA_UTILS_LOGGER_H_
#include <assert.h>
#include <stdio.h>

#define NA_ASSERT(cond, fmt, ...) assert(cond)

enum class LogSeverity {
	kInfo,
	kWarn,
	kError,
	kFatal,
};

#define NA_LOG_HELPER(severity, ...) fprintf(stderr, __VA_ARGS__);
#define NA_LOG_IF_HELPER(severity, cond, ...) do { \
	if(cond) { \
		fprintf(stderr, __VA_ARGS__); \
	} \
} while (0)

#define NA_LOG_INFO(...) NA_LOG_HELPER(LogSeverity::kInfo, __VA_ARGS__)
#define NA_LOG_WARN(...) NA_LOG_HELPER(LogSeverity::kWarn, __VA_ARGS__)
#define NA_LOG_ERROR(...) NA_LOG_HELPER(LogSeverity::kError, __VA_ARGS__)
#define NA_LOG_FATAL(...) NA_LOG_HELPER(LogSeverity::kFatal, __VA_ARGS__)


#define NA_LOG_FATAL_IF(cond, ...) NA_LOG_IF_HELPER(LogSeverity::kFatal, cond, __VA_ARGS__)
#define NA_LOG_ERROR_IF(cond, ...) NA_LOG_IF_HELPER(LogSeverity::kError, cond, __VA_ARGS__)

#define NA_DEBUG

#ifdef NA_DEBUG
#define NA_GL_CHECK(call) \
	do { \
		call; \
		GLenum gl_err = glGetError(); \
		if(gl_err != 0) { NA_LOG_ERROR("GL error 0x%x: %s", gl_err, glEnumName(gl_err)); } \
	} while (0)
#else
#define NA_GL_CHECK(call) call
#endif // NA_DEBUG

#define NA_LEAVE_IF(ret, cond, ...) do { \
if(cond) { NA_ASSERT(cond, __VAR_ARGS__); } else { } \
}while (0)


/* Lazy loader will use raw data from assimp directly instead of 
copying data around, which is expected to be quicker
*/
#define NA_LAZY_LOADER

#endif // !_NABLA_UTILS_LOGGER_H_
