#include "drawcall.h"

namespace nabla {
namespace renderer {

Shader ActivateShader(ShaderHandle hshader) {
	static ShaderHandle current;
	static Shader current_shader;
	if (current == hshader || hshader.IsNil()) return current_shader;
	current_shader = OpenHandle(hshader);
	current_shader.Use();
	current = hshader;
	return current_shader;
}



NA_DRAWCALL_IMPL(IndexedDrawCall) {
	if (hmesh.IsNil()) return;
	auto mat = OpenHandle(hmesh);
	auto shader = ActivateShader(hshader);
	
	shader.SetModel(model);
	glBindVertexArray(mat.vao);
	glDrawElements(GL_TRIANGLES, mat.vbo, GL_UNSIGNED_INT, 0);
}




NA_DRAWCALL_IMPL(MaterialDrawCall) {
	auto id = OpenHandle(md);
	auto desc = GetMaterialDecriptor(md);
	auto shader = ActivateShader(desc.hshader);
	void* payload = reinterpret_cast<char*>(this) + offset_by_bytes;
	int _degug_int;
	switch (desc.type)
	{
	case MaterialType::kFloat:
		glUniform1f(id, *(static_cast<float*>(payload)));
		break;
	case MaterialType::kInt:
		glUniform1i(id, *(static_cast<int*>(payload)));
		break;
	case MaterialType::kVec3:
		glUniform3fv(id, 1, (static_cast<float*>(payload)));
		break;
	case MaterialType::kMat4:
		glUniformMatrix4fv(id, 1, GL_FALSE, (static_cast<float*>(payload)));
		break;
	default:
		//TODO: log error
		break;
	}
	assert(glGetError() == 0);
	// uuu = glGetError();
	
}
}
}