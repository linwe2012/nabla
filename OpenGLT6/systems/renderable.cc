#include "renderable.h"
#include "editor/gui.h"
#include "glm/gtc/matrix_transform.hpp"

namespace nabla {

RenderableSystem* gptrRenderableSys = nullptr;
EntityManager* gptrEntityManager = nullptr;
void SetRenderable(RenderableSystem* render) { gptrRenderableSys = render; }
void SetEntityManager(EntityManager* manager) { gptrEntityManager = manager; }
RenderableSystem* GetRenderable() { return gptrRenderableSys; }
EntityManager* GetEntityManager() { return gptrEntityManager; }

void RenderableSystem::OnGui(const Vector<Entity>& actives)
{
	for (auto e : actives) {
		if (!Has(e))
			continue;
		auto& rend = dense_[sparse_[e.index()]];
		auto& r = rend.transform;

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
		model = glm::scale(model, r.transform.scale);
		model = glm::mat4_cast(r.transform.quaternion) * model;
		model = glm::translate(model, r.transform.position);

		for (auto sys : before_render_) {
			sys->Update(r.lookback);
		}
		glm::vec3 fentity;
		fentity.x = (r.lookback.index() & 0xFF) / ((float)255.0f);
		fentity.y = ((r.lookback.index() & 0xFF00) >> 8) / ((float)255.0f);
		fentity.z = ((r.lookback.index() & 0xFF0000) >> 16) / ((float)255.0f);
		SetUniform(info.hentity_, fentity);
		SetUniform(info.hmodel_, model);
		DrawMesh(r.hmesh);
	}
}


void RenderableSystem::Add(Entity e, renderer::MeshHandle hmesh, Transform t, renderer::RenderPass pass)
{
	sparse_.size_at_least(e.index(), Entity::kInvalidIndex);

	sparse_[e.index()] = dense_.size();
	dense_.push_back(Renderable{
		t,
		hmesh,
		pass,
		e,
		});
}

const RenderableSystem::Renderable& RenderableSystem::GetRenderable(Entity e) const
{
	NA_LEAVE_IF(invalid_, !Has(e), "No such entitiy");
		
	return dense_[sparse_[e.index()]];
}

const std::shared_ptr<RenderableSystem::VertexData> RenderableSystem::GetVertices(Entity e)
{
	if (!Has(e)) {
		return std::shared_ptr<VertexData>(nullptr);
	}

	auto mesh = renderer::OpenHandle(GetRenderable(e).hmesh);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	void* data = glMapBufferRange(GL_ARRAY_BUFFER, 0, mesh.num_vertices * sizeof(glm::vec3), GL_MAP_READ_BIT);
	NA_ASSERT(glGetError() == 0);

	glm::vec3* vecs = reinterpret_cast<glm::vec3*>(data);
	return std::shared_ptr<VertexData>(new VertexData{ vecs , mesh.num_vertices, mesh.vbo });
}




RenderableSystem::VertexData::~VertexData()
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	NA_ASSERT(glGetError() == 0);
}

}