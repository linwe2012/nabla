#include "shader.h"

#include "containers/map.h"

namespace nabla {
namespace renderer {

struct ShaderInfoHasher {
	size_t operator()(const ShaderInfo& r) { return r.bit1; }
};

struct ShaderData {
	size_t uniform_size; /* size of the uniform buffer */
	Shader shader;
};

static HashMap<ShaderInfo, Shader, ShaderInfoHasher> gShaderInfos;


/** constructor generates the shader on the fly
@return true if shader successfully compiled
*/
bool Shader::CompileShader(ShaderSourceCode ssc, ShaderFilePath _sfp, Set<std::string> _macros)
{
	macros = _macros;
	sfp_ = _sfp;
	// shader Program
	ID = glCreateProgram();
	try
	{
		int vertex = CompileShaderPass(ssc.vertex, GL_VERTEX_SHADER);
		int fragment = CompileShaderPass(ssc.fragment, GL_FRAGMENT_SHADER);
		int geometry = 0; (void)geometry;
		if (ssc.geometry != nullptr) {
			geometry = CompileShaderPass(ssc.geometry, GL_GEOMETRY_SHADER);
		}
		glLinkProgram(ID);
		CheckCompileErrors(ID, "PROGRAM");

		glDeleteShader(vertex);
		glDeleteShader(fragment);
		if (ssc.geometry != nullptr)
			glDeleteShader(geometry);

		uniform_model = glGetUniformLocation(ID, "model");
		return true;
	}
	catch (std::ifstream::failure e)
	{
		NA_LOG_ERROR("Unable to open shader files: %s", e.what());
		return false;
	}

}

bool Shader::CompileShader(ShaderFilePath sfp)
{
	sfp_ = sfp;
	// shader Program
	ID = glCreateProgram();

	try
	{

		int vertex = CompileShaderFilePass(sfp.vertex, GL_VERTEX_SHADER);
		int fragment = CompileShaderFilePass(sfp.fragment, GL_FRAGMENT_SHADER);
		int geometry = CompileShaderFilePass(sfp.geometry, GL_GEOMETRY_SHADER);

		glLinkProgram(ID);
		if (!CheckCompileErrors(ID, "PROGRAM")) {
			return false;
		}

		glDeleteShader(vertex);
		glDeleteShader(fragment);
		if (sfp.geometry != nullptr)
			glDeleteShader(geometry);

		uniform_model = glGetUniformLocation(ID, "model");
		return true;
	}
	catch (std::ifstream::failure e)
	{
		NA_LOG_ERROR("Unable to open shader files: %s", e.what());
		return false;
	}
}


/*
* @exception: throws std::ifstream::failure
* will not report the problem
*/
GLuint Shader::CompileShaderFilePass(const char* path, GLenum type) {
	if (path == nullptr) return static_cast<GLuint>(-1);
	std::ifstream is(path);
	std::stringstream ss;
	ss << is.rdbuf();

	std::string code = ss.str();

	// compile shader and error checks
	return CompileShaderPass(code.c_str(), type);
}


GLuint Shader::CompileShaderPass(const char* source_code_txt, GLenum type)
{
	GLuint shader;

	shader = glCreateShader(type);
	glShaderSource(shader, 1, &source_code_txt, NULL);
	glCompileShader(shader);
	CheckCompileErrors(shader, ShaderType2String(type));

	glAttachShader(ID, shader);
	return shader;
}


/** @brief utility function for checking shader compilation/linking errors. */
inline int Shader::CheckCompileErrors(GLuint shader, std::string type)
{
	GLint success;
	GLchar infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			NA_LOG_ERROR("SHADER_COMPILATION_ERROR of type: %s\n%s\n", type.c_str(), infoLog);
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			NA_LOG_ERROR("PROGRAM_LINKING_ERROR of type: %s\n%s\n", type.c_str(), infoLog);
		}
	}
	return success;
}


inline const char* Shader::ShaderType2String(GLenum type) {
	static const char* vertex = "VERTEX";
	static const char* fragment = "FRAGMENT";
	static const char* geometry = "GEOMETRY";
	static const char* unknown = "UNKNOW";
	switch (type)
	{
	case GL_VERTEX_SHADER:   return vertex;
	case GL_FRAGMENT_SHADER: return fragment;
	case GL_GEOMETRY_SHADER: return geometry;
	default: return unknown;
	}
}

}
}

