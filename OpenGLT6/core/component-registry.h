#ifndef _NABLA_CORE_COMPONENT_REGISTRY_H_
#define _NABLA_CORE_COMPONENT_REGISTRY_H_
#include <stdint.h>

#include "containers/vector.h"
#include "containers/map.h"

#include "entity.h"

#ifdef DEVELOPMENT
#include <typeinfo>
#endif // DEVELOPMENT

namespace nabla {

template<typename T>
struct UndeletablePtrHandle {
	UndeletablePtrHandle(T *data) : data_(data) {}
	T* operator->() { return data_; }
	T& operator*() { return *data_; }

	const T* operator->() const { return data_; }
	const T& operator*() const { return *data_; }

	void set(T* data) { data_ = data; }
private:
	T* data_;
};

template<typename T>
struct SharableComponent {
	SharableComponent() :data(nullptr), entity_to_component_id(nullptr) {}
	SharableComponent(T* shared, Vector<Entity::entity_t>* lookup)
		:data(shared), entity_to_component_id(lookup) {}
	UndeletablePtrHandle<T> data;
	Vector<Entity::entity_t>* entity_to_component_id;
};

class ComponentRegistry {
public:
	static ComponentRegistry* initService() {
		return new ComponentRegistry();
	}

	ComponentRegistry() {
#ifdef DEVELOPMENT
		types_.reserve(128);
		types_.push_back(0);
#endif
		data_.reserve(128);
		data_.push_back(nullptr);
	}

	struct Handle {
		Handle() :offset(0) {}
		friend class ComponentRegistry;
		bool IsNil() { return offset <= 0; }
	private:
		Handle(int x) : offset(x) {}
		int offset;
	};

	// Expose component free for other class to request
	template<typename T>
	Handle Expose(const char* name, SharableComponent<T>* component);

	// component must be the same type!!!!
	template<typename T>
	void changeComponent(Handle handle, SharableComponent<T>* component);

	template<typename T>
	SharableComponent<T>* request(const char* name);

	template<typename T>
	bool validate(int offset);
private:
	HashMap<std::string, Handle> name_book_;
	Vector<void*> data_;
#ifdef DEVELOPMENT
	Vector<size_t> types_;
#endif // DEVELOPMENT
};


template<typename T>
inline ComponentRegistry::Handle ComponentRegistry::Expose(const char* name, SharableComponent<T>* component) {
	
	auto itr = name_book_.find(name);
	if (itr != name_book_.end()) return Handle{ -1 };
	Handle h{ int(data_.size()) };
	data_.push_back(static_cast<void*>(component));
#ifdef DEVELOPMENT
	size_t tid = typeid(T).hash_code();
	types_.push_back(tid);
#endif // DEVELOPMENT
	return h;
}


template<typename T>
SharableComponent<T>* ComponentRegistry::request(const char* name) {
	void* res = nullptr;
	auto itr = name_book_.find(name);
	if (itr == name_book_.end()) {
		return nullptr;
	}
	auto offset = itr->second.offset;
	validate<T>(offset);
	return static_cast<SharableComponent<T>*>(data_[offset]);
}

template<typename T>
void ComponentRegistry::changeComponent(Handle handle, SharableComponent<T>* component) {
	ASSERT(!handle.IsNil(), "Unable to dereference nil handle");
	ASSERT(handle.offset < data_.size(), "Unable to dereference bad handle");
	data_[handle.offset] = component;
	validate<T>(handle.offset);
}

template<typename T>
bool ComponentRegistry::validate(int offset) {
#ifdef DEVELOPMENT
	ASSERT(typeid(T).hash_code() == types_[offset], "Wrong Type requested!");
#endif
}
}
#endif // !_NABLA_CORE_COMPONENT_REGISTRY_H_
