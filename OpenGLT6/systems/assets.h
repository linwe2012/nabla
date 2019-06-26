#ifndef _NABLA_SYSTEM_ASSETS_H_
#define _NABLA_SYSTEM_ASSETS_H_
#include "isystem.h"

namespace nabla {

class AssetsSystem : public ISystem {
public:
	// activities on gui, note that you can actually do nothing
	virtual void OnGui(const Vector<Entity>& actives) override;

	// remove enitiy from system
	virtual void Remove(Entity) override {};

	virtual bool Has(Entity) const override { return false; };

	virtual void Update(Entity) override {};

	// called upon every frame, update collide object
	void Update(Clock& clock) override {};

	void Add(Entity) override {};

	const char* name() const override {
		return "Assets";
	}

	void Initialize(SystemContext&) override;

private:
	SystemContext* ctx_ = nullptr;
	struct Data;
	Data* data_ = nullptr;
	// std::unique_ptr<Data> data_;
};

}


#endif // !_NABLA_SYSTEM_ASSETS_H_

