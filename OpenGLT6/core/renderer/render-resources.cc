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

	MaterialHandle NewTextureCubic(const Vector<unsigned char*>& data, int width, int height, TextureFormat format);

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
	

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	NA_ASSERT(glGetError() == 0, "Invalid opengl cmd");
	MaterialHeader header;
	header.type = MaterialType::kSampler2D;
	MaterialHandle md(headers_.size());
	buffer_.push_back(id);
	headers_.push_back(header);
	return md;
}

//TODO: Support for other texture
MaterialHandle MaterialManger::NewTextureCubic(const Vector<unsigned char*>& data, int width, int height, TextureFormat format) {
	uint32_t id;
	glGenTextures(1, &id);
	if (id == 0) {
		NA_LOG_ERROR("Unable to generate texture buffer");
		return MaterialHandle::MakeNil();
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, id);

	int cnt = 0;
	for (auto datum : data) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + cnt,
			0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, datum
		);
		++cnt;
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	NA_ASSERT(glGetError() == 0, "Invalid opengl cmd");
	MaterialHeader header;
	header.type = MaterialType::kSamplerCubic;
	MaterialHandle md(headers_.size());
	buffer_.push_back(id);
	headers_.push_back(header);
	return md;
}

inline MaterialHandle MaterialManger::NewUniform(ShaderHandle target, const char* name, MaterialType type) {
	Shader shader = ::nabla::renderer::OpenHandle(target);
	int id = glGetUniformLocation(shader.ID, name);
	if (id < 0) {
		NA_ASSERT(false, "Invalid handle");
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
	case MaterialType::kSamplerCubic:
	case MaterialType::kSampler2D:
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

MaterialHandle NewTextureCubic(const Vector<unsigned char*>& data, int width, int height, TextureFormat format)
{
	return gMaterialManger.NewTextureCubic(data, width, height, format);
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
		// std::cout << vertex;
		ssc.vertex = vertex.c_str();
	}

	{
		std::ifstream ifs(path.fragment);
		fragment = PreprocessShader(ifs, macros, version);
		std::cout << fragment;
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
	unsigned int VBO = 0, VAO = 0, EBO = 0;
	//TODO(L) Check gl errors
	//TODO(L) delelte vbo if already exits
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	if (indices.ptr != nullptr) {
		glGenBuffers(1, &EBO);
	}

	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, data.size_by_bytes, data.ptr, GL_STATIC_DRAW);
	
	if (indices.ptr != nullptr) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size_by_bytes, indices.ptr, GL_STATIC_DRAW);
	}

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
	gMeshBuffer.buffer[handle.index()].num_vertices = data.size_by_bytes / sizeof(glm::vec3);
	if (indices.ptr != nullptr) {
		gMeshBuffer.buffer[handle.index()].num_indices = indices.size_by_bytes / sizeof(unsigned int);
	}
	gMeshBuffer.buffer[handle.index()].num_indices = data.size_by_bytes / sizeof(float);
	return handle;
}

MeshBuffer OpenHandle(MeshHandle md) {
	return gMeshBuffer.buffer[md.index()];
}



/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
///////                Gbuffer                ///////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
static BufferManger<FrameBuffer, FrameBufferHandle, 16> gFrameBuffers;

FrameBufferHandle NewGBuffer(int width, int height, ShaderHandle attached_shader, const Vector<Attachment>& textures)
{
	FrameBufferHandle hframe = gFrameBuffers.NewHandle();
	FrameBuffer g;
	glGenFramebuffers(1, &g.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, g.fbo);
	g.width = width;
	g.height = height;

	auto shader = OpenHandle(attached_shader);
	shader.Use();

	for (auto texture : textures) {
		int type = -1;
		int external_type = GL_RGB;
		if (texture.format == TextureFormat::kRGB) {
			type = GL_RGB16F;
		}
		else if (texture.format == TextureFormat::kRGBA) {
			type = GL_RGBA;
		}
		else if (texture.format == TextureFormat::kFloat) {
			type = GL_R32F;
			external_type = GL_RED;
		}
		else {
			NA_ASSERT(false, "type not supported");
		}

		uint32_t color;
		glGenTextures(1, &color);
		glBindTexture(GL_TEXTURE_2D, color);
		glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, external_type, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + g.attachments.size(), GL_TEXTURE_2D, color, 0);
		NA_ASSERT(glGetError() == 0);

		g.attach_ids.push_back(GL_COLOR_ATTACHMENT0 + g.attachments.size());
		shader.SetInt(texture.name, g.attachments.size());

		NA_ASSERT(glGetError() == 0);
		
		g.attachments.push_back(color);

		g.clear_color.push_back(texture.clear_color);
	}

	glDrawBuffers(g.attachments.size(), &g.attach_ids[0]);

	glGenRenderbuffers(1, &g.rbo_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, g.rbo_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, g.rbo_depth);

	NA_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not complete!");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	NA_ASSERT(glGetError() == 0);

	g.attached_shader = attached_shader;

	gFrameBuffers.buffer[hframe.index()] = g;

	return hframe;
}

const FrameBuffer& OpenHandle(FrameBufferHandle hf)
{
	return gFrameBuffers.Open(hf);
}




}
}