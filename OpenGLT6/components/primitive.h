#ifndef _NABLA_CORE_PRMITIVE_COMPONENT_H_
#define _NABLA_CORE_PRMITIVE_COMPONENT_H_

#include "core/entity.h"
#include "glm.h"

namespace nabla {


struct Transform {
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);
	glm::quat quaternion;
};

struct RigidBody {
	glm::vec3 velocity;
	glm::vec3 accleration;
	float mass;
	float drag;
	glm::vec3 _min;
	glm::vec3 _max;
};

}

#endif