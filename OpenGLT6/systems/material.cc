#include "material.h"
#include "editor/gui.h"
#include "cppstring.h"
#include "renderable.h"

namespace nabla {
struct SimpMaterial {
	
};
using namespace renderer;

struct MatrialSysterm::Data {
	ShaderHandle hwith_texture;
	ShaderHandle hpbr;
	renderer::MaterialHandle h_specular;
	renderer::MaterialHandle hpbr_specular;
	renderer::MaterialHandle h_albedo;
	renderer::MaterialHandle h_metallic;
	renderer::MaterialHandle h_roughness;
	renderer::MaterialHandle h_ao;
	renderer::MaterialHandle h_model;
	renderer::MaterialHandle h_proj;
	renderer::MaterialHandle h_view;
	renderer::MaterialHandle h_entity;

	renderer::MaterialHandle hpbr_model;
	renderer::MaterialHandle hpbr_proj;
	renderer::MaterialHandle hpbr_view;
	renderer::MaterialHandle hpbr_entity;

	// renderer::MaterialHandle hpbr_albedo;
	// renderer::MaterialHandle hpbr_metallic;
	// renderer::MaterialHandle hpbr_roughness;
	// renderer::MaterialHandle hpbr_ao;
};							 

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
	data_ = new Data;
	auto& d = *data_;

	{
		Set<std::string> macros;
		macros.insert("NormalMap");
		macros.insert("Bitangent");
		macros.insert("Tangent");
		macros.insert("Specular");
		macros.insert("TexCoords");
		macros.insert("AlbedoMap");
		macros.insert("MetallicMap");
		macros.insert("RoughnessMap");
		macros.insert("AOMap");

		d.hpbr = NewShader({
			"test/ubershaders/gbuffer.vs",
			"test/ubershaders/gbuffer.fs"
			}, macros);
		auto s = OpenHandle(d.hpbr);
		s.Use();
		s.SetInt("AlbedoMap", 0);
		s.SetInt("MetallicMap", 1);
		s.SetInt("RoughnessMap", 2);
		s.SetInt("AOMap", 3);
		NA_ASSERT(glGetError() == 0);
		
		d.hpbr_specular = NewUniform(d.hpbr, "Specular", MaterialType::kVec3);
		d.hpbr_entity = NewUniform(d.hpbr, "Entity", MaterialType::kVec3);
		d.hpbr_model = NewUniform(d.hpbr, "model", MaterialType::kMat4);
		d.hpbr_view = NewUniform(d.hpbr, "view", MaterialType::kMat4);
		d.hpbr_proj = NewUniform(d.hpbr, "projection", MaterialType::kMat4);
	}

	{
		Set<std::string> macros;
		macros.insert("NormalMap");
		macros.insert("Bitangent");
		macros.insert("Tangent");
		macros.insert("TexCoords");
		macros.insert("DiffuseMap");
		macros.insert("Specular");
		d.hwith_texture = NewShader({
			"test/ubershaders/gbuffer.vs",
			"test/ubershaders/gbuffer.fs"
			}, macros);
		auto s = OpenHandle(d.hwith_texture);
		s.Use();
		s.SetInt("DiffuseMap", 0);
		d.h_albedo = NewUniform(d.hwith_texture, "Albedo", MaterialType::kVec3);
		d.h_metallic = NewUniform(d.hwith_texture, "Metallic", MaterialType::kFloat);
		d.h_roughness = NewUniform(d.hwith_texture, "Roughness", MaterialType::kFloat);
		d.h_ao = NewUniform(d.hwith_texture, "AO", MaterialType::kFloat);
		d.h_specular = NewUniform(d.hwith_texture, "Specular", MaterialType::kVec3);
		d.h_entity = NewUniform(d.hwith_texture, "Entity", MaterialType::kVec3);
		d.h_model = NewUniform(d.hwith_texture, "model", MaterialType::kMat4);
		d.h_view = NewUniform(d.hwith_texture, "view", MaterialType::kMat4);
		d.h_proj = NewUniform(d.hwith_texture, "projection", MaterialType::kMat4);
		NA_ASSERT(glGetError() == 0);
	}

	ctx_ = &ctx;
}

void MatrialSysterm::AttachTexture(Entity e, renderer::MaterialHandle t)
{

	if (!ctx_->render->Has(e)) {
		return;
	}
	auto& d = *data_;
	ctx_->render->SetShader(e, data_->hwith_texture, d.h_entity, d.h_model);
	if (!Has(e)) {
		Add(e);
	}
	dense_[sparse_[e.index()]].texture = t;
}

void MatrialSysterm::AttachPBRTexture(Entity e, PBRMaterial pbrs)
{
	if (!ctx_->render->Has(e)) {
		return;
	}
	auto& d = *data_;
	ctx_->render->SetShader(e, data_->hpbr, d.hpbr_entity, d.hpbr_model);
	if (!Has(e)) {
		Add(e);
	}

	dense_[sparse_[e.index()]].pbr_map = pbrs;
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
		

		if (m.texture.IsNil()) {
			ImGui::ColorEdit3("Diffuse", &m.diffuse[0]);
		}
		else {
			ImGui::Text("Using diffuse texture");
			if (ImGui::Button("Delete")) {
				ctx_->render->SetShader(e, ShaderHandle(), MaterialHandle(), MaterialHandle());
				m.texture = MaterialHandle();
			}
		}
		
		ImGui::DragFloat("Specular", &m.specular, 0.01f, -1.0f, 2.0f);
		
		
		
		if (m.pbr_map.albedo_map.IsNil()) {
			ImGui::Text("Physically Based Rendering");
			ImGui::ColorEdit3("Albedo", &m.albedo[0]);
			ImGui::DragFloat("Metallic", &m.metallic, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Roughness", &m.roughness, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Ambient Occulison", &m.ambient_occulsion, 0.01f, 0.0f, 1.0f);
		}
		else {
			ImGui::Text("Physically Based Rendering -- Using Texture");
			if (ImGui::Button("Delete")) {
				ctx_->render->SetShader(e, ShaderHandle(), MaterialHandle(), MaterialHandle());
				dense_[sparse_[e.index()]].pbr_map.albedo_map = MaterialHandle();
			}
		}
		
	}
}



void MatrialSysterm::Update(Entity e)
{
	if (!Has(e)) {
		return;
	}

	auto& m = dense_[sparse_[e.index()]];
	auto& d = *data_;
	if (m.texture.IsNil() && m.pbr_map.albedo_map.IsNil()) {
		SetUniform(hdiffuse_, m.diffuse);
		SetUniform(hspecular_, m.specular);
		SetUniform(halbedo_, m.albedo);
		SetUniform(hmetallic_, m.metallic);
		SetUniform(hroughness_, m.roughness);
		SetUniform(hambient_occulsion_, m.ambient_occulsion);
	}
	else if (m.pbr_map.albedo_map.IsNil()) {
		UseShader(data_->hwith_texture);
		SetUniform(d.h_proj, GetGlobalProjectionMatrix());
		SetUniform(d.h_view, GetGlobalViewMatrix());
		// UseShader(data_->hwith_texture);
		UseTexture(m.texture, 0);
		SetUniform(d.h_specular, m.specular);
		SetUniform(d.h_albedo, m.albedo);
		SetUniform(d.h_metallic, m.metallic);
		SetUniform(d.h_roughness, m.roughness);
		SetUniform(d.h_ao, m.ambient_occulsion);
	}
	else {
		UseShader(data_->hpbr);
		SetUniform(d.hpbr_proj, GetGlobalProjectionMatrix());
		SetUniform(d.hpbr_view, GetGlobalViewMatrix());
		auto& p = m.pbr_map;
		SetUniform(d.hpbr_specular, m.specular);
		UseTexture(p.albedo_map, 0);
		UseTexture(p.metallic_map, 1);
		UseTexture(p.roughness_map, 2);
		UseTexture(p.ao_map, 3);
	}

	//SetUniform(hspecular_, m.specular);
	//if (m.pbr_map.albedo_map.IsNil()) {
	//	SetUniform(halbedo_, m.albedo);
	//	SetUniform(hmetallic_, m.metallic);
	//	SetUniform(hroughness_, m.roughness);
	//	SetUniform(hambient_occulsion_, m.ambient_occulsion);
	//}
	//else {
	//	
	//	auto& p = m.pbr_map;
	//	UseTexture(p.albedo_map, 0);
	//	UseTexture(p.metallic_map, 1);
	//	UseTexture(p.roughness_map, 2);
	//	UseTexture(p.ao_map, 3);
	//}
	

	

// #define SET_UNI(name, ...) renderer::SetUniform(h##name##_, dense_[sparse_[e.index()]].##name);
// 	NA_BUILTIN_MATERIAL_SYS_LIST(SET_UNI)
// #undef SET_UNI
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