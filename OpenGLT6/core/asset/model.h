#ifndef _NABLA_CORE_ASSET_MODEL_LOADER_H_
#define _NABLA_CORE_ASSET_MODEL_LOADER_H_

#include <set>

#include "utils.h"
#include "filesystem.h"
#include "containers/vector.h"

#include "texture.h"

//TODO(L): Move that out!
struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;


namespace nabla {

class ModelAsset {
public:	
	struct Mesh {
		using Texture = renderer::MaterialHandle;
		bool has_position;
		bool has_normal;
		bool has_tex_coords;
		bool has_tangent_bitangent; /* they come together */
		
		renderer::MeshHandle h_mesh;
		Vector<glm::vec3> soup; /**< positon, normal, tex_coords, tangent, bitangent */

#define LIST_TEXTURE(name, type, ainame) Texture name##Map;
#define DO_NOTHING(...) 
		NA_BUILTIN_TEXTURE_LIST(LIST_TEXTURE, LIST_TEXTURE, DO_NOTHING)
#undef  LIST_TEXTURE
#undef  DO_NOTHING

		Texture AmbientOcclusionMap;
		// Vector<TextureAsset> textures;
	};

	/** loads a model with supported ASSIMP extensions from file 
	and stores the resulting meshes in the meshes vector. 
	textures will be in gpu and vertices and others are still in cpu memory
	@note path must be absolute or relative to program launching pos
	*/
	void LoadModel(const char* abs_path) {
		fs::path path = abs_path;

		LoadModel(path);
	}

	void LoadModel(const fs::path abs_path);

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

private:
	Vector<Mesh> meshes_;
	fs::path dir_;
};

}

#endif // !_NABLA_CORE_ASSET_MODEL_LOADER_H_
