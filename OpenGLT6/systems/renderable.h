#ifndef _NABLA_RENDERABLE_SYSTEM_H_
#define _NABLA_RENDERABLE_SYSTEM_H_

#include "isystem.h"
#include "core/renderer.h"

#include "components/primitive.h"

namespace nabla {

class RenderableSystem : public ISystem {
	struct RenderHandle_ {
		renderer::ShaderHandle hshader_;
		renderer::MaterialHandle hmodel_;
		renderer::MaterialHandle hentity_;
		int entity_attachment_id_;
	};
public:
	struct Renderable {
		Transform transform;
		renderer::MeshHandle hmesh;
		renderer::RenderPass pass = renderer::RenderPass::kForward;
		Entity lookback;
		std::string name;
	};
	
	void SetRenderPassShader(renderer::RenderPass pass, 
		renderer::ShaderHandle hshader,
		renderer::MaterialHandle hmodel,
		renderer::MaterialHandle hentity,
		int entity_attachment_id) {
		render_handles_.size_at_least((uint16_t)pass, RenderHandle_{});
		render_handles_[(uint16_t)pass] = RenderHandle_{
			hshader, hmodel, hentity, entity_attachment_id
		};
	}

	void OnGui(const Vector<Entity>& actives) override;

	bool Has(Entity e) const override {
		if (e.index() >= sparse_.size()) {
			return false;
		}
		if (sparse_[e.index()] == Entity::kInvalidIndex) {
			return false;
		}
		return true;
	};

	void Update(Clock& clock) override;

	const char* name() const override { return "Renderable"; }

	void Add(Entity, renderer::MeshHandle hmesh, Transform t = Transform(), renderer::RenderPass pass = renderer::RenderPass::kForward);

	const Renderable& GetRenderable(Entity) const;

	Transform* GetTransformEdit(Entity e) {
		if (!Has(e)) {
			return nullptr;
		}

		return &dense_[sparse_[e.index()]].transform;
	}

	void AttachBeforeRender(ISystem* sys) {
		before_render_.push_back(sys);
	}

	//TODO
	void Remove(Entity) override {}

	void Initilize() override {}

	void Update(Entity) override {}

	void Add(Entity) override {}

	std::array<glm::vec3, 2> GetAABB(Entity);

private:
	
	Vector<Entity::entity_t> sparse_;
	Vector<Renderable> dense_;
	Renderable invalid_;
	
	Vector<RenderHandle_> render_handles_;
	Vector<ISystem*> before_render_;
};

}
#endif // !_NABLA_RIGID_BODY_SYSTEM_H_

