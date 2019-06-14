#ifndef _NABLA_CORE_ASSET_BOOTSTRAP_H_
#define  _NABLA_CORE_ASSET_BOOTSTRAP_H_

#include "containers/vector.h"
#include "containers/map.h"
#include "filesystem.h"

#include "yaml-cpp/yaml.h"

#include "model.h"

#include "core/renderer.h"

namespace nabla {
	// should uniform override the default texture map
	struct MaterialOverride {
#define UNIFORM_OVERRIDE_MAP(name, ...) bool name;
#define DO_NOTHING(...)
		NA_BUILTIN_TEXTURE_LIST(UNIFORM_OVERRIDE_MAP, DO_NOTHING, UNIFORM_OVERRIDE_MAP)
#undef UNIFORM_OVERRIDE_MAP
#undef DO_NOTHING
	};

	struct LoadedModel {
		Vector<BuiltinTextureCombo>meshes_;
	};

	class AssetManager {
	public:
		void ParseAssetsFromFile(const char* path);
		void ParseModelFromFile(const char* path);
		static renderer::MaterialHandle LoadTexture(const char* path);
		static renderer::MaterialHandle LoadTexture(const char* path, const char* p2);

		LoadedModel LoadModelToGPU(const char* model_id_str, bool auto_shader = true);

		struct ModelInfo {
#define NA_ASSET_MODEL_INFO_LIST(V/* manditory */, X)\
		V(std::string, bin)                          \
		X(std::string, material)      

#define DEF_MODEL_INFO(type, name) type name;
			NA_ASSET_MODEL_INFO_LIST(DEF_MODEL_INFO, DEF_MODEL_INFO)
#undef DEF_MODEL_INFO
				bool loaded = false;
			fs::path abs_path;
		};
		
		// guarantees return a object map
		YAML::Node GetMaterial(const char* name);

		renderer::MaterialHandle GetTexture(const char* name);

	private:
		LoadedModel LoadModelToGPU(std::map<std::string, ModelInfo>::iterator gotcha, bool auto_shader, MaterialOverride matover = MaterialOverride());

		Map<fs::path, YAML::Node> files_;
		YAML::Node materials_;
		YAML::Node textures_;
		std::map<std::string, ModelInfo> models_;
		YAML::Node root_;
		fs::path cwd_; /**< current working directory */
	};
}

#endif // !_NABLA_CORE_ASSET_BOOTSTRAP_H_
