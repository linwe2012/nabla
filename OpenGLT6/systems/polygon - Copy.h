#pragma once
#include "containers/vector.h"
#include "systems/isystem.h"
namespace nabla {

class PolygonSystem : public ISystem {
public:
	enum {
		kCube,
		kSphere,
		kCylinder,
		kCone,
		kPrism,
		kFrustrum
	};


	// called upon system first registered
	void Initilize() override {};

	// activities on gui, note that you can actually do nothing
	virtual void OnGui(const Vector<Entity>& actives) override {};

	// remove enitiy from system
	virtual void Remove(Entity) override {};

	virtual bool Has(Entity) const override {};

	virtual void Update(Entity) override {};

	// called upon every frame
	void Update(Clock& clock) override {};

	const char* name() const override {
		return "polygon";
	}
};


}
