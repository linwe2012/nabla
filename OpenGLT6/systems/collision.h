#ifndef _NABLA_SYSTEM_COLLISON_H_
#define _NABLA_SYSTEM_COLLISON_H_
#include "isystem.h"
#include "core/sparse-map.h"
#include "components/primitive.h"
#include "renderable.h"

namespace nabla {
class CollisonSystem : public ISystem {
public:
	// called upon system first registered
	void Initilize() override {};

	// activities on gui, note that you can actually do nothing
	virtual void OnGui(const Vector<Entity>& actives) override {};

	// remove enitiy from system
	virtual void Remove(Entity) override {};

	virtual bool Has(Entity) const override {};

	virtual void Update(Entity) override {};

	// called upon every frame, update collide object
	void Update(Clock& clock) override;

	void Add(Entity) override;

	const char* name() const override {
		return "polygon";
	}

private:
	SparseBindirectMap<RigidBody> rigids_;
	RenderableSystem* render;
	// octree etc.
};
}


#endif // !_NABLA_SYSTEM_COLLISON_H_

