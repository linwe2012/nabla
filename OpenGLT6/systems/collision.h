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
		void Initialize(SystemContext& ctx) override;

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

		// predict if two AABBs will collide
		bool IsCollide(RigidBody r1, RigidBody  r2, const glm::mat4& mat1, const glm::mat4& mat2);

		// compute velocity after collison
		void ComputeCollison(RigidBody& r1, RigidBody& r2, const glm::mat4& mat1, const glm::mat4& mat2);

		//transform AABB to world coordinate
		void TransformAABB(glm::vec3& min, glm::vec3& max, const glm::mat4& mat);

		// update AABB of a set of vertices
		void UpdateAABB(const glm::vec3* v, size_t num, glm::vec3& min, glm::vec3& max)
		{
			for (size_t i = 0; i < num; i++) {
				if (v[i].x < min.x) min.x = v[i].x;
				if (v[i].y < min.y) min.y = v[i].y;
				if (v[i].z < min.z) min.z = v[i].z;
				if (v[i].x > max.x) max.x = v[i].x;
				if (v[i].y > max.y) max.y = v[i].y;
				if (v[i].z > max.z) max.z = v[i].z;
			}
		}

		// reset AABB
		void ResetAABB(glm::vec3& min, glm::vec3& max)
		{
			min = glm::vec3(99999.0f, 99999.0f, 99999.0f);
			max = glm::vec3(-99999.0f, -99999.0f, -99999.0f);
		}

		// octree etc.
	};
}


#endif // !_NABLA_SYSTEM_COLLISON_H_

