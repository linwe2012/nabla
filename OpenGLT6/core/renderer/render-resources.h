#ifndef _NABLA_CORE_RENDER_RESOURCES_H_
#define _NABLA_CORE_RENDER_RESOURCES_H_
#include <stdint.h>
#include <functional>
#include "logger.h"

#include "shader.h"

#include "containers/set.h"

namespace nabla {
namespace renderer {

template <int Index, int Version, int strong_type>
class Handle {
public:
	using index_t = uint16_t;
	Handle(uint32_t index) :version_(0), index_(index) {}
	Handle() : index_(GetNilIndex(Index)), version_(0) {}
	static Handle MakeNil() { 
		constexpr uint32_t nil = GetNilIndex(Index);
		return Handle{ nil };
	}

	bool IsNil() {
		constexpr uint32_t nil = GetNilIndex(Index);
		return index_ == nil;
	}

	uint32_t index() const { return index_; }
	uint32_t version() const { return version_; }

	bool operator<(const Handle& rhs) const { return index() < rhs.index(); }
	bool operator==(const Handle& rhs) const { return index() == rhs.index(); }
	bool operator!=(const Handle& rhs) const { return !operator==(rhs); }

private:
	constexpr static uint32_t GetNilIndex(int i) {
		if (i == 1) return 1;
		return 2 * GetNilIndex(i - 1);
	}

	uint32_t index_ : Index;
	uint32_t version_ : Version;
};




using ShaderHandle = Handle<12, 20, 0>;
using MaterialHandle = Handle<12, 20, 1>;
using MeshHandle =  Handle<12, 20, 2>;
using NameHandle = Handle<13, 1, 3>;
using FrameBufferHandle = Handle<12, 20, 4>;

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
///////                Shaders                ///////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

ShaderHandle NewShader(ShaderFilePath);

Shader OpenHandle(ShaderHandle);

ShaderHandle NewShader(ShaderInfo info);

ShaderHandle NewShader(ShaderFilePath path,
	                   Set<std::string>& macros,
	                   uint32_t version = 330);


/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
///////                Materials              ///////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
enum struct MaterialType : uint8_t {
	kUniform,

	kSampler2D,
	kSamplerCubic,
	kFloat,
	kMat4,
	kVec3,
	kVec4,
	kInt,
	kCount,
};

enum struct RenderPass : uint8_t {
	kSkybox,
	kForward,  /**< geometry & Gbuffers */
	kDeferred, /**< incl. lighting */
	kPostProc,
};

enum TextureFormat {
	kInavlid,
	kRed = GL_RED,
	kRGB = GL_RGB,
	kRGBA = GL_RGBA,
	kRG16F,
	kRG,
	kUchar,
	kRGB16F,
	kFloat,
	kInt32,
};

struct MaterialHeader {
	RenderPass render_pass = RenderPass::kForward;
	MaterialType type = MaterialType::kSampler2D;
	ShaderHandle hshader;
	uint32_t width;
	uint32_t height;
};


MaterialHandle NewTexture(const unsigned char* data,
	                      int width, int height,
	                      TextureFormat format,
	                      TextureFormat source_format = TextureFormat::kInavlid, 
	                      TextureFormat source_type = TextureFormat::kUchar);

MaterialHandle NewTextureCubic(const Vector<unsigned char*>& data,
	                        int width, int height,
	                        TextureFormat format, 
	                        TextureFormat source_format = TextureFormat::kInavlid, 
	                        TextureFormat source_type = TextureFormat::kUchar);

std::string PreprocessShader(
	std::istream& is,
	const Set<std::string>& macros,
	uint32_t version
);

MaterialHandle NewUniform(ShaderHandle target, const char* name, MaterialType type);

uint32_t OpenHandle(MaterialHandle md);

MaterialHeader GetMaterialDecriptor(MaterialHandle md);

uint32_t GetMaterialSize(MaterialHandle md);



/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
///////                Meshes                 ///////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

struct MeshBuffer {
	uint32_t vao;
	uint32_t vbo;
	uint32_t ebo;
	uint32_t num_indices;
	uint32_t num_vertices;
};

struct MemoryInfo {
	void* ptr;
	size_t size_by_bytes;
};

struct LayoutInfo {
	enum Type {
		kFloat = GL_FLOAT,
	};
	template<typename T> 
	static LayoutInfo CreatePacked(uint32_t _position, uint32_t _first_element_offset_by_bytes) {
		static_assert(false, "This should no be called, possible not implemented");
	}

	template<>
	static LayoutInfo CreatePacked<glm::vec3>(uint32_t _position, uint32_t _first_element_offset_by_bytes) {
		return LayoutInfo{
			_position,
			3,
			kFloat,
			false,
			sizeof(glm::vec3),
			_first_element_offset_by_bytes
		};
	}

	template<>
	static LayoutInfo CreatePacked<glm::vec2>(uint32_t _position, uint32_t _first_element_offset_by_bytes) {
		return LayoutInfo{
			_position,
			2,
			kFloat,
			false,
			sizeof(glm::vec2),
			_first_element_offset_by_bytes
		};
	}

	uint32_t position;
	uint32_t count_per_vertex; /**< e.g. if vec3, `type=kFloat; count_per_vertex=3`*/
	Type type;
	bool normalized;           /**< clamp to (0, 1) */
	uint32_t stride_by_bytes;  /**< offset b/w vertex attribute, by bytes */
	uint32_t first_element_offset_by_bytes;
};

MeshHandle NewMesh(MemoryInfo data, MemoryInfo indices, const Vector<LayoutInfo>& layouts);

MeshBuffer OpenHandle(MeshHandle md);

struct FrameBuffer {
	uint32_t fbo;
	uint32_t rbo_depth;
	
	
	// uint32_t position;
	// uint32_t normal;
	// uint32_t diffuse_spec;
	// 
	// uint32_t albedo;
	// uint32_t metallic_roughness_ao;
	
	int width;
	int height;
	Vector<uint32_t> attachments;
	Vector<uint32_t> attach_ids;
	Vector<const char*> attachments_name;
	Vector<glm::vec4> clear_color;
	ShaderHandle attached_shader;
	ShaderHandle blit_shader;
};

struct Attachment {
	Attachment(TextureFormat _format, const char* _name)
		: format(_format), name(_name), clear_color(0) {}
	Attachment(TextureFormat _format, const char* _name, glm::vec4 _clear_color)
		: format(_format), name(_name), clear_color(_clear_color) {}
	
	TextureFormat format;
	const char* name;
	glm::vec4 clear_color;
};


// use default buffer
FrameBufferHandle NewGBuffer(int width, int height, ShaderHandle attached_shader, ShaderHandle blit_shader, const Vector<Attachment>& textures);

const FrameBuffer& OpenHandle(FrameBufferHandle);

struct InitConfig {
	const char* name = "Nabla";
	int width = 800;
	int height = 600;
	int fps_hint = 60;
};

void PrepareCapture();


struct IBLMapComputResult {
	MaterialHandle irradiance;
	MaterialHandle prefilter;
	MaterialHandle brdfLUT;
};

IBLMapComputResult ComputeIBLMaps(MaterialHandle skybox);

void RenderCube();
void RenderQuad();

void RestoreViewport();

void SetGlobalProjectionMatrix(glm::mat4 p);
void SetGlobalViewMatrix(glm::mat4 p);

glm::mat4 GetGlobalViewMatrix();

glm::mat4 GetGlobalProjectionMatrix();

//enum Type : uint32_t {
	//	kNone,
	//	kDiffuse,
	//	kSpecular,
	//	kEmissive,
	//	kAmbient,
	//	kHeight,
	//	kNormals,
	//	kShininess,
	//	kOpacity,
	//	kPBR,
	//};

/*
struct VertexBuffer
{
	uint32_t vbo = 0;
	uint32_t size = 0;

	void Create(const void* data_, uint32_t size_) {
		size = size_;
		glGenBuffers(1, &vbo);
		if (vbo == 0) {
			NA_LOG_ERROR("Unable to generate vertex buffer");
			return;
		}

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, size, data_, GL_STATIC_DRAW);
	}

	void Update(uint32_t offset, void* data_, uint32_t size_) {
		NA_ASSERT(vbo != 0, "Invalid VBO, i.e. vertex buffer not initilized");
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, offset, size_, data_);
	}

	void Destroy() {
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glDeleteBuffers(1, &vbo);
	}
};

struct IndexBuffer {
	uint32_t ebo;
	uint32_t size;

	void Create(unsigned int* data, uint32_t size_by_bytes) {
		size = size_by_bytes;
		glGenBuffers(1, &ebo);
		if (ebo == 0) {
			NA_LOG_ERROR("Unable to generate index buffer");
			return;
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_by_bytes, data, GL_STATIC_DRAW);
	}
};
*/

}

}


#endif // !_NABLA_CORE_RENDER_RESOURCES_H_
