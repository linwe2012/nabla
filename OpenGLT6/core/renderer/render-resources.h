#ifndef _NABLA_CORE_RENDER_RESOURCES_H_
#define _NABLA_CORE_RENDER_RESOURCES_H_
#include <stdint.h>
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
	bool operator!=(const Handle& rhs) const { return !operator==(); }

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

	kSampler,
	kFloat,
	kMat4,
	kVec3,
	kInt,
	kCount,
};

enum struct RenderPass : uint8_t {
	KForward,  /**< geometry & Gbuffers */
	kDeferred, /**< incl. lighting */
	kPostProc,
};

enum TextureFormat {
	kRed = GL_RED,
	kRGB = GL_RGB,
	kRGBA = GL_RGBA,
};

struct MaterialHeader {
	RenderPass render_pass;
	MaterialType type;
	ShaderHandle hshader;
};


MaterialHandle NewTexture(const unsigned char* data,
	int width, int height,
	TextureFormat format);

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


struct Light {
	enum Type : uint8_t {
		Direction,
		Point,
		Spot
	};
	glm::vec3 position;
	glm::vec3 direction; /**< ingnored by point light*/
	glm::vec3 color;
	float linear;
	float quad;
	float radius;
	float cutoff; /**< used only by spot light */
	float outer_cutoff; /**< used only by spot light */
	Type type;
	bool draw_mesh;
	MeshHandle hmesh;
};



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
