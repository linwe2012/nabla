#ifndef _NABLA_UTILS_UTILS_H_
#define _NABLA_UTILS_UTILS_H_

#include "logger.h"
#include <stdint.h>

#include <functional>

struct Defer {
	Defer(std::function<void()> func)
		: func_(func)
	{}

	~Defer() {
		func_();
	}

	std::function<void()> func_;
};

#endif // !_NABLA_UTILS_UTILS_H_
