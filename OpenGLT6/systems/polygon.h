#pragma once
#include "containers/vector.h"
#include "systems/isystem.h"
#include "core/renderer.h"

#include "renderable.h"

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

	void Add(Entity) override {}

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

	void BindRenderable(RenderableSystem* render);

	const char* name() const override {
		return "polygon";
	}

	RenderableSystem* render;
};


}