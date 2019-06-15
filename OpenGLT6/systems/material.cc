#include "material.h"
#include "editor/gui.h"
#include "cppstring.h"
#include "renderable.h"

namespace nabla {
	
void MatrialSysterm::SetUpMaterialHandles(renderer::ShaderHandle shader, MatrialSysterm::Uniforms names)
{
	using namespace renderer;
#define GET_UNIFORM(name, type, ...) \
	h##name##_ = renderer::NewUniform(shader, names.##name, renderer::MaterialType::k##type); \
	NA_LEAVE_IF((void) 0, h##name##_.IsNil(), "Invalid Hanlde"); 

	NA_BUILTIN_MATERIAL_SYS_LIST(GET_UNIFORM)
#undef GET_UNIFORM
}



void MatrialSysterm::RegisterMaterial(std::string scoped_name, MaterialPrototype proto)
{
	auto pieces = Explode(scoped_name, '.');

	auto* base = &root_mat_scope_;

	int n_scopes = pieces.size() - 1;

	for (int i = 0; i < n_scopes; ++i) {
		auto& subs = base->subs;
		auto*  new_base = &subs[pieces[i]];
		new_base->super = base;
		base = new_base;
	}

	auto* target = &base->protos_[pieces.back()];
	target->super = base;
}



void MatrialSysterm::Add(Entity e, Material material)
{
	sparse_.size_at_least(e.index(), Entity::kInvalidIndex);
	sparse_[e.index()] = dense_.size();
	dense_.push_back(material);
}

void MatrialSysterm::Initialize(SystemContext& ctx)
{
	ctx.render->AttachBeforeRender(this);
}

void MatrialSysterm::OnGui(const Vector<Entity>& actives)
{
	using namespace renderer;
	if (actives.size() == 0) {
		return;
	}
	if (!ImGui::CollapsingHeader("Material")) {
		return;
	}
	for (auto e : actives) {
		if (!Has(e)) {
			return;
		}

		auto& m = dense_[sparse_[e.index()]];

		//void* data = m.data;
		//int i = 0;

		//for (auto h : m.proto->handles) {
			//auto str = m.proto->names[i].c_str();
			//switch (GetMaterialDecriptor(h).type)
			//{
			// case MaterialType::kSampler2D: // fall through
			// case MaterialType::kSamplerCubic:
			// 	UseTexture(h);
			// case MaterialType::kInt:
			// 	SetUniform(h, *(int*)data);
			// 	data = ((int*)data) + 1;
			// 	break;

			// case MaterialType::kFloat:
			// 	ImGui::DragFloat(str, (float*)data, 0.01f, 0.0f, 2.0f);
			// 	data = ((float*)data) + 1;
			// 	break;
			// case MaterialType::kVec3:
			// 	ImGui::ColorEdit3(str, (float*)data);
			// 	data = ((glm::vec3*)data) + 1;
			// 	break;
			// case MaterialType::kMat4:
			// 	SetUniform(h, *(glm::mat4*)data);
			// 	data = ((glm::mat4*)data) + 1;
			// 	break;
			//default:
				//break;
			//}
			//++i;
		//}
		

		ImGui::ColorEdit3("Diffuse", &m.diffuse[0]);
		ImGui::DragFloat("Specular", &m.specular, 0.01f, -1.0f, 2.0f);
		
		
		ImGui::Text("Physically Based Rendering");
		ImGui::ColorEdit3("Albedo", &m.albedo[0]);
		ImGui::DragFloat("Metallic", &m.metallic, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Roughness", &m.roughness, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Ambient Occulison", &m.ambient_occulsion, 0.01f, 0.0f, 1.0f);
	}
}



void MatrialSysterm::Update(Entity e)
{
	if (!Has(e)) {
		return;
	}
#define SET_UNI(name, ...) renderer::SetUniform(h##name##_, dense_[sparse_[e.index()]].##name);
	NA_BUILTIN_MATERIAL_SYS_LIST(SET_UNI)
#undef SET_UNI
}
/*
void MatrialSysterm::Update(Entity e)
{
	using namespace renderer;
	MaterialInstance& m = dense_[sparse_[e.index()]];
	void* data = m.data;

	for (auto h : m.proto->handles) {
		switch (GetMaterialDecriptor(h).type)
		{
		case MaterialType::kSampler2D: // fall through
		case MaterialType::kSamplerCubic:
			UseTexture(h);
		case MaterialType::kInt:
			SetUniform(h, *(int*)data);
			data = ((int*)data) + 1;
			break;
		case MaterialType::kFloat:
			SetUniform(h, *(float*)data);
			data = ((float*)data) + 1;
			break;
		case MaterialType::kVec3:
			SetUniform(h, *(glm::vec3*)data);
			data = ((glm::vec3*)data) + 1;
			break;
		case MaterialType::kMat4:
			SetUniform(h, *(glm::mat4*)data);
			data = ((glm::mat4*)data) + 1;
			break;
		default:
			break;
		}
	}
}*/

}