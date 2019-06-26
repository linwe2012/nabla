#ifndef _NABLA_CORE_ASSET_MODEL_LOADER_H_
#define _NABLA_CORE_ASSET_MODEL_LOADER_H_

#include <set>

#include "utils.h"
#include "filesystem.h"
#include "containers/vector.h"

#include "texture.h"

#include <mutex>
//TODO(L): Move that out!
struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;


namespace nabla {

class ModelAsset {
public:	
	enum LoadMethod : char {
		kAuto, // load if exits, not load if otherwise
		kIgnore,
		kCompute,
		kReserve,
	};

	struct Mesh {
		using Texture = renderer::MaterialHandle;
		bool has_position;
		bool has_normal;
		bool has_tex_coords;
		bool has_tangent_bitangent; /* they come together */
		
		renderer::MeshHandle h_mesh;
		Vector<glm::vec3> soup; /**< positon, normal, tex_coords, tangent, bitangent */

		Vector<glm::vec3> Position;
#define BUILTIN_TEXTURE_DECL(name, ...) Texture name##Map;
#define DO_NOTHING(...) 
		NA_BUILTIN_TEXTURE_LIST(BUILTIN_TEXTURE_DECL, BUILTIN_TEXTURE_DECL, DO_NOTHING)
#undef DO_NOTHING
#undef BUILTIN_TEXTURE_DECL
		Texture AmbientOcclusionMap;
		glm::mat4 transform;
		// Vector<TextureAsset> textures;
	};

	struct Options {
		Options() : use_builtin(false) {}
		Options(LoadMethod m) {
			NormalMapMethod = m;
			TangentAndBitangentMapMethod = m;
			TextureCoordMapMethod = m;
		}

		LoadMethod NormalMapMethod = LoadMethod::kAuto;
		LoadMethod TangentAndBitangentMapMethod = LoadMethod::kAuto;
		LoadMethod TextureCoordMapMethod = LoadMethod::kAuto;
		bool use_builtin = false;;
	};

	/** loads a model with supported ASSIMP extensions from file 
	and stores the resulting meshes in the meshes vector. 
	textures will be in gpu and vertices and others are still in cpu memory
	@note path must be absolute or relative to program launching pos
	*/
	void LoadModel(const char* abs_path, Options opt, std::mutex& render_mutex) {
		fs::path path = abs_path;

		LoadModel(path, opt, render_mutex);
	}

	void LoadModel(const fs::path abs_path, Options opt, std::mutex& render_mutex);

	const Vector<Mesh>& meshes() {
		return meshes_;
	}
	
private:

	/** processes a node in a recursive fashion.
	Processes each individual mesh located at the node
	and repeats this process on its children nodes (if any).
	*/
	void ProcessNode(aiNode* node, const aiScene* scene);

	void ProcessNode(aiNode* node, const aiScene* scene, int hierachy);

	void ProcessMesh(aiMesh* mesh, const aiScene* scene);

	void ProcessMaterialTexture(Mesh* dst, const aiScene* scene, const aiMesh* mesh);

	// void LoadObj(const fs::path abs_path);
	// 
	// void LoadObjMesh(std::ifstream& fin, int* points_cnt,int*, int*);

private:
	struct Vertex	{		glm::vec3 pos;		glm::vec3 nor;		glm::vec2 tex;	};	void LoadObj(const fs::path abs_path);	void LoadObjMesh(std::ifstream& fin, int* points_cnt, int* texture_cnt, int* normal_cnt);
	
	glm::mat4 transform;
	glm::mat4 parent_transform = glm::mat4(1.0f);
	Vector<Mesh> meshes_;
	fs::path dir_;
	Options opt_;
	std::mutex* render_mutex_;
};

}

#endif // !_NABLA_CORE_ASSET_MODEL_LOADER_H_
