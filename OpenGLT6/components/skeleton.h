#ifndef  _NABLA_SYSTEM_SKELETON_H_
#define _NABLA_SYSTEM_SKELETON_H_

#include "glm/glm.hpp"


namespace nabla {

struct LocalPoseComponent{
	glm::mat4 local_pose;
};

struct SkeletonComponent {
	size_t size;
	uint16_t* hierarchy;
	glm::mat4* local_poses;
	glm::mat4* global_poses;
};

struct MeshStaticComponent {

};

struct PolygonStaticComponent {
	unsigned int vao;
	unsigned int vbo;
};

class Sampler {
	virtual float sample(float) = 0;
};



struct AnimatableComponent {
	float (*sampler)(float) = nullptr;
	glm::vec3 scale = glm::vec3(1.0f);
	glm::vec3 translate = glm::vec3(0.0f);
	glm::vec3 rotate_axis = glm::vec3(1.0f);
	float rad = 0.0f;
	float time = 0.0f;
	bool repeat : 1;
	bool active : 1;
};


}





#endif // ! _NABLA_SYSTEM_SKELETON_H_
