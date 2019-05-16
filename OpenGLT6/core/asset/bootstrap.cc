#include "bootstrap.h"

#include "utils.h"
#include <sstream>

namespace nabla {

void AssetManager::ParseAssetsFromFile(const char* path)
{
	try {
		root_ = YAML::LoadFile(path);
		cwd_ = fs::absolute(path).parent_path();

		auto get_absolute = [this](YAML::Node node) -> std::string {
			std::string rel = node.as<std::string>();
			return fs::absolute(rel, cwd_).string();
		};

		auto mat_desc = root_["materials"];
		if (mat_desc.IsNull()) {
			NA_LOG_WARN("no materials descriptor in file: %s, is that intended?", path);
		}
		else {
			std::string mat_path = get_absolute(mat_desc);
			materials_ = YAML::LoadFile(mat_path);
		}

		auto model_desc = root_["models"];
		if (mat_desc.IsNull()) {
			NA_LOG_WARN("no models descriptor in file: %s, is that intended?", path);
		}
		else {
			std::string model_path = get_absolute(model_desc);
			ParseModelFromFile(model_path.c_str());
		}
	}
	catch (YAML::ParserException& e) {
		NA_LOG_ERROR("failed to load assets descriptor %s", e.what());
	}
	catch (YAML::BadFile & e) {
		NA_LOG_ERROR("can't open file %s", e.what());
	}
}


void AssetManager::ParseModelFromFile(const char* path)
{
	auto models = YAML::LoadFile(path);
	for (YAML::const_iterator it = models.begin(); it != models.end(); ++it) {
		if (!it->second.IsMap()) {
			NA_LOG_ERROR("expect model %s to be object", it->first.as<std::string>().c_str());
			continue;
		}
		ModelInfo model;

#define MODEL_INFO_MAN(type, name)          \
		if (it->second[#name].IsNull()) {   \
			NA_LOG_ERROR("expect model field " #name " not null in %s", it->first.as<std::string>().c_str()); \
			continue;                        \
		}                                    \
		else {                               \
			model. name = it->second[#name].as<type>(); \
		}

#define MODEL_INFO_NOT(type, name)          \
		if (it->second[#name].IsNull()) {   \
			NA_LOG_INFO("model field " #name " is missing in %s", it->first.as<std::string>().c_str()); \
		}                                    \
		else {                               \
			model. name = it->second[#name].as<type>(); \
		}
		
		NA_ASSET_MODEL_INFO_LIST(MODEL_INFO_MAN, MODEL_INFO_NOT);
#undef MODEL_INFO_MAN
#undef MODEL_INFO_NOT

		std::string id;
		if (it->second["id"].IsDefined()) {
			id = it->second["id"].as < std::string >();
		}
		else {
			id = it->first.as<std::string>();
		}
		model.abs_path = path;
		model.abs_path = model.abs_path.parent_path() / it->second["bin"].as<std::string>();
		models_[id] = std::move(model);
	}
}

LoadedModel AssetManager::LoadModelToGPU(const char* model_id_str, bool auto_shader)
{
	auto gotcha = models_.find(model_id_str);
	if (gotcha == models_.end()) {
		NA_LOG_ERROR("Unable to find model of id: %s", model_id_str);
		return LoadedModel();
	}
	return LoadModelToGPU(gotcha, auto_shader);
}

YAML::Node AssetManager::GetMaterial(const char* name)
{
	std::istringstream iss(name);
	std::string tok;
	
	auto node = materials_;

	while (std::getline(iss, tok, '.')) {
		if (node.IsNull() || !node.IsMap()) {
			return YAML::Node();
		}
		node = node[tok];
	}

	if (!node.IsMap()) {
		return YAML::Node();
	}

	return node;
}

template <typename T>
struct IsFloat : std::false_type {};

template<>
struct IsFloat<float> : std::true_type {};

template <typename T>
struct IsVec3 : std::false_type {};

template<>
struct IsVec3<glm::vec3> : std::true_type {};

LoadedModel AssetManager::LoadModelToGPU(std::map<std::string, ModelInfo>::iterator gotcha, bool auto_shader, MaterialOverride matover)
{
	if (gotcha->second.loaded) {
		NA_LOG_ERROR("model %s already loaded to gpu", gotcha->first.c_str());
		return LoadedModel();
	}

	auto& info = gotcha->second;
	ModelAsset model;
	model.LoadModel(info.abs_path);
	renderer::ShaderInfo shaderinfo;
	const auto& meshes = model.meshes();
	const auto material = GetMaterial(info.material.c_str());
	renderer::ShaderInfo sinfo;
	for (const auto& mesh : meshes) {
		LoadedModel lm;

		Set<std::string> macros;
		auto define_map = [&macros](bool ifoverride_map, renderer::MaterialHandle md, const char* fallback_uniform) {
			if (ifoverride_map) {
				macros.insert(fallback_uniform);
			}
			else {
				if (md.IsNil()) {

				}
			}
		};

		int total_pbrs = 0;
		int num_pbrs = 0;
		auto test_pbr_pass = [&material, &total_pbrs, &num_pbrs](bool ifoverride_map, renderer::MaterialHandle md, const char* fallback_uniform) -> bool {
			++total_pbrs;
			
			if (ifoverride_map || md.IsNil()) {
				if (!material.IsDefined()) {
					return false;
				}
				std::string uniform = fallback_uniform;
				std::transform(uniform.begin(), uniform.end(), uniform.begin(), std::tolower);
				auto node = material[uniform];
				if (!node.IsDefined()) {
					return false;
				}
			}
			++num_pbrs;
			return true;
		};

		bool enable_pbr = true;
#define DO_NOTHING(...)
#define TEST_PBR(name, ...) test_pbr_pass(matover.##name, renderer::MaterialHandle(), #name);
		NA_BUILTIN_TEXTURE_LIST(DO_NOTHING, DO_NOTHING, TEST_PBR)
#undef TEST_PBR

		if (total_pbrs != num_pbrs) {
			NA_LOG_WARN("PBR Materials partially defined, check material file or material override settings");
		}
#define DEFINE_MACROS_MAP(name, type, ainame) \
		

		// lm.vertex = renderer::NewVertexBuffer(mesh.position.begin(), mesh.position.size());
		{
			auto albedo = material["albedo"];
			
			if (material && albedo && albedo.IsSequence()) {
				if (albedo.size() != 3) {
					NA_LOG_ERROR("material %s's albedo is not vec3.", info.material.c_str());
				}
				else {
					glm::vec3 data;
					data.r = albedo[0].as<float>();
					data.g = albedo[1].as<float>();
					data.b = albedo[2].as<float>();
					// renderer::NewUniform()
				}
			}
		}
		

	}

	LoadedModel lm;
	LoadedModel::SingleModel sm;
	sm.hMesh = meshes[0].h_mesh;
	lm.meshes_.push_back(sm);
	return lm;
}



}

