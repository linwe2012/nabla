#ifndef _NABLA_OCEAN_SYSTEM_H_
#define _NABLA_OCEAN_SYSTEM_H_

#include "isystem.h"
#include "glm.h"
static constexpr float PI = 3.14159265358979323846f;

namespace nabla {

struct OceanParameters {
	float g; // gravity
	int N; // dimension -- N should be a power of 2
	float A; // phillips spectrum parameter -- affects heights of waves (Amplitude)
	glm::vec2 w; //direction of the wind
	float length;				// length parameter

	float Kx(int n_prime) const;
	float Kz(int m_prime) const;
	struct Data;
	Data* data;
};

class OceanSystem : public ISystem {
public:
	void Add(Entity) override {}

	// called upon system first registered
	void Initialize(SystemContext&) override;

	// activities on gui, note that you can actually do nothing
	virtual void OnGui(const Vector<Entity>& actives) override;

	// remove enitiy from system
	virtual void Remove(Entity) override {};

	virtual bool Has(Entity) const override { return false; };

	virtual void Update(Entity) override {};

	// called upon every frame
	void Update(Clock& clock) override {};

	const char* name() const override {
		return "ocean";
	}

private:

	struct Data;
	Data* data_;
};



}


#endif // !_NABLA_OCEAN_SYSTEM_H_
