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
	};
public:
	struct Renderable {
		RigidBody rigid;
		renderer::MeshHandle hmesh;
		renderer::RenderPass pass;
		Entity lookback;
		std::string name;
	};
	
	void SetRenderPassShader(renderer::RenderPass pass, renderer::ShaderHandle hshader, renderer::MaterialHandle hmodel) {
		render_handles_.size_at_least((uint16_t)pass, RenderHandle_{});
		render_handles_[(uint16_t)pass] = RenderHandle_{
			hshader, hmodel
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

	const char* name() const override { return "RigidBody"; }

	void Add(Entity, renderer::MeshHandle hmesh, RigidBody r = RigidBody(), renderer::RenderPass pass = renderer::RenderPass::KForward);

	const RigidBody& GetRigid(Entity) const;

	void AttachBeforeRender(ISystem* sys) {
		before_render_.push_back(sys);
	}

	//TODO
	void Remove(Entity) override {}

	void Initilize() override {}

	void Update(Entity) override {}

private:
	
	Vector<Entity::entity_t> sparse_;
	Vector<Renderable> dense_;
	Renderable invalid_;
	
	Vector<RenderHandle_> render_handles_;
	Vector<ISystem*> before_render_;
	
};

}
#endif // !_NABLA_RIGID_BODY_SYSTEM_H_
