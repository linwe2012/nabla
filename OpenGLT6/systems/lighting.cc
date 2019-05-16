#include "lighting.h"
#include "glm/gtc/matrix_transform.hpp"
#include "editor/gui.h"

namespace nabla {
void LightingSystem::SetShader(renderer::ShaderHandle lightingpass) {
	hshader_ = lightingpass;
	hcamera_ = renderer::NewUniform(lightingpass, "viewPos", renderer::MaterialType::kVec3);
	hnum_point_ = renderer::NewUniform(lightingpass, "num_points", renderer::MaterialType::kInt);
	hnum_spot_ = renderer::NewUniform(lightingpass, "num_spots", renderer::MaterialType::kInt);

	renderer::OpenHandle(lightingpass).Use();
	renderer::OpenHandle(lightingpass).SetInt("gPosition", 0);
	renderer::OpenHandle(lightingpass).SetInt("gNormal", 1);
	renderer::OpenHandle(lightingpass).SetInt("gAlbedoSpec", 2);
}

// activities on gui, note that you can actually do nothing

void LightingSystem::OnGui(const Vector<Entity>& actives) {
	for (auto e : actives) {
		auto p = lights_.find(e);
		if (p == lights_.end()) {
			continue;
		}
		
		switch (p->second.type)
		{
		case Light::kPoint:
			DrawGuiPoint(p->second);
			break;
		case Light::kSpot:
			DrawGuiSpot(p->second);
			break;
		default:
			break;
		}
		
	}
}

void LightingSystem::DrawGuiPoint(Light& l)
{
	if (ImGui::TreeNode(l.name.c_str())) {
		ImGui::ColorPicker3("Color", &l.color[0]);
		ImGui::DragFloat("Position.x", &l.position.x, 0.5f);
		ImGui::DragFloat("Position.y", &l.position.y, 0.5f);
		ImGui::DragFloat("Position.z", &l.position.z, 0.5f);
		ImGui::DragFloat("Linear", &l.linear, 0.01f, 0.0f, 2.0f, "%.3f");
		ImGui::DragFloat("Quad", &l.quad, 0.0001f, 0.0f, 1.0f, "%.4f");

		ImGui::TreePop();
		ImGui::Separator();
	}
}

void LightingSystem::DrawGuiSpot(Light& l)
{
	if (ImGui::TreeNode(l.name.c_str())) {
		ImGui::ColorPicker3("Color", &l.color[0]);
		ImGui::DragFloat("Pos.x", &l.position.x, 0.5f);
		ImGui::DragFloat("Pos.y", &l.position.y, 0.5f);
		ImGui::DragFloat("Pos.z", &l.position.z, 0.5f);

		ImGui::DragFloat("Yaw", &l.yaw, 0.2f);
		ImGui::DragFloat("Pitch", &l.pitch, 0.2f);

		ImGui::DragFloat("Linear", &l.linear, 0.01f, -0.5f, 2.0f, "%.3f");
		ImGui::DragFloat("Quad", &l.quad, 0.0001f, 0.0f, 1.0f, "%.4f");

		ImGui::DragFloat("Inner Cutoff (Deg)", &l.cutoff, 0.1f, 0.0f, 180.0f);
		ImGui::DragFloat("Outer Cutoff (Deg)", &l.outer_cutoff, 0.1f, 0.0f, 180.0f);

		ImGui::TreePop();
		ImGui::Separator();
	}
}


void LightingSystem::Update(Clock& clock)
{
	using namespace renderer;
	rc_ = renderer::GetRenderContext();
	//rc_->NewTarget(MeshHandle::MakeNil())
		//.Attach(hshader_)
	SetUniform(hcamera_, camera_pos_);
	SetUniform(hnum_point_, num_point_);
	SetUniform(hnum_spot_, num_spot_);

	int npoint = 0;
	int nspot = 0;
	for (const auto& lp : lights_) {
		const auto& l = lp.second;
		switch (l.type)
		{
		case Light::kPoint:
			RenderPoint(npoint++, l);
			break;
		case Light::kSpot:
			RenderSpot(nspot++, l);
			break;
		default:
			break;
		}
	}

}

void LightingSystem::AddPointHandle(int id) {
	using namespace renderer;
	//TODO(L) Log error
	if (hpoint_.size() >= max_point_)
		return;
	PointHandle p;
	std::string base = "points[" + std::to_string(id) + "].";
	{
		std::string a = base + "Position";
		p.position = renderer::NewUniform(hshader_, a.c_str(), MaterialType::kVec3);
	}

	{
		std::string a = base + "Color";
		p.color = renderer::NewUniform(hshader_, a.c_str(), MaterialType::kVec3);
	}

	{
		std::string a = base + "Linear";
		p.linear = renderer::NewUniform(hshader_, a.c_str(), MaterialType::kFloat);
	}

	{
		std::string a = base + "Quadratic";
		p.quad = renderer::NewUniform(hshader_, a.c_str(), MaterialType::kFloat);
	}

	{
		std::string a = base + "Radius";
		p.radius = NewUniform(hshader_, a.c_str(), MaterialType::kFloat);
	}

	hpoint_.push_back(std::move(p));
}

void nabla::LightingSystem::AddSpotLightHandle(int id)
{
	using namespace renderer;
	//TODO(L) Log error
	if (hspot_.size() >= max_spot_)
		return;
	SpotHandle p;
	std::string base = "spots[" + std::to_string(id) + "].";
	{
		std::string a = base + "Position";
		p.position = renderer::NewUniform(hshader_, a.c_str(), MaterialType::kVec3);
	}

	{
		std::string a = base + "Color";
		p.color = renderer::NewUniform(hshader_, a.c_str(), MaterialType::kVec3);
	}

	{
		std::string a = base + "Linear";
		p.linear = NewUniform(hshader_, a.c_str(), MaterialType::kFloat);
	}

	{
		std::string a = base + "Quadratic";
		p.quad = renderer::NewUniform(hshader_, a.c_str(), MaterialType::kFloat);
	}

	{
		std::string a = base + "Cutoff";
		p.cutoff = renderer::NewUniform(hshader_, a.c_str(), MaterialType::kFloat);
	}

	{
		std::string a = base + "OuterCutoff";
		p.outer_cutoff = renderer::NewUniform(hshader_, a.c_str(), MaterialType::kFloat);
	}

	{
		std::string a = base + "Direction";
		p.direction = renderer::NewUniform(hshader_, a.c_str(), MaterialType::kVec3);
	}
	hspot_.push_back(std::move(p));
}


void LightingSystem::RenderPoint(int id, const Light& l)
{
	if (id == hpoint_.size()) {
		AddPointHandle(id);
	}

	auto& h = hpoint_[id];
	
	using namespace renderer;
	glm::mat4 model(1.0f);
	model = glm::scale(model, l.mesh_scale);
	model = glm::translate(model, l.position);

	// rc_->NewTarget(l.hmesh)
		//.Attach(hshader_)
		//.ModelMat(model)
	SetUniform(h.color, l.color);
	SetUniform(h.position, l.position);
	SetUniform(h.linear, l.linear);
	SetUniform(h.quad, l.quad);
	SetUniform(h.radius, l.radius);	
}

void LightingSystem::RenderSpot(int id, const Light& l)
{
	if (id == hspot_.size()) {
		AddSpotLightHandle(id);
	}

	auto& h = hspot_[id];
	using namespace renderer;
	glm::mat4 model(1.0f);
	model = glm::scale(model, l.mesh_scale);
	model = glm::translate(model, l.position);

	glm::vec3 direction;
	direction.y = glm::sin(glm::radians(l.pitch));
	direction.x = glm::cos(glm::radians(l.pitch)) * glm::cos(glm::radians(l.yaw));
	direction.z = glm::cos(glm::radians(l. pitch)) * glm::sin(glm::radians(l.yaw));

	float cutoff = glm::cos(glm::radians(l.cutoff));
	float outer_cutoff = glm::cos(glm::radians(l.outer_cutoff));

	//rc_->NewTarget(l.hmesh)
		//.ModelMat(model)
		SetUniform(h.position, l.position);
		SetUniform(h.direction, direction);
		SetUniform(h.color, l.color)	  ;
		SetUniform(h.linear, l.linear)	  ;
		SetUniform(h.quad, l.quad)		  ;
		SetUniform(h.cutoff, cutoff)	  ;
		SetUniform(h.outer_cutoff, outer_cutoff);
}



}






