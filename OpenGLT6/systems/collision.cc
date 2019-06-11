#include "collision.h"

namespace nabla {

void nabla::CollisonSystem::Update(Clock& clock)
{
	// ... compute collision, update rigids

	
	for (auto& rigid_itr : rigids_) {
		Transform* trans = render->GetTransformEdit(rigid_itr.entity);
		if (trans == nullptr) {
			continue;
		}
		RigidBody& rigid = rigid_itr.component;

		trans->position += rigid.velocity * clock.GetLastFrameDurationFloat();
	}

}

void CollisonSystem::Add(Entity e)
{
}

}

