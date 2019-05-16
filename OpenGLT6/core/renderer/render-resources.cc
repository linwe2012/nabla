#include "render-resources.h"
#include <regex>

namespace nabla {
namespace renderer {

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
///////                Materials              ///////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

class MaterialManger
{
public:

	MaterialManger(int size);

	MaterialHeader GetDecriptor(MaterialHandle mat);

	MaterialHandle NewTexture(const unsigned char* data,
		int width, int height,
		TextureFormat format);

	MaterialHandle NewUniform(ShaderHandle target, const char* name, MaterialType type);

	void DescribeHanlde(MaterialHandle md, RenderPass render_pass);

	uint32_t OpenHandle(MaterialHandle md);

	uint32_t GetMaterialSize(MaterialHandle md);

private:
	Vector<MaterialHeader> headers_;
	Vector<uint32_t> buffer_;
	uint32_t size_;
};

/////////////////////////////////////////////////////
///////           Materials  Manager          ///////
/////////////////////////////////////////////////////

inline MaterialManger::MaterialManger(int size)
	:size_(size)
{
	headers_.reserve(size);
	buffer_.reserve(size);
}

inline MaterialHeader MaterialManger::GetDecriptor(MaterialHandle mat) {
	return headers_[mat.index()];
}

MaterialHandle MaterialManger::NewTexture(const unsigned char* data,
	int width, int height,
	TextureFormat format) {
	uint32_t id;
	glGenTextures(1, &id);
	if (id == 0) {
		NA_LOG_ERROR("Unable to generate texture buffer");
		return MaterialHandle::MakeNil();
	}

	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	MaterialHeader header;
	header.type = MaterialType::kSampler;
	MaterialHandle md(headers_.size());
	buffer_.push_back(id);
	headers_.push_back(header);
	return md;
}

inline MaterialHandle MaterialManger::NewUniform(ShaderHandle target, const char* name, MaterialType type) {
	Shader shader = ::nabla::renderer::OpenHandle(target);
	int id = glGetUniformLocation(shader.ID, name);
	if (id < 0) {
		return MaterialHandle::MakeNil();
	}
	MaterialHeader header;
	header.type = type;
	header.hshader = target;
	// header.render_pass = 
	MaterialHandle md(headers_.size());
	buffer_.push_back(static_cast<uint32_t>(id));
	headers_.push_back(header);
	return md;
}

inline void MaterialManger::DescribeHanlde(MaterialHandle md, RenderPass render_pass) {
	headers_[md.index()].render_pass = render_pass;
}

inline uint32_t MaterialManger::OpenHandle(MaterialHandle md) {
	return buffer_[md.index()];
}

inline uint32_t MaterialManger::GetMaterialSize(MaterialHandle md) {
	switch (GetDecriptor(md).type)
	{
	case MaterialType::kFloat: // fall through
	case MaterialType::kSampler:
		return sizeof uint32_t;

	case MaterialType::kVec3:
		return sizeof glm::vec3;
	case MaterialType::kMat4:
		return sizeof glm::mat4;
	default:
		return 0;
	}
}


/////////////////////////////////////////////////////
///////           Materials  APIS             ///////
/////////////////////////////////////////////////////

MaterialManger gMaterialManger(1024);

MaterialHandle NewTexture(const unsigned char* data, int width, int height, TextureFormat format)
{
	return gMaterialManger.NewTexture(data, width, height, format);
}

MaterialHandle NewUniform(ShaderHandle target, const char* name, MaterialType type)
{
	return gMaterialManger.NewUniform(target, name, type);
}

uint32_t OpenHandle(MaterialHandle md)
{
	return gMaterialManger.OpenHandle(md);
}

MaterialHeader GetMaterialDecriptor(MaterialHandle md)
{
	return gMaterialManger.GetDecriptor(md);
}

uint32_t GetMaterialSize(MaterialHandle md)
{
	return gMaterialManger.GetMaterialSize(md);
}


#pragma warning( push )
#pragma warning (disable: 26495)
template <typename Buffer, typename Handle, int kMax>
struct BufferManger {
	Buffer buffer[kMax];
	//typename Handle::index_t handles[kMax];
	uint16_t  handles[kMax];
	int current;
	BufferManger() : current(0) {
		for (int i = 0; i < kMax; ++i) {
			handles[i] = i + 1;
		}
	}

	Handle NewHandle() {
		if (current == kMax) {
			return Handle::MakeNil();
		}

		int cur = current;
		current = handles[cur];

		return Handle(cur);
	}

	Buffer& Open(Handle h) {
		return buffer[h.index()];
	}

};
#pragma warning (pop)
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
///////                Shaders                ///////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////


bool EvaluateMacros(std::string& line, size_t p, std::string& tok, const Set<std::string>& macros) {
	bool eval = true;
	while (p < line.size()) {
		bool should_exist = true;
		tok.clear();

		while (isspace(line[p]))
			++p;
		if (line[p] == '!') {
			should_exist = false;
			++p;
		}
		while (isalnum(line[p]) || line[p] == '_') {
			tok += line[p];
			++p;
		}

		bool exist = macros.count(tok);
		eval = !(exist ^ should_exist); // both stands will give true

		while (isspace(line[p]))
			++p;

		if (p > line.size() - 2) break;

		if (line[p] == '|' && line[p + 1] == '|') {
			// true || ?
			if (eval) break;
		}

		if (line[p] == '&' && line[p + 1] == '&') {
			// false && ?
			if (!eval) break;
		}
		p += 2;
	}
	return eval;
}


std::string PreprocessShader(std::istream& is,
	                         const Set<std::string>& macros,
	                         uint32_t version) {
	std::string line;
	std::string tok;
	std::ostringstream oss;
	oss << "#version " << std::to_string(version) << " core\n";
	
	size_t p;
	int cnt = 0;
	while (is.good()) {
		std::getline(is, line);
		bool eval = true;
		int id = -1;
		for (p = 0; p < line.size(); ++p) {
			if (p > line.size() - 3) break;

			if (line[p] == '$' && line[p + 1] == 'I' && line[p + 2] == 'D') {
				id = p;
			}

			if (line[p] == '/' && line[p + 1] == '/') {
				tok.clear();
				p += 2;
				while (p < line.size() && !isspace(line[p])) {
					tok += line[p];
					++p;
				}
				while (isspace(line[p])) {
					++p;
				}

				/// ::= //@ bla
				if (tok == "@") {
					eval = EvaluateMacros(line, p, tok, macros);
				}

				/// ::= //#ID 10
				else if (tok == "#ID") {
					tok.clear();
					while (!isspace(line[p])) {
						tok += line[p];
						++p;
					}

					id = atoi(tok.c_str());
				}
				break;
			}
			if (line[p] == '/' && line[p + 1] == '/' && line[p + 2] == '@') {
				p += 3;
				eval = EvaluateMacros(line, p, tok, macros);

				break;
			}
		}

		if (eval) {
			if (id >= 0) {
				std::string idstr = std::to_string(cnt);
				line[id    ] = idstr.size() > 0 ? idstr[0] : ' ';
				line[id + 1ull] = idstr.size() > 1 ? idstr[1] : ' ';
				line[id + 2ull] = idstr.size() > 2 ? idstr[2] : ' ';
				++cnt;
			}
			oss << line << '\n';
		}
	}
	
	
	return oss.str();
}


static BufferManger<Shader, ShaderHandle, 4096> gShaderBuffer;

ShaderHandle NewShader(ShaderFilePath shaderpath)
{
	ShaderHandle h = gShaderBuffer.NewHandle();
	if (h.IsNil()) {
		return h;
	}

	gShaderBuffer.buffer[h.index()].CompileShader(shaderpath);
	return h;
}

Shader OpenHandle(ShaderHandle sh)
{
	return gShaderBuffer.Open(sh);
}

ShaderHandle NewShader(ShaderFilePath path,
                       Set<std::string>& macros,
                       uint32_t version /* = 330 */) {
	ShaderSourceCode ssc(nullptr, nullptr, nullptr);
	std::string vertex;
	std::string fragment;
	std::string geometry;
	
	{
		std::ifstream ifs(path.vertex);
		vertex = PreprocessShader(ifs, macros, version);
		ssc.vertex = vertex.c_str();
	}

	{
		std::ifstream ifs(path.fragment);
		fragment = PreprocessShader(ifs, macros, version);
		ssc.fragment = fragment.c_str();
	}

	if(path.geometry != nullptr)
	{
		std::ifstream ifs(path.geometry);
		geometry = PreprocessShader(ifs, macros, version);
		ssc.geometry = geometry.c_str();
	}

	ShaderHandle h = gShaderBuffer.NewHandle();
	if (h.IsNil()) {
		return h;
	}

	gShaderBuffer.buffer[h.index()].CompileShader(ssc);
	return h;
}


ShaderHandle NewShader(ShaderInfo info)
{
	//TODO(L)
	return ShaderHandle();
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
///////                Meshes                 ///////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

static BufferManger<MeshBuffer, MeshHandle, 4096> gMeshBuffer;

MeshHandle NewMesh(MemoryInfo data, MemoryInfo indices, const Vector<LayoutInfo>& layouts) {
	unsigned int VBO, VAO, EBO;
	//TODO(L) Check gl errors
	//TODO(L) delelte vbo if already exits
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, data.size_by_bytes, data.ptr, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size_by_bytes, indices.ptr, GL_STATIC_DRAW);

	for (const auto& ly : layouts) {
		glVertexAttribPointer(ly.position, ly.count_per_vertex, 
			ly.type, ly.normalized ? GL_TRUE : GL_FALSE, 
			ly.stride_by_bytes, (void*)static_cast<size_t>(ly.first_element_offset_by_bytes));
		glEnableVertexAttribArray(ly.position);
	}

	glBindVertexArray(0);
	auto handle = gMeshBuffer.NewHandle();
	gMeshBuffer.buffer[handle.index()].ebo = EBO;
	gMeshBuffer.buffer[handle.index()].vao = VAO;
	gMeshBuffer.buffer[handle.index()].vbo = VBO;
	gMeshBuffer.buffer[handle.index()].num_indices = indices.size_by_bytes / sizeof(unsigned int);
	return handle;
}

MeshBuffer OpenHandle(MeshHandle md) {
	return gMeshBuffer.buffer[md.index()];
}
}
}