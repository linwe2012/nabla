#include "assets.h"
#include "renderable.h"
#include "core/asset/bootstrap.h"
#include "editor/gui.h"
#include "containers/map.h"
#include <glm/gtx/matrix_decompose.hpp>
#include "stb_image.h"
#include "material.h"

namespace nabla {
using namespace renderer;
using Vec2 = ImVec2;
using Vec4 = ImVec4;
struct AssetsSystem::Data {
	
	struct Mesh {
		MeshHandle mesh;
		glm::mat4 transform;
	};

	struct Model {
		Vector<Mesh> meshes;
		MeshHandle preview; //TODO
		fs::path bin;
		bool operator<(const Model& rhs) const { return bin < rhs.bin; }
	};

	struct Texture {
		MaterialHandle texture;
		fs::path bin;
		Vec2 uv0 = Vec2(0.f, 0.f);
		Vec2 uv1 = Vec2(1.f, 1.f);
		bool clicked = false;
		bool operator<(const Model& rhs) const { return bin < rhs.bin; }
	};

	struct PBRTexture{
		MatrialSysterm::PBRMaterial pbr;
		bool clicked = false;
	};

	Map<std::string, Model> models;
	Map<std::string, Texture> textures;
	Map<std::string, PBRTexture> pbr_textures;
	void ParseFromYaml(YAML::Node, SystemContext& ctx);
	void LoadOneTexture(Texture* _target, SystemContext& ctx);
	void LoadOneModel(Model* _target, SystemContext& ctx);
	void ParseFromYamlTexture(YAML::Node, SystemContext& ctx);
	bool use_builtin = false;
	Vec2 preview_image_size = Vec2(80.0f, 80.0f);
};



void AssetsSystem::OnGui(const Vector<Entity>& actives)
{
	if (ImGui::CollapsingHeader("Assets")) {
		if (ImGui::TreeNode("Models")) {
			ImGui::Checkbox("Use builtin loader", &data_->use_builtin);
			for (auto& itr : data_->models) {
				// ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Button,        (ImVec4)ImColor::HSV(205.0f / 360.0f, 1.0f, 0.39f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(205.0f / 360.0f, 0.7f, 0.49f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive,  (ImVec4)ImColor::HSV(205.0f / 360.0f, 0.8f, 0.59f));
				if (ImGui::Button(itr.first.c_str())) {
					//TODO: Give a hint of path name
					data_->LoadOneModel(&itr.second, *ctx_);
					for (auto& mesh : itr.second.meshes) {
						Transform transform;
						glm::vec3 skew;
						glm::vec4 persp;
						glm::decompose(mesh.transform, transform.scale, transform.quaternion, transform.position, skew, persp);
						ctx_->render->Add(ctx_->entity_manager->Create(), mesh.mesh, transform);
					}
				}
				ImGui::PopStyleColor(3);
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Textures")) {
			int cnt = 0;
			bool enale_loading = true;
			for (auto& itr : data_->textures) {
				if (cnt % 4 != 0) {
					ImGui::SameLine();
				}
				
				if (itr.second.texture.IsNil() && enale_loading) {
					data_->LoadOneTexture(&itr.second, *ctx_);
					if (ctx_->clock->IsTimeout() && cnt != 0) {
						enale_loading = false;
					}
				}
				
				if (itr.second.texture.IsNil()) {
					continue;
				}

				auto& t = itr.second;
				t.clicked = ImGui::ImageButton((ImTextureID)renderer::OpenHandle(t.texture),
					data_->preview_image_size, t.uv0, t.uv1);
				if (t.clicked) {
					for (auto e : actives) {
						ctx_->material->AttachTexture(e, t.texture);
					}
				}
				++cnt;
			}
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("PBR")) {
			for (auto& pbrit : data_->pbr_textures) {
				auto& pbr = pbrit.second;
				if (pbr.pbr.albedo_map.IsNil()) {
					continue;
				}
				if (pbr.pbr.ao_map.IsNil()) {
					continue;
				}
				if (pbr.pbr.metallic_map.IsNil()) {
					continue;
				}
				if (pbr.pbr.roughness_map.IsNil()) {
					continue;
				}

				pbr.clicked = ImGui::Button(pbrit.first.c_str());
				if (pbr.clicked) {
					for (auto e : actives) {
						ctx_->material->AttachPBRTexture(e, pbr.pbr);
					}
				}
			}
			ImGui::TreePop();
		}
	}
}

void AssetsSystem::Data::ParseFromYaml(YAML::Node m, SystemContext& ctx) {
	if (!m.IsDefined() || m.IsNull() || !m.IsMap()) {
		//TODO: log warn
		return;
	}

	for (auto it = m.begin(); it != m.end(); ++it) {
		if (!it->second.IsMap()) {
			//TODO: log warn
			continue;
		}

		if (auto nbin = it->second["bin"];
			nbin.IsDefined() && !nbin.IsNull()) {
			if (models.count(it->first.as<std::string>())) {
				//TODO
			}
			auto& the_model = models[it->first.as<std::string>()];
			the_model.bin = fs::weakly_canonical(ctx.assets->cwd() / nbin.as<std::string>());
		}
		else {
			//TODO: log warn
			continue;
		}
	}
}

void AssetsSystem::Data::LoadOneTexture(Texture* _target, SystemContext& ctx)
{
	auto& t = *_target;
	int w, h, nch;

	if (!fs::exists(_target->bin)) {
		return;
	}


	unsigned char* data = stbi_load(t.bin.string().c_str(), &w, &h, &nch, 3);
	if (data == nullptr) {
		return;
	}

	t.texture = renderer::NewTexture(data, w, h, renderer::TextureFormat::kRGB);
	stbi_image_free(data);
}

void AssetsSystem::Data::LoadOneModel(Model* _target, SystemContext& ctx)
{
	Model& target = *_target;
	if (target.meshes.size() == 0) {
		ModelAsset model_asset;
		ModelAsset::Options opts(ModelAsset::kCompute);
		opts.use_builtin = use_builtin;
		model_asset.LoadModel(target.bin, opts, ctx.status->render_job);
		for (auto& mesh : model_asset.meshes()) {
			target.meshes.push_back(
				Data::Mesh{
					mesh.h_mesh,
					mesh.transform
				}
			);
		}
	}
}

renderer::MaterialHandle LoadATextureFromFile(std::string p) {
	using namespace renderer;
	int w, h, nch;

	if (!fs::exists(p)) {
		return MaterialHandle();
	}


	unsigned char* data = stbi_load(p.c_str(), &w, &h, &nch, 3);
	if (data == nullptr) {
		return MaterialHandle();
	}

	MaterialHandle res = renderer::NewTexture(data, w, h, renderer::TextureFormat::kRGB);
	stbi_image_free(data);
	return res;
}

void AssetsSystem::Data::ParseFromYamlTexture(YAML::Node text, SystemContext& ctx)
{
	if (!text.IsDefined() || text.IsNull() || !text.IsMap()) {
		return;
	}

	for (auto text_itr = text.begin(); text_itr != text.end(); ++text_itr) {
		if (text_itr->second.IsMap()) {
			if (text_itr->second["pbr"].IsDefined() && text_itr->second["pbr"].as<bool>()) {
				if (!text_itr->second["ao"].IsDefined()) {
					continue;
				}
				if (!text_itr->second["roughness"].IsDefined()) {
					continue;
				}
				if (!text_itr->second["metal"].IsDefined()) {
					continue;
				}
				if (!text_itr->second["albedo"].IsDefined()) {
					continue;
				}
				auto& pbr = pbr_textures[text_itr->first.as<std::string>()];
				
				auto albedo = fs::weakly_canonical(ctx.assets->cwd() / text_itr->second["albedo"].as<std::string>());
				pbr.pbr.albedo_map = LoadATextureFromFile(albedo.string());

				auto ao = fs::weakly_canonical(ctx.assets->cwd() / text_itr->second["ao"].as<std::string>());
				pbr.pbr.ao_map = LoadATextureFromFile(ao.string());

				auto metallic = fs::weakly_canonical(ctx.assets->cwd() / text_itr->second["metal"].as<std::string>());
				pbr.pbr.metallic_map = LoadATextureFromFile(metallic.string());

				auto roughness = fs::weakly_canonical(ctx.assets->cwd() / text_itr->second["roughness"].as<std::string>());
				pbr.pbr.roughness_map = LoadATextureFromFile(roughness.string());
			}
			else {
				//TODO
			}
			
			continue;
		}
		if (models.count(text_itr->first.as<std::string>())) {
			//TODO
		}
		auto& the_texture = textures[text_itr->first.as<std::string>()];

		the_texture.bin = fs::weakly_canonical(ctx.assets->cwd() / text_itr->second.as<std::string>());
	}
}

void AssetsSystem::Initialize(SystemContext& ctx)
{
	ctx_ = &ctx;
	data_ = new Data;
	data_->ParseFromYaml(ctx.assets->yaml_models(), ctx);
	data_->ParseFromYamlTexture(ctx.assets->yaml_textures(), ctx);
}

}




