#include "renderable.h"
#include "editor/gui.h"
#include "glm/gtc/matrix_transform.hpp"

namespace nabla {
struct RenderableSystem::Data {
	renderer::ShaderHandle wired_shader;
	renderer::MaterialHandle wired_projection;
	renderer::MaterialHandle wired_model;
	renderer::MaterialHandle wired_view;
	renderer::MaterialHandle wired_color;
};

RenderableSystem* gptrRenderableSys = nullptr;
EntityManager* gptrEntityManager = nullptr;
void SetRenderable(RenderableSystem* render) { gptrRenderableSys = render; }
void SetEntityManager(EntityManager* manager) { gptrEntityManager = manager; }
RenderableSystem* GetRenderable() { return gptrRenderableSys; }
EntityManager* GetEntityManager() { return gptrEntityManager; }

void RenderableSystem::OnGui(const Vector<Entity>& actives)
{
	Entity latest;
	latest.MarkNil();
	// last_actives_.clear();
	
	for (auto e : actives) {
		if (!Has(e))
			continue;
		latest = e;
		auto& rend = dense_[sparse_[e.index()]];
		rend.selected = Renderable::kSelected;
	}

	if (latest.IsNil()) {
		return;
	}
	auto& rend = dense_[sparse_[latest.index()]];
	rend.selected = Renderable::kLatestSelect;

	
	if (ImGui::CollapsingHeader("Transform")) {
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

		ImGui::Checkbox("Hide", &rend.hide);
	}
	
}

void RenderableSystem::Update(Clock& clock) {
	using namespace renderer;
	
	ShaderHandle hshader;
	for (auto& r : dense_) {
		if (r.hide) {
			continue;
		}


		auto& info = render_handles_[(uint16_t)r.pass];
		if (!r.shader.IsNil()) {
			//if (hshader != r.shader) {
				//hshader = r.shader;
				UseShader(r.shader);
			//}
		}
		else {//if (hshader != info.hshader_) {
			// hshader = info.hshader_;

			UseShader(info.hshader_);
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
		if (r.shader.IsNil()) {
			SetUniform(info.hentity_, fentity);
			SetUniform(info.hmodel_, model);
		}
		else {
			SetUniform(r.hentity, fentity);
			SetUniform(r.hmodel, model);
		}
		DrawMesh(r.hmesh);
		if (r.selected) {
			auto& d = *data_;
			ScopedState post(RenderPass::kPostProc);
			ScopedState state(State::Line);
			UseShader(d.wired_shader);
			SetUniform(d.wired_color, 
				(r.selected == Renderable::kLatestSelect) ? 
				latest_selected_color_ : prev_selected_color_
			);
			SetUniform(d.wired_projection, GetGlobalProjectionMatrix());
			SetUniform(d.wired_view, GetGlobalViewMatrix());
			SetUniform(d.wired_model, model);
			DrawMesh(r.hmesh);
		}
		r.selected = Renderable::kNotSelected;
	}
}

void RenderableSystem::SetShader(Entity e, renderer::ShaderHandle shader, renderer::MaterialHandle hentity, renderer::MaterialHandle hmodel)
{
	if (!Has(e)) {
		return;
	}
	auto& r = dense_[sparse_[e.index()]];

	r.shader = shader;
	r.hentity = hentity;
	r.hmodel = hmodel;

}


void RenderableSystem::Add(Entity e, renderer::MeshHandle hmesh, Transform t, renderer::RenderPass pass)
{
	sparse_.size_at_least(e.index(), Entity::kInvalidIndex);

	sparse_[e.index()] = dense_.size();
	dense_.push_back(Renderable{
		t,
		hmesh,
		renderer::ShaderHandle(),
		pass,
		e,
		});
}

const RenderableSystem::Renderable& RenderableSystem::GetRenderable(Entity e) const
{
	NA_LEAVE_IF(invalid_, !Has(e), "No such entitiy");
		
	return dense_[sparse_[e.index()]];
}

RenderableSystem::Renderable* RenderableSystem::GetRenderableEdit(Entity e)
{
	if (!Has(e)) {
		return nullptr;
	}
	return &dense_[sparse_[e.index()]];
}


void RenderableSystem::Initialize(SystemContext&)
{
	using namespace renderer;
	data_ = new Data();
	auto& d = *data_;
	d.wired_shader = NewShader({
		"nabla/shaders/select-wired.vs",
		"nabla/shaders/select-wired.fs"
		});
	d.wired_projection = NewUniform(d.wired_shader, "projection", MaterialType::kMat4);
	d.wired_view = NewUniform(d.wired_shader, "view", MaterialType::kMat4);
	d.wired_model = NewUniform(d.wired_shader, "model", MaterialType::kMat4);
	d.wired_color = NewUniform(d.wired_shader, "color", MaterialType::kVec4);
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