#ifndef _NABLA_CORE_UTILS_CONTAINERS_SET_H_
#define _NABLA_CORE_UTILS_CONTAINERS_SET_H_
#include <set>
namespace nabla {
	template<typename T, typename Less = std::less<T>, typename Alloc = std::allocator<T>>
	using Set = std::set<T, Less, Alloc>;
}

#endif // !_NABLA_CORE_UTILS_CONTAINERS_SET_H_

