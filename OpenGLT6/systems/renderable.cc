#include "renderable.h"
#include "editor/gui.h"
#include "glm/gtc/matrix_transform.hpp"

namespace nabla {



void RenderableSystem::OnGui(const Vector<Entity>& actives)
{
	for (auto e : actives) {
		if (!Has(e))
			continue;
		auto& rend = dense_[sparse_[e.index()]];
		auto& r = rend.rigid;

		ImGui::DragFloat("Pos.x", &r.position.x, 0.02f);
		ImGui::DragFloat("Pos.y", &r.position.y, 0.02f);
		ImGui::DragFloat("Pos.z", &r.position.z, 0.02f);

		ImGui::DragFloat("Quan.w", &r.quaternion.w, 0.02f);
		ImGui::DragFloat("Quan.x", &r.quaternion.x, 0.02f);
		ImGui::DragFloat("Quan.y", &r.quaternion.y, 0.02f);
		ImGui::DragFloat("Quan.z", &r.quaternion.z, 0.02f);

		ImGui::DragFloat("Scale.x", &r.scale.x, 0.02f);
		ImGui::DragFloat("Scale.y", &r.scale.y, 0.02f);
		ImGui::DragFloat("Scale.z", &r.scale.z, 0.02f);
	}
}

void RenderableSystem::Update(Clock& clock) {
	using namespace renderer;
	
	ShaderHandle hshader;
	for (auto& r : dense_) {
		auto& info = render_handles_[(uint16_t)r.pass];
		if (hshader != info.hshader_) {
			hshader = info.hshader_;
			UseShader(hshader);
		}
		glm::mat4 model(1.0f);
		model = glm::scale(model, r.rigid.scale);
		model = glm::mat4_cast(r.rigid.quaternion) * model;
		model = glm::translate(model, r.rigid.position);

		for (auto sys : before_render_) {
			sys->Update(r.lookback);
		}

		SetUniform(info.hmodel_, model);
		DrawMesh(r.hmesh);
	}
}


void RenderableSystem::Add(Entity e, renderer::MeshHandle hmesh, RigidBody r, renderer::RenderPass pass)
{
	sparse_.size_at_least(e.index(), Entity::kInvalidIndex);

	sparse_[e.index()] = dense_.size();
	dense_.push_back(Renderable{
		r,
		hmesh,
		pass,
		e,
		});
}

const RigidBody& RenderableSystem::GetRigid(Entity e) const
{
	NA_LEAVE_IF(invalid_.rigid, !Has(e), "No such entitiy");
		
	return dense_[sparse_[e.index()]].rigid;
}




}