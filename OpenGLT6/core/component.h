#ifndef _NABLA_CORE_SHARABLE_COMPONENT_H_
#define _NABLA_CORE_SHARABLE_COMPONENT_H_


#include "entity.h"

namespace nabla {



}

#endif


#if 0
namespace componentbase {
	template<typename... Types>
	class SparseArray {
	public:
		/*
		T* lookup(Entity e) {
			auto pos = lookup_[e.index()];
			if (pos >= data_.size()) {
				return nullptr;
			}
			return &data_[pos];
		}

		T* pop(Entity e) {
			auto pos = lookup_[e.index()];
			if (pos >= data_.size()) {
				return nullptr;
			}
			auto last = data_.size() - 1;
			if (pos != last) {
				using std::swap;
				swap(data_[pos], data_[last]);
			}
			data_.pop_back();
			return &data_[last];
		}

		T * pop(Entity e) {
			auto pos = lookup_[e.index()];
			if (pos >= data_.size()) {
				return nullptr;
			}
			auto last = data_.size() - 1;
			if (pos != last) {
				using std::swap;
				swap(data_[pos], data_[last]);
			}

			auto res = &data_[last];
			// since stl won't actually call dtor
			// it is safe
			data_.pop_back();
			return res;
		}
		*/

		template<typename T>
		T* get() { return std::get<T*>(data_); }

	private:
		friend class ComponentRegistry;
		std::tuple<Types*> data_;

		// Vector<unsigned int> lookup_;
		// Vector<T> data_;
		// size_t actual_size_;
	};
}
#endif