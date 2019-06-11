#ifndef _NABLA_CORE_SPARSE_MAP_H_
#define _NABLA_CORE_SPARSE_MAP_H_

#include "containers/vector.h"
#include "entity.h"
#include "platform.h"

namespace nabla {
template<typename T>
class SparseMap {
public:
	
	bool count(Entity e) { 
		if (e.index() >= sparse_.size()) {
			return false;
		}
		if (sparse_[e.index()] == Entity::kInvalidIndex) {
			return false;
		}
		return true;
	}

	T* ptr(Entity e) {
		if (!Has(e)) {
			return nullptr;
		}
		return &dense_[sparse_[e.index()]];
	}

	T& operator[](Entity e) {
		return dense_[sparse_[e.index()]];
	}

	const T& operator[](Entity e) const {
		return dense_[sparse_[e.index()]];
	}

	void Add(Entity e, T data) {
		sparse_.size_at_least(e.index(), Entity::kInvalidIndex);
		sparse_[e.index()] = dense_.size();
		dense_.push_back(data);
	}

	T* begin() { return dense_.begin(); }

	T* end() { return dense_.end(); }

	size_t size() { return dense_.size(); }

	void erase(Entity e) {
		NA_LEAVE_IF((void)0, !Has(e)), "Erasing inexist element");
		dense_.erase(dense_.begin() + sparse_[e.index()]);
		sparse_[e.index()] = Entity::kInvalidIndex;
	}
private:
	Vector<Entity::entity_t> sparse_;
	Vector<T> dense_;
};

template<typename Component>
class SparseBindirectMap {
public:
	struct T {
		Component component;
		Entity entity;
	};

	bool count(Entity e) {
		if (e.index() >= sparse_.size()) {
			return false;
		}
		if (sparse_[e.index()] == Entity::kInvalidIndex) {
			return false;
		}
		return true;
	}

	T* ptr(Entity e) {
		if (!Has(e)) {
			return nullptr;
		}
		return &dense_[sparse_[e.index()]];
	}

	T& operator[](Entity e) {
		return dense_[sparse_[e.index()]];
	}

	const T& operator[](Entity e) const {
		return dense_[sparse_[e.index()]];
	}

	void Add(Entity e, Component data) {
		sparse_.size_at_least(e.index(), Entity::kInvalidIndex);
		sparse_[e.index()] = dense_.size();
		dense_.push_back(T{
			data,
			e
			});
	}

	T* begin() { return dense_.begin(); }

	T* end() { return dense_.end(); }

	size_t size() { return dense_.size(); }

	void erase(Entity e) {
		NA_LEAVE_IF((void)0, !Has(e)), "Erasing inexist element");
		dense_.erase(dense_.begin() + sparse_[e.index()]);
		sparse_[e.index()] = Entity::kInvalidIndex;
	}
private:
	Vector<Entity::entity_t> sparse_;
	Vector<T> dense_;
};


struct SparseIndex : public Vector<Entity::entity_t> {
	bool Has(Entity e) {
		if (e.index() >= size()) {
			return false;
		}
		if (operator[](e.index()) == Entity::kInvalidIndex) {
			return false;
		}
		return true;
	}

	void Add(Entity e, size_t offset) {
		size_at_least(e.index(), Entity::kInvalidIndex);

	}
};
}

#endif // !_NABLA_CORE_SPARSE_MAP_H_
