#ifndef _ENGINE_RENDER_SHADER_H_
#define _ENGINE_RENDER_SHADER_H_

#include <utils/logger.h>

#include <glad/glad.h>
#include "glm.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "containers/vector.h"
#include "containers/set.h"

namespace nabla {
namespace renderer {

#define NA_BUILTIN_SHADER_TOGGOLE_LIST(V, X) \
	V(DiffuseMap, uint32_t)	    \
	V(SpecularMap, uint32_t)    \
	V(EmissiveMap, uint32_t)    \
	V(AmbientMap, uint32_t)	    \
	V(HeightMap, uint32_t)	    \
	V(NormalsMap, uint32_t)	    \
	V(ShininessMap, uint32_t)   \
	V(OpacityMap, uint32_t)	    \
	X(Albedo, glm::vec3)	    \
	X(Metllic, float)		    \
	X(Roughness, float)	        \
	X(AmbientOcclusion, float)  

/** current is a 64 bit bitset, could be expanded to 128 or higher*/
struct ShaderInfo {
	enum DefaultFlags : int {
		kNone,
#define ENUM_FLAGS(x, type) k##x,
		NA_BUILTIN_SHADER_TOGGOLE_LIST(ENUM_FLAGS, ENUM_FLAGS)
#undef ENUM_FLAGS
		kNumBuiltinFlags
	};

/** please make sure types are aligned by 4 bytes, i.e. there should be no member
has field whose size is larger than 4 bytes
*/
#define ENSURE_ALIGNMENT(x, type) static_assert(alignof(type) == alignof(uint32_t), \
	"default types should have alignment of 4 bytes");
	NA_BUILTIN_SHADER_TOGGOLE_LIST(ENSURE_ALIGNMENT, ENSURE_ALIGNMENT)
#undef ENSURE_ALIGNMENT

	uint64_t bit1 = 0;
	constexpr bool bit(int position) {
		uint64_t b = bit1 >> position;
		return b & 1ULL;
	}

	void set_bit(int position) {
		bit1 |= (1ULL << position);
	}

	void clear_bit(int position) {
		bit1 &= ~(1 << position);
	}

	/** This is intended for std::map */
	bool operator<(const ShaderInfo& rhs) const { return bit1 < rhs.bit1; }
	bool operator==(const ShaderInfo& rhs) const { return bit1 == rhs.bit1; }
	bool operator!=(const ShaderInfo& rhs) const { return !operator==(rhs); }

	constexpr size_t GetNumBits() { return sizeof(ShaderInfo); }

	size_t GetUniformSizeOfBuiltins() {
		size_t res = 0;
#define ENUM_FLAGS_SIZE(x, type)                              \
			if(!std::is_void_v<uint32_t> && bit(k##x)) {      \
				res += sizeof(type);                          \
			}                                                 
		
		NA_BUILTIN_SHADER_TOGGOLE_LIST(ENUM_FLAGS_SIZE, ENUM_FLAGS_SIZE);
#undef ENUM_FLAGS_SIZE
	}
};

struct ShaderFilePath {
	ShaderFilePath()
		: vertex(nullptr), fragment(nullptr) {}
	ShaderFilePath(const char* vertexPath, const char* fragmentPath)
		:vertex(vertexPath), fragment(fragmentPath) {}

	ShaderFilePath(const char* vertexPath, const char* fragmentPath, const char* geometryPath)
		:vertex(vertexPath), fragment(fragmentPath), geometry(geometryPath) {}

	const char* vertex;
	const char* fragment;
	const char* geometry = nullptr;

};

struct ShaderSourceCode {
	ShaderSourceCode(const char* _vertex, const char* _fragment)
		:vertex(_vertex), fragment(_fragment) {}

	ShaderSourceCode(const char* _vertex, const char* _fragment, const char* _geometry)
		:vertex(_vertex), fragment(_fragment), geometry(_geometry) {}

	const char* vertex;
	const char* fragment;
	const char* geometry = nullptr;
};

class Shader
{
public:
	unsigned int ID = 0;
	unsigned int uniform_model = 0;
	Shader() :ID(0), uniform_model(0) {}

	// 
	Shader(ShaderFilePath sfp)
	{
		CompileShader(sfp);
	}

	/**
	@param type see also CompileShaderFilePass()
	*/
	const char* ShaderType2String(GLenum type);


	bool CompileShader(ShaderSourceCode ssc, ShaderFilePath sfp, Set<std::string> _macros);

	bool CompileShader(ShaderFilePath sfp);

	/**
	* Function CompileShaderFilePass
	* @brief compile specific shader
	* @param type should be `GL_VERTEX_SHADER` or `GL_FRAGMENT_SHADER` or `GL_GEOMETRY_SHADER`
	* @exception: throws std::ifstream::failure
	* will not report the problem
	*/
	GLuint CompileShaderFilePass(const char* path, GLenum type);

	GLuint CompileShaderPass(const char* source_code_txt, GLenum type);

	/** activate the shader */
	void Use()
	{
		glUseProgram(ID);
	}

	// utility uniform functions
	// ------------------------------------------------------------------------
	void SetBool(const std::string& name, bool value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}
	// ------------------------------------------------------------------------
	void SetInt(const std::string& name, int value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}
	// ------------------------------------------------------------------------
	void SetFloat(const std::string& name, float value) const
	{
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}
	// ------------------------------------------------------------------------
	void SetVec2(const std::string& name, const glm::vec2& value) const
	{
		glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void SetVec2(const std::string& name, float x, float y) const
	{
		glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
	}
	// ------------------------------------------------------------------------
	void SetVec3(const std::string& name, const glm::vec3& value) const
	{
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void SetVec3(const std::string& name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
	}
	// ------------------------------------------------------------------------
	void SetVec4(const std::string& name, const glm::vec4& value) const
	{
		glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void SetVec4(const std::string& name, float x, float y, float z, float w)
	{
		glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
	}
	// ------------------------------------------------------------------------
	void SetMat2(const std::string& name, const glm::mat2& mat) const
	{
		glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void SetMat3(const std::string& name, const glm::mat3& mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void SetMat4(const std::string& name, const glm::mat4& mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

	void SetModel(const glm::mat4& mat) const {
		if (uniform_model != 0) {
			glUniformMatrix4fv(uniform_model, 1, GL_FALSE, &mat[0][0]);
		}
	}

	Set<std::string> macros;
private:
	// utility function for checking shader compilation/linking errors.
	// ------------------------------------------------------------------------
	int CheckCompileErrors(GLuint shader, std::string type);
	ShaderFilePath sfp_;
};


/** constructor generates the shader on the fly
@return true if shader successfully compiled
*/



/* try to find shader with specified info, if not, will compile from source 
@param[in] fallback if unable to find compiled shader w/ info, will compile form fallback
*/
Shader GetShaderByInfo(ShaderInfo info,
	                   ShaderFilePath fallback, 
	                   const Vector<const char *>* macros = nullptr);
}
}
#endif // !_ENGINE_RENDER_SHADER_H_