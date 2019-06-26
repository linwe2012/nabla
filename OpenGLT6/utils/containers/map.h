#ifndef _NA_UTILS_CONTAINERS_MAP_H_
#define _NA_UTILS_CONTAINERS_MAP_H_

#pragma warning (push, 0)
#include <map>
#include <unordered_map>
#pragma warning (pop)

namespace nabla {

template<typename K, typename V, typename Less = std::less<K>, typename Alloc = std::allocator<std::pair<const K, V>>>
using Map = std::map<K, V, Less, Alloc>;

template<typename K, typename V, typename Hasher = std::hash<K>, typename KeyEQ = std::equal_to<K>, typename Alloc = std::allocator<std::pair<const K, V>>>
using HashMap = std::unordered_map<K, V, Hasher, KeyEQ, Alloc>;
}


#endif // !_NA_UTILS_CONTAINERS_MAP_H_
