#ifndef _NABLA_CORE_ENTITY_H_
#define _NABLA_CORE_ENTITY_H_

#include "assertion.h"
#include <stdint.h>

namespace nabla {

struct Entity
{
	using entity_t = unsigned int;
	using index_t = uint16_t;
private:
	enum :entity_t {
		kIndexMask = 0xFFFFFF,
		kGenerationMask = ~kIndexMask,
		kNil = kIndexMask | kGenerationMask,
	};
public:
	enum : entity_t { kInvalidIndex = kIndexMask };
	enum : entity_t { kMaxEntities = kIndexMask };
	entity_t id;

	entity_t index() const { return id & kIndexMask; }
	entity_t generation() const { return id & kGenerationMask; }
	
	void set_index(entity_t index) { id = generation() | index; }
	void set_generation(char gen);

	bool IsNil() const { return (id & kGenerationMask) == kGenerationMask; }
	void MarkNil() { id |= kGenerationMask; }

	bool operator<(const Entity& rhs) const { return id < rhs.id; }
	bool operator==(const Entity& rhs) const { return id == rhs.id; }
	bool operator!=(const Entity& rhs) const { return id != rhs.id; }

	static Entity CreateNil() { return Entity{ kNil }; }

	Entity() : id (kNil) {}

	Entity(entity_t _id) : id(_id) {}
};

inline void Entity::set_generation(char gen) {
	ASSERT(gen != 255, "Unable to set generation to nil");
	entity_t h = gen;
	id = index() | (h << 24);
}

}



#endif // !_NABLA_CORE_ENTITY_H_
