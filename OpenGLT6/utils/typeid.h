#ifndef _NABLA_UTILS_TYPEID_H_
#define _NABLA_UTILS_TYPEID_H_

#include <type_traits>

// This is implementation of runtime typeid
class TypeId {
	static unsigned int identifier;
	template<typename T>
	static const auto inner = identifier++;

public:
	template<typename T>
	unsigned int static const vbo = inner<std::decay<T>>;
};

#define TYPESCURE Vector<size_t> type_ids_; \
	template<typename T>  \
	bool validate() { \
		\
	}


#endif // !_NABLA_UTILS_TYPEID_H_


