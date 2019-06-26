#include "model.h"

#pragma warning( push )
#pragma warning( disable: 26495 )

#include "glm.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#pragma warning(pop)

#include "stb_image_impl.h"

namespace nabla {
using renderer::LayoutInfo;
using renderer::MemoryInfo;
using Texture = renderer::MaterialHandle;


void ModelAsset::LoadModel(const fs::path abs_path, Options opt, std::mutex& render_mutex) {
	dir_ = abs_path;
	std::string str = abs_path.string();
	opt_ = opt;

	render_mutex_ = &render_mutex;

	if (opt.use_builtin) {
		LoadObj(abs_path);
		return;
	}

	Assimp::Importer importer;

	int flag = aiProcess_Triangulate | aiProcess_FlipUVs;

	if (opt.NormalMapMethod == LoadMethod::kCompute) {
		flag |= aiProcess_GenNormals;

	}

	if (opt.TangentAndBitangentMapMethod == LoadMethod::kCompute) {
		flag |= aiProcess_CalcTangentSpace;
	}

	const aiScene* scene = importer.ReadFile(
		str.c_str(),
		flag);

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
	transform = parent_transform * (*(glm::mat4*)(&node->mTransformation));
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

		ProcessMesh(mesh, scene);
	}

	auto last_transform = parent_transform;
	parent_transform = transform;
	for (unsigned int i = 0; i < node->mNumChildren; ++i) {
		ProcessNode(node->mChildren[i], scene, hierachy + 1);
	}
	parent_transform = last_transform;
}


void ModelAsset::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
	static_assert(sizeof(mesh->mVertices[0]) == sizeof(glm::vec3), "Unable to convert aiVector to glm vector");
	static_assert(offsetof(aiVector3D, x) == offsetof(glm::vec3, x), "aiVector's internal layout doen't match glm::vec3");
	static_assert(offsetof(aiVector3D, y) == offsetof(glm::vec3, y), "aiVector's internal layout doen't match glm::vec3");
	static_assert(offsetof(aiVector3D, z) == offsetof(glm::vec3, z), "aiVector's internal layout doen't match glm::vec3");

	meshes_.construct_back();

	size_t cnt = 0;
	size_t id = 0;
	// size by float
	size_t sz_vec3 = (sizeof(glm::vec3) / sizeof(float));
	size_t sz_vec2 = (sizeof(glm::vec2) / sizeof(float));

	Vector<LayoutInfo> layouts;

	auto& m = meshes_.back();
	m.transform = transform;

	m.has_position = true;
	cnt += sz_vec3;

	{
		auto beg = reinterpret_cast<glm::vec3*>(mesh->mVertices);
		auto end = beg + mesh->mNumVertices;
		m.Position.insert(m.Position.end(), beg, end);
	}


	Vector<float> soup;
	soup.reserve(cnt * mesh->mNumVertices);


	{

		auto size = static_cast<uint32_t>(sizeof(float) * (soup.end() - soup.begin()));
		layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(id++, size));
		auto beg = reinterpret_cast<float*>(mesh->mVertices);
		auto end = beg + mesh->mNumVertices * sz_vec3;
		soup.insert(soup.end(), beg, end);
	}


	{
		m.has_normal = false;
		auto size = static_cast<uint32_t>(sizeof(float) * (soup.end() - soup.begin()));
		auto beg = reinterpret_cast<float*>(mesh->mNormals);
		auto end = beg + mesh->mNumVertices * sz_vec3;

		switch (opt_.NormalMapMethod)
		{
		case LoadMethod::kIgnore:
			break;
		case LoadMethod::kCompute: // fall through
		case LoadMethod::kAuto:
			if (!mesh->HasNormals()) {
				break;
			}
			layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(id++, size));
			soup.insert(soup.end(), beg, end);
			m.has_normal = true;
			break;

		case LoadMethod::kReserve:
			layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(id++, size));
			soup.resize(soup.size() + mesh->mNumVertices * sz_vec3);
			m.has_normal = true;
			break;

		default:
			break;
		}
	}

	{
		auto size = static_cast<uint32_t>(sizeof(float) * (soup.end() - soup.begin()));
		auto beg = reinterpret_cast<float*>(mesh->mTangents);
		auto end = beg + mesh->mNumVertices * sz_vec3;

		m.has_tangent_bitangent = false;
		switch (opt_.TangentAndBitangentMapMethod)
		{
		case LoadMethod::kIgnore:
			break;
		case LoadMethod::kCompute: // fall through
		case LoadMethod::kAuto:
			if (!mesh->HasTangentsAndBitangents()) {
				break;
			}

			layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(id++, size));

			soup.insert(soup.end(), beg, end);

			size = static_cast<uint32_t>(sizeof(float) * (soup.end() - soup.begin()));
			layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(id++, size));
			beg = reinterpret_cast<float*>(mesh->mBitangents);
			end = beg + mesh->mNumVertices * sz_vec3;
			soup.insert(soup.end(), beg, end);

			m.has_tangent_bitangent = true;
			break;
		case LoadMethod::kReserve:
			layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(id++, size));
			soup.resize(soup.size() + mesh->mNumVertices * sz_vec3 * 2);
			m.has_tangent_bitangent = true;
			break;

		default:
			break;
		}
	}

	{
		auto size = static_cast<uint32_t>(sizeof(float) * (soup.end() - soup.begin()));

		m.has_tex_coords = false;
		switch (opt_.TextureCoordMapMethod)
		{
		case LoadMethod::kIgnore:
			break;
		case LoadMethod::kCompute: // fall through
		case LoadMethod::kAuto:
			layouts.push_back(LayoutInfo::CreatePacked<glm::vec2>(id++, size));
			if (!mesh->HasTextureCoords(0)) {
				break;
			}
			for (size_t i = 0; i < mesh->mNumVertices; ++i) {
				soup.push_back(mesh->mTextureCoords[0][i].x);
				soup.push_back(mesh->mTextureCoords[0][i].y);
			}
			m.has_tex_coords = true;

			break;
		case LoadMethod::kReserve:
			layouts.push_back(LayoutInfo::CreatePacked<glm::vec2>(id++, size));
			soup.resize(soup.size() + mesh->mNumVertices * sz_vec2);
			m.has_tex_coords = true;
		default:
			break;
		}
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

	{
		std::scoped_lock render_lock(*render_mutex_);
		m.h_mesh = renderer::NewMesh(MemoryInfo{ soup.begin(), soup.size() * sizeof(float) },
			MemoryInfo{ indices.begin(), indices.size() * sizeof(unsigned int) },
			layouts
		);

		ProcessMaterialTexture(&meshes_.back(), scene, mesh);
	}
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
			path = fs::weakly_canonical(dir / path);
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


void ModelAsset::LoadObj(const fs::path abs_path)
{
	std::string path = abs_path.string();
	std::ifstream fin(path);
	if (!fin.is_open()) {
		std::cerr << "[load error]cannot open file\n";
		return;
	}

	int points_cnt = 0;
	int normal_cnt = 0;
	int texture_cnt = 0;
	while (!fin.eof()) {
		LoadObjMesh(fin, &points_cnt, &texture_cnt, &normal_cnt);
	}
	fin.close();

}

void ModelAsset::LoadObjMesh(std::ifstream& fin, int* points_cnt, int* texture_cnt, int* normal_cnt)
{
	Vector<float> soup;

	meshes_.push_back(Mesh());
	Mesh& m = meshes_.back();

	Vector<glm::vec3> points;
	Vector<glm::vec3> normal;
	Vector<glm::vec3> tangents;   // fill in zeros
	Vector<glm::vec3> bitangents; // fill in zeros
	Vector<glm::vec2> textCoords;
	Vector<unsigned>  indices;
	Vector<glm::vec3> points_idx;
	Vector<glm::vec3> normal_idx;
	Vector<glm::vec2> textCoords_idx;
	std::vector<Vertex> vertices;

	int flag = -1;
	glm::vec3 v;
	std::string tmp;
	//Parsing text
	while (!fin.eof()) {
		fin >> tmp;
		if (tmp == "v") {
			if (flag == -1)
				flag = 0;
			if (flag == 1) {
				(*points_cnt) += points_idx.size();
				(*normal_cnt) += normal_idx.size();
				(*texture_cnt) += textCoords_idx.size();
				fin.seekg(-1, std::ios::cur);
				break;
			}
			fin >> v[0] >> v[1] >> v[2];
			points_idx.push_back(v);
		}
		else if (tmp == "vt") {
			fin >> v[0] >> v[1];
			textCoords_idx.push_back(glm::vec2(v[0], v[1]));
			getline(fin, tmp);
		}
		else if (tmp == "vn") {
			fin >> v[0] >> v[1] >> v[2];
			normal_idx.push_back(v);
		}
		else if (tmp == "g") {
			flag = 1;
		}
		else if (tmp == "f") {
			flag = 1;
			std::vector<std::string> vs(3);
			fin >> vs[0] >> vs[1] >> vs[2];
			glm::ivec3 iv;

			if (sscanf(vs[0].c_str(), "%d/%d/%d", &iv[0], &iv[1], &iv[2]) == 3) {
				glm::ivec3 p_idx, t_idx, n_idx;
				for (int i = 0; i < 3; ++i)
					sscanf(vs[i].c_str(), "%d/%d/%d", &p_idx[i], &t_idx[i], &n_idx[i]);
				p_idx -= *points_cnt;
				t_idx -= *texture_cnt;
				n_idx -= *normal_cnt;
				for (int i = 0; i < 3; ++i) {
					Vertex v;
					v.pos = points_idx[p_idx[i] - 1];
					v.tex = textCoords_idx[t_idx[i] - 1];
					if (n_idx[i] != 0) {
						v.nor = normal_idx[n_idx[i] - 1];
					}
					else {
						v.nor = glm::normalize(v.pos);
					}

					vertices.push_back(v);
				}
			}
			else if (sscanf(vs[0].c_str(), "%d/%d/%d", &iv[0], &iv[1], &iv[2]) == 2) {
				glm::ivec3 p_idx, n_idx;
				for (int i = 0; i < 3; ++i)
					sscanf(vs[i].c_str(), "%d//%d", &p_idx[i], &n_idx[i]);
				p_idx -= *points_cnt;
				n_idx -= *normal_cnt;
				for (int i = 0; i < 3; ++i) {
					Vertex v;
					v.pos = points_idx[p_idx[i] - 1];
					v.tex = glm::vec2(0);
					if (n_idx[i] != 0) {
						v.nor = normal_idx[n_idx[i] - 1];
					}
					else {
						v.nor = glm::normalize(v.pos);
					}
					vertices.push_back(v);
				}
			}
		}
		else
			getline(fin, tmp);
	}

	for (int i = 0; i < vertices.size(); i++) {
		glm::vec3 v = { 0.0f,0.0f,0.0f };
		tangents.push_back(v);
		bitangents.push_back(v);
		points.push_back(vertices[i].pos);
		normal.push_back(vertices[i].nor);
		textCoords.push_back(vertices[i].tex);
		indices.push_back(i);
	}

	using namespace renderer;
	Vector<LayoutInfo> layouts;
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(0, soup.size() * sizeof(float)));
	soup.insert(soup.end(), reinterpret_cast<float*>(points.begin()), reinterpret_cast<float*>(points.end()));

	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(1, soup.size() * sizeof(float)));
	soup.insert(soup.end(), reinterpret_cast<float*>(normal.begin()), reinterpret_cast<float*>(normal.end()));

	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(2, soup.size() * sizeof(float)));
	soup.insert(soup.end(), reinterpret_cast<float*>(tangents.begin()), reinterpret_cast<float*>(tangents.end()));

	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(3, soup.size() * sizeof(float)));
	soup.insert(soup.end(), reinterpret_cast<float*>(bitangents.begin()), reinterpret_cast<float*>(bitangents.end()));

	layouts.push_back(LayoutInfo::CreatePacked<glm::vec2>(4, soup.size() * sizeof(float)));
	soup.insert(soup.end(), reinterpret_cast<float*>(textCoords.begin()), reinterpret_cast<float*>(textCoords.end()));

	m.transform = glm::mat4(1.0f); // or anything you like
	m.h_mesh = NewMesh(
		MemoryInfo{ soup.begin(), soup.size() * sizeof(float) }, // vertices
		MemoryInfo{ indices.begin(), indices.size() * sizeof(unsigned) }, // indices
		layouts
	);

}

}