#include "model.h"

#pragma warning( push )
#pragma warning( disable: 26495 )

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#pragma warning(pop)

#include "stb_image_impl.h"


namespace nabla {
using renderer::LayoutInfo;
using renderer::MemoryInfo;
using Texture = renderer::MaterialHandle;

void ModelAsset::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
	meshes_.construct_back();

	size_t cnt = 0;
	size_t id = 0;
	// size by float
	size_t sz_vec3 = (sizeof(glm::vec3) / sizeof(float));
	size_t sz_vec2 = (sizeof(glm::vec2) / sizeof(float));

	Vector<LayoutInfo> layouts;

	auto& m = meshes_.back();

	m.has_position = true;
	cnt += sz_vec3;

	// compute size info, so we can allocate memory only once
	if (mesh->HasNormals()) {
		m.has_normal = true;
		cnt += sz_vec3;
	}

	if (mesh->HasTangentsAndBitangents()) {
		m.has_tangent_bitangent = true;
		cnt += sz_vec3 * 2;
	}

	if (mesh->HasTextureCoords(0)) {
		m.has_tex_coords = true;
		cnt += sz_vec2;
	}

	Vector<float> soup;
	soup.reserve(cnt * mesh->mNumVertices);
	static_assert(sizeof(mesh->mVertices[0]) == sizeof(glm::vec3), "Unable to convert aiVector to glm vector");
	static_assert(offsetof(aiVector3D, x) == offsetof(glm::vec3, x), "aiVector's internal layout doen't match glm::vec3");
	static_assert(offsetof(aiVector3D, y) == offsetof(glm::vec3, y), "aiVector's internal layout doen't match glm::vec3");
	static_assert(offsetof(aiVector3D, z) == offsetof(glm::vec3, z), "aiVector's internal layout doen't match glm::vec3");

	{
		layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(id++, sizeof(float) * (soup.end() - soup.begin())));
		auto beg = reinterpret_cast<float*>(mesh->mVertices);
		auto end = beg + mesh->mNumVertices * sz_vec3;
		soup.insert(soup.end(), beg, end);
	}
	
	if (mesh->HasNormals()) {
		layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(id++, sizeof(float) * (soup.end() - soup.begin())));
		auto beg = reinterpret_cast<float*>(mesh->mNormals);
		auto end = beg + mesh->mNumVertices * sz_vec3;
		soup.insert(soup.end(), beg, end);
	}

	if (mesh->HasTangentsAndBitangents()) {
		layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(id++, sizeof(float) * (soup.end() - soup.begin())));
		auto beg = reinterpret_cast<float*>(mesh->mTangents);
		auto end = beg + mesh->mNumVertices * sz_vec3;
		soup.insert(soup.end(), beg, end);

		layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(id++, sizeof(float) * (soup.end() - soup.begin())));
		beg = reinterpret_cast<float*>(mesh->mBitangents);
		end = beg + mesh->mNumVertices * sz_vec3;
		soup.insert(soup.end(), beg, end);
	}

	if (mesh->HasTextureCoords(0)) {
		layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(id++, sizeof(float) * (soup.end() - soup.begin())));
		auto beg = reinterpret_cast<float*>(mesh->mTangents);
		auto end = beg + mesh->mNumVertices * sz_vec3;
		soup.insert(soup.end(), beg, end);
	}

	
	Vector<uint32_t> indices;
	
	// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		// retrieve all indices of the face and store them in the indices vector
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	m.h_mesh = renderer::NewMesh(MemoryInfo{ soup.begin(), soup.size() * sizeof(float) },
		MemoryInfo{ indices.begin(), indices.size() * sizeof(unsigned int) },
		layouts
	);

	ProcessMaterialTexture(&meshes_.back(), scene, mesh);
}


/** auxilary function used by ProcessMaterialTexture() to process a given type
	@param[in] dst the destination to be written to
*/
void ProcessMaterialTexturePass(Texture& dst,
	                            const aiMaterial* materials,
	                            BuiltinMaterial material_type,
	                            int ai_type,
                                const fs::path dir)
{
	aiTextureType type = static_cast<aiTextureType>(ai_type);
	int n_tex = materials->GetTextureCount(type);
	for (int i = 0; i < n_tex; ++i) {
		aiString str;
		materials->GetTexture(type, i, &str);
		fs::path path = str.C_Str();
		if (path.is_relative()) {
			path = fs::canonical(path, dir);
		}

		std::string file = path.string();

		dst = TextureAsset::LoadTexture(file, material_type).GetMaterialHandle();
	}
}


void ModelAsset::ProcessMaterialTexture(Mesh* dst, const aiScene* scene, const aiMesh* mesh)
{
	aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
#define PROCESS_PASS(name, type, macro) ProcessMaterialTexturePass(dst->name##Map, mat, BuiltinMaterial::k##name##Map, aiTextureType_##macro, dir_);
#define DO_NOTHING(...) 
	NA_BUILTIN_TEXTURE_LIST(PROCESS_PASS, PROCESS_PASS, DO_NOTHING);
#undef DO_NOTHING
	ProcessMaterialTexturePass(dst->AmbientOcclusionMap, mat, BuiltinMaterial::kAmbientOcclusionMap, aiTextureType_LIGHTMAP, dir_);
}


void ModelAsset::LoadModel(const fs::path abs_path) {
	dir_ = abs_path;
	std::string str = abs_path.string();
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(
		str.c_str(),
		aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

	// check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		NA_LOG_ERROR("ERROR::ASSIMP:: %s", importer.GetErrorString());
		return;
	}

	ProcessNode(scene->mRootNode, scene);
}


/** processes a node in a recursive fashion.
Processes each individual mesh located at the node
and repeats this process on its children nodes (if any).
*/

void ModelAsset::ProcessNode(aiNode* node, const aiScene* scene) {
	meshes_.reserve(scene->mNumMeshes);

	ProcessNode(node, scene, 0);
}

void ModelAsset::ProcessNode(aiNode* node, const aiScene* scene, int hierachy) {
	// meshes_.reserve(meshes_.size() + node->mNumMeshes);

	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		ProcessMesh(mesh, scene);
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i) {
		ProcessNode(node->mChildren[i], scene, hierachy + 1);
	}
}

}