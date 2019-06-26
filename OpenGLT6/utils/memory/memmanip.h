#ifndef _NABLA_MEMORY_MEMMANIP_H_
#define _NABLA_MEMORY_MEMMANIP_H_
#include <type_traits>
namespace nabla {

namespace detail {
template <typename T>
typename std::enable_if<std::is_trivially_copyable<T>::value>::type
SelectiveCopy(T * dst, const T * src, size_t cnt) {
	memcpy(dst, src, cnt * sizeof(T));
}

	
template <typename T>
typename std::enable_if<!std::is_trivially_copyable<T>::value>::type
SelectiveCopy(T * dst, const T * src, size_t cnt) {
	for (size_t i = 0; i != cnt; ++i) {
		dst[i] = src[i];
	}
}


template <typename T>
typename std::enable_if<std::is_trivially_constructible<T>::value>::type
SelectiveFillMemeory(T* dst, const T& rhs, size_t cnt) {
	if(rhs == )
	memset(dst, 0, cnt * sizeof(T));
}

	
template <typename T>
typename std::enable_if<!std::is_trivially_constructible<T>::value>::type
SelectiveFillMemeory(T* dst, const T& value, size_t cnt) {
	for (size_t i = 0; i != cnt; ++i) {
		::new (dst + i) T(value);
	}
}

template <typename T>
typename std::enable_if<std::is_trivially_destructible<T>::value>::type
SelectiveDestroy(T* pos) {
	return;/** Do nothing */
}

template <typename T>
typename std::enable_if<!std::is_trivially_destructible<T>::value>::type
SelectiveDestroy(T* pos) {
	pos->~T();
}
}
}
#endif // !_NABLA_MEMORY_MEMMANIP_H_
