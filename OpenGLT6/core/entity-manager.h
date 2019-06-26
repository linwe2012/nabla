#ifndef _NABLA_CORE_ENTITY_MANAGER_H_
#define _NABLA_CORE_ENTITY_MANAGER_H_

#include "entity.h"

#include "containers/Vector.h"



namespace nabla {

class EntityManager {
public:
	EntityManager(size_t initial_size = 1024);

	// I honestly don't know what should be happening when copying 
	EntityManager(const EntityManager&) = delete;
	const EntityManager& operator=(const  EntityManager&) = delete;

	// Create a new entity
	Entity Create();

	void Destroy(Entity e);

	bool IsAlive(Entity e) { return generations_[e.index()] == e.generation(); }

private:
	Vector<Entity> entities_;
	Vector<char> generations_;
	Entity next_available_;
	
	size_t num_available_;
};

inline EntityManager::EntityManager(size_t initial_size) 
{
	Entity nil = Entity::CreateNil();
	entities_.resize(initial_size, nil);
	generations_.resize(initial_size, 0);
	next_available_.MarkNil();
	next_available_.set_index(0);
	num_available_ = initial_size;
}

inline Entity EntityManager::Create()
{
	// if we are out of available slots
	if (num_available_ == 0) {
		if (entities_.size() == Entity::kMaxEntities) {
			return Entity::CreateNil();
		}
		auto new_size = entities_.size() * 2;
		new_size = new_size > Entity::kMaxEntities ? Entity::kMaxEntities : new_size;
		num_available_ = new_size - entities_.size();
		entities_.resize(new_size);
		generations_.resize(new_size, 0);
	}

	Entity::entity_t position = next_available_.index();

	if (next_available_.IsNil()) {
		next_available_.set_index(position + 1);
	}
	else {
		next_available_ = entities_[position];
	}

	Entity e;
	
	e.set_index(position);
	e.set_generation(generations_[position]);
	--num_available_;
	return e;
}

inline void EntityManager::Destroy(Entity e)
{
	ASSERT(IsAlive(e), "Destroying dead entity");
	if (!IsAlive(e)) return;

	// update generation
	++generations_[e.index()];
	if (generations_[e.index()] == 255) {
		generations_[e.index()] = 0;
	}

	// recycle entity
	entities_[e.index()] = next_available_;
	next_available_ = e;
	++num_available_;
}
}

#endif // !_NABLA_CORE_ENTITY_MANAGER_H_
