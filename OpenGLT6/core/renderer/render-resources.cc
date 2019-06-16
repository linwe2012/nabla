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
		TextureFormat format, TextureFormat source_format, TextureFormat source_type = TextureFormat::kUchar);

	MaterialHandle NewUniform(ShaderHandle target, const char* name, MaterialType type);

	void DescribeHanlde(MaterialHandle md, RenderPass render_pass);

	uint32_t OpenHandle(MaterialHandle md);

	uint32_t GetMaterialSize(MaterialHandle md);

	MaterialHandle NewTextureCubic(const Vector<unsigned char*>& data, int width, int height, 
		TextureFormat format, TextureFormat source_format, TextureFormat source_type = TextureFormat::kUchar);

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

int GetGLFormat(TextureFormat f) {
	switch (f)
	{
	case nabla::renderer::kRed: return GL_RED;
	case nabla::renderer::kRGB: return GL_RGB;
	case nabla::renderer::kRGBA: return GL_RGBA;
	case nabla::renderer::kRG16F: return GL_RG16F;
	case nabla::renderer::kRG: return GL_RG;
	case nabla::renderer::kUchar: return GL_UNSIGNED_BYTE;
	case nabla::renderer::kRGB16F: return GL_RGB16F;
	case nabla::renderer::kFloat: return GL_FLOAT;
	case nabla::renderer::kInt32: return GL_INT;
	default:return 0;
	}
}

MaterialHandle MaterialManger::NewTexture(const unsigned char* data,
	                                      int width, int height,
	                                     TextureFormat format, 
										 TextureFormat source_format,
										 TextureFormat source_type) {
	uint32_t id;
	glGenTextures(1, &id);
	if (id == 0) {
		NA_LOG_ERROR("Unable to generate texture buffer");
		return MaterialHandle::MakeNil();
	}
	

	glBindTexture(GL_TEXTURE_2D, id);
	
	if(source_format == TextureFormat::kInavlid) {
		source_format = format;
	}


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // GL_LINEAR_MIPMAP_LINEAR
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GetGLFormat(format), width, height, 0, GetGLFormat(source_format), GetGLFormat(source_type), data);
	glGenerateMipmap(GL_TEXTURE_2D);

	NA_ASSERT(glGetError() == 0, "Invalid opengl cmd");
	MaterialHeader header;
	header.type = MaterialType::kSampler2D;
	header.width = width;
	header.height = height;
	MaterialHandle md(headers_.size());
	buffer_.push_back(id);
	headers_.push_back(header);
	return md;
}

//TODO: Support for other texture
MaterialHandle MaterialManger::NewTextureCubic(const Vector<unsigned char*>& data, int width, int height, 
	                                           TextureFormat format, 
	                                           TextureFormat source_format, 
	                                           TextureFormat source_type) {
	uint32_t id;
	glGenTextures(1, &id);
	if (id == 0) {
		NA_LOG_ERROR("Unable to generate texture buffer");
		return MaterialHandle::MakeNil();
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, id);

	int texture_format = GetGLFormat(format);

	if (source_format == TextureFormat::kInavlid) {
		source_format = format;
	}

	int src_format = GetGLFormat(source_format);
	int src_type = GetGLFormat(source_type);

	int cnt = 0;

	if (data.size() == 0) {
		for (; cnt < 6; ++cnt) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + cnt,
				0, texture_format, width, height, 0, src_format, src_type, nullptr
			);
		}
	}
	else {
		for (auto datum : data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + cnt,
				0, texture_format, width, height, 0, src_format, src_type, datum
			);
			++cnt;
		}
	}
	

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	NA_ASSERT(glGetError() == 0, "Invalid opengl cmd");
	MaterialHeader header;
	header.type = MaterialType::kSamplerCubic;
	header.width = width;
	header.height = height;
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

MaterialHandle NewTexture(const unsigned char* data, int width, int height, TextureFormat format, TextureFormat source_format, TextureFormat source_type)
{
	return gMaterialManger.NewTexture(data, width, height, format, source_format, source_type);
}

MaterialHandle NewTextureCubic(const Vector<unsigned char*>& data, int width, int height, TextureFormat format, TextureFormat source_format, TextureFormat source_type)
{
	return gMaterialManger.NewTextureCubic(data, width, height, format, source_format, source_type);
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

	gShaderBuffer.buffer[h.index()].CompileShader(ssc, path);
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
	gMeshBuffer.buffer[handle.index()].num_vertices = data.size_by_bytes / sizeof(glm::vec3) / layouts.size();
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

FrameBufferHandle NewGBuffer(int width, int height, ShaderHandle attached_shader, ShaderHandle blit_shader, const Vector<Attachment>& textures)
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
	g.blit_shader = blit_shader;

	gFrameBuffers.buffer[hframe.index()] = g;

	return hframe;
}

const FrameBuffer& OpenHandle(FrameBufferHandle hf)
{
	return gFrameBuffers.Open(hf);
}

struct CaptureFrameBuffer {
	glm::mat4 captureProjection;
	glm::mat4 captureViews[6];
	unsigned int captureFBO;
	unsigned int captureRBO;
	bool prepared = false;
};

CaptureFrameBuffer gDefaultCaptureFrameBuffer;

void PrepareCapture()
{
	auto& f = gDefaultCaptureFrameBuffer;
	glGenFramebuffers(1, &f.captureFBO);
	glGenRenderbuffers(1, &f.captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, f.captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, f.captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, f.captureRBO);

	f.captureViews[0] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	f.captureViews[1] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	f.captureViews[2] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	f.captureViews[3] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	f.captureViews[4] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	f.captureViews[5] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
}
void RenderCube();
void RenderQuad();

void ComputeIrradianceMap(MaterialHandle irradiance,
	                      MaterialHandle skybox, 
	                      ShaderHandle compute_shader) {
	auto irr_id = OpenHandle(irradiance);
	auto irr_header = GetMaterialDecriptor(irradiance);
	auto id = OpenHandle(skybox);
	auto& f = gDefaultCaptureFrameBuffer;
	glBindFramebuffer(GL_FRAMEBUFFER, f.captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, f.captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, irr_header.width, irr_header.height);
	auto shader = OpenHandle(compute_shader);
	shader.Use();
	shader.SetInt("skybox", 0);
	shader.SetMat4("projection", f.captureProjection);

	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
	glViewport(0, 0, irr_header.width, irr_header.height);
	glBindFramebuffer(GL_FRAMEBUFFER, f.captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		shader.SetMat4("view", f.captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irr_id, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		RenderCube();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// pbr: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
// ----------------------------------------------------------------------------------------------------
void ComputePrefilterMap(MaterialHandle prefilter, 
	                     MaterialHandle skybox,
	                     ShaderHandle compute_shader) {
	auto& f = gDefaultCaptureFrameBuffer;
	auto pre_filter_shader = OpenHandle(compute_shader);
	auto pre_id = OpenHandle(prefilter);
	pre_filter_shader.Use();
	pre_filter_shader.SetInt("skybox", 0);
	pre_filter_shader.SetMat4("projection", f.captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, OpenHandle(skybox));

	glBindFramebuffer(GL_FRAMEBUFFER, f.captureFBO);
	unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		// reisze framebuffer according to mip-level size.
		unsigned int mipWidth = 128 * std::pow(0.5, mip);
		unsigned int mipHeight = 128 * std::pow(0.5, mip);
		glBindRenderbuffer(GL_RENDERBUFFER, f.captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		pre_filter_shader.SetFloat("roughness", roughness);
		for (unsigned int i = 0; i < 6; ++i)
		{
			pre_filter_shader.SetMat4("view", f.captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, pre_id, mip);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			RenderCube();
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ComputeBrdfLUTTexture(MaterialHandle brdfLUT, ShaderHandle compute_shader) {

	auto& f = gDefaultCaptureFrameBuffer;
	auto header = GetMaterialDecriptor(brdfLUT);
	glBindFramebuffer(GL_FRAMEBUFFER, f.captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, f.captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, header.width, header.height);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, OpenHandle(brdfLUT), 0);

	glViewport(0, 0, header.width, header.height);
	OpenHandle(compute_shader).Use();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	RenderQuad();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

IBLMapComputResult ComputeIBLMaps(MaterialHandle skybox) {
	auto& f = gDefaultCaptureFrameBuffer;
	IBLMapComputResult res;

	if (f.prepared == false) {
		PrepareCapture();
	}
	auto shader_irr = NewShader(ShaderFilePath{
		"nabla/shaders/cubemap.vs",
		"nabla/shaders/cubemap-irradiance-conv.fs"
		});
	res.irradiance = NewTextureCubic(Vector<unsigned char*>(), 32, 32, TextureFormat::kRGB16F, TextureFormat::kRGB, TextureFormat::kFloat);
	ComputeIrradianceMap(res.irradiance, skybox, shader_irr);
	NA_ASSERT(glGetError() == 0);
	auto shader_prefilter = NewShader(ShaderFilePath{
		"nabla/shaders/cubemap.vs",
		"nabla/shaders/cubemap-prefilter.fs"
		});
	res.prefilter = NewTextureCubic(Vector<unsigned char*>(), 128, 128, TextureFormat::kRGB16F, TextureFormat::kRGB, TextureFormat::kFloat);
	ComputePrefilterMap(res.prefilter, skybox, shader_prefilter);
	NA_ASSERT(glGetError() == 0);
	auto shader_brdf = NewShader(ShaderFilePath{
		"nabla/shaders/brdf.vs",
		"nabla/shaders/brdf.fs"
		});
	res.brdfLUT = NewTexture(nullptr, 512, 512, TextureFormat::kRG16F, TextureFormat::kRG, TextureFormat::kFloat);
	ComputeBrdfLUTTexture(res.brdfLUT, shader_brdf);
	RestoreViewport();
	NA_ASSERT(glGetError() == 0);
	return res;
}


// RenderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
static unsigned int cubeVAO = 0;
static unsigned int cubeVBO = 0;
void RenderCube()
{
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

static unsigned int quadVAO = 0;
static unsigned int quadVBO;
void RenderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

struct GlobalData {
	glm::mat4 projection;
	glm::mat4 view;
};
GlobalData gGlobalRenderData;

void SetGlobalProjectionMatrix(glm::mat4 p) {
	gGlobalRenderData.projection = p;
}
void SetGlobalViewMatrix(glm::mat4 p) {
	gGlobalRenderData.view = p;
}

glm::mat4 GetGlobalViewMatrix() {
	return gGlobalRenderData.view;
}

glm::mat4 GetGlobalProjectionMatrix() {
	return gGlobalRenderData.projection;
}


}
}