#include "collision.h"
#include <algorithm>

namespace nabla {
void CollisonSystem::Initialize(SystemContext& ctx)
{
	render = ctx.render;
}

void CollisonSystem::Update(Clock& clock)
{
	// compute collision
	for (auto& rigid_itr1 : rigids_) {
		for (auto& rigid_itr2 : rigids_) {
			if (rigid_itr1.entity == rigid_itr2.entity) continue;
			Transform* trans1 = render->GetTransformEdit(rigid_itr1.entity);
			Transform* trans2 = render->GetTransformEdit(rigid_itr2.entity);
			if (trans1 == nullptr || trans2 == nullptr) {
				continue;
			}
			glm::mat4 mat1 = glm::mat4(1.0f);
			glm::mat4 mat2 = glm::mat4(1.0f);
			mat1 = glm::translate(mat1, trans1->position);
			mat2 = glm::translate(mat2, trans2->position);
			mat1 = glm::scale(mat1, trans1->scale);
			mat2 = glm::scale(mat2, trans2->scale);

			// if collide
			if (IsCollide(rigid_itr1.component, rigid_itr2.component, mat1, mat2)) {
				ComputeCollison(rigid_itr1.component, rigid_itr2.component, mat1, mat2);
			}
		}
	}

	// update rigids
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
	RigidBody rigid;
	rigid.velocity = glm::vec3(0.0f);
	rigid.accleration = glm::vec3(0.0f);
	rigid.mass = 1.0f;
	auto pvertices = render->GetVertices(e);
	if (!pvertices) {
		NA_LOG_WARN("Entity has no tranform component, will use a default bounding box instead");
		rigid._max = glm::vec3(1.0f);
		rigid._min = glm::vec3(-1.0f);
	}
	else {
		glm::vec3* vertices = pvertices->positions;
		size_t num_vertices = pvertices->num_vertices;
		for (size_t i = 0; i < num_vertices; ++i) {
			rigid._max.x = std::max(rigid._max.x, vertices[i].x);
			rigid._max.y = std::max(rigid._max.y, vertices[i].y);
			rigid._max.z = std::max(rigid._max.z, vertices[i].z);

			rigid._min.x = std::max(rigid._min.x, vertices[i].x);
			rigid._min.y = std::max(rigid._min.y, vertices[i].y);
			rigid._min.z = std::max(rigid._min.z, vertices[i].z);
		}
	}

	rigids_.Add(e, rigid);
}

// predict if two AABBs will collide
bool CollisonSystem::IsCollide(RigidBody r1, RigidBody r2, const glm::mat4& mat1, const glm::mat4& mat2)
{
	//transform to world coordinate
	TransformAABB(r1._min, r1._max, mat1);
	TransformAABB(r2._min, r2._max, mat2);

	float dt = 0.2;
	r1._max += r1.velocity * dt;
	r1._min += r1.velocity * dt;
	r2._max += r2.velocity * dt;
	r2._min += r2.velocity * dt;

	return !((r1._max.x < r2._max.x) || (r2._max.x < r1._min.x)
		|| (r1._max.y < r2._max.y) || (r2._max.y < r1._min.y)
		|| (r1._max.z < r2._max.z) || (r2._max.z < r1._min.z));
}

// compute velocity after collison
void CollisonSystem::ComputeCollison(RigidBody& r1, RigidBody& r2, const glm::mat4& mat1, const glm::mat4& mat2)
{
	// get center in world coordinate
	glm::vec3 min1 = r1._min;
	glm::vec3 max1 = r1._max;
	glm::vec3 min2 = r2._min;
	glm::vec3 max2 = r2._max;
	TransformAABB(min1, max1, mat1);
	TransformAABB(min2, max2, mat2);

	// compute normal direction
	glm::vec3 center1 = 0.5f * (max1 + min1);
	glm::vec3 center2 = 0.5f * (max2 + min2);
	glm::vec3 normal_dir = glm::normalize(center2 - center1);

	// normal velocity
	glm::vec3 v1_x = normal_dir * glm::dot(r1.velocity, normal_dir);
	glm::vec3 v2_x = normal_dir * glm::dot(r2.velocity, normal_dir);

	// tangential velocity
	glm::vec3 v1_y = r1.velocity - v1_x;
	glm::vec3 v2_y = r2.velocity - v2_x;

	// new normal velocity
	glm::vec3 u1_x = ((2 * r2.mass) * v2_x + (r1.mass - r2.mass) * v1_x) / (r1.mass + r2.mass);
	glm::vec3 u2_x = ((2 * r1.mass) * v1_x + (r2.mass - r1.mass) * v2_x) / (r1.mass + r2.mass);

	// updata new velocity
	r1.velocity = u1_x + v1_y;
	r2.velocity = u2_x + v2_y;
}

//transform AABB to world coordinate
void CollisonSystem::TransformAABB(glm::vec3& min, glm::vec3& max, const glm::mat4& mat)
{
	glm::vec3 v[8];
	v[0] = glm::vec3(min.x, max.y, max.z);
	v[1] = glm::vec3(min.x, min.y, max.z);
	v[2] = glm::vec3(max.x, min.y, max.z);
	v[3] = glm::vec3(max.x, max.y, max.z);
	v[4] = glm::vec3(max.x, max.y, min.z);
	v[5] = glm::vec3(max.x, min.y, min.z);
	v[6] = glm::vec3(min.x, min.y, min.z);
	v[7] = glm::vec3(min.x, max.y, min.z);

	//transform
	for (int i = 0; i < 8; i++) {
		glm::vec4 tmp(v[i], 1);
		tmp = mat * tmp;
		v[i] = glm::vec3(tmp.x, tmp.y, tmp.z);
	}
	//reset AABB
	ResetAABB(min, max);
	UpdateAABB(v, 8, min, max);
}


}

