#include "assets.h"
#include "renderable.h"
#include "core/asset/bootstrap.h"
#include "editor/gui.h"
#include "containers/map.h"
#include <glm/gtx/matrix_decompose.hpp>
namespace nabla {
using namespace renderer;

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


	Map<std::string, Model> models;
	void ParseFromYaml(YAML::Node, SystemContext& ctx);
	void LoadOneModel(Model* _target, SystemContext& ctx);
	bool use_builtin = false;
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

void AssetsSystem::Initialize(SystemContext& ctx)
{
	ctx_ = &ctx;
	data_ = new Data;
	data_->ParseFromYaml(ctx.assets->yaml_models(), ctx);
}

}




