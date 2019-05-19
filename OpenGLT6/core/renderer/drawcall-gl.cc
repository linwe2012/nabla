#include "drawcall.h"

namespace nabla {
namespace renderer {

static Shader ActivateShader(ShaderHandle hshader) {
	static ShaderHandle current;
	static Shader current_shader;
	if (current == hshader || hshader.IsNil()) return current_shader;
	current_shader = OpenHandle(hshader);
	current_shader.Use();
	current = hshader;
	assert(glGetError() == 0);
	return current_shader;
}

NA_DRAWCALL_IMPL(UseShaderDrawCall) {
	ActivateShader(hshader);
}

NA_DRAWCALL_IMPL(IndexedDrawCall) {
	if (hmesh.IsNil()) return;
	auto mat = OpenHandle(hmesh);
	auto shader = ActivateShader(hshader);
	
	// shader.SetModel(model);
	glBindVertexArray(mat.vao);
	if (mat.ebo != 0) {
		glDrawElements(GL_TRIANGLES, mat.num_indices, GL_UNSIGNED_INT, 0);
	}
	else {
		glDrawArrays(GL_TRIANGLES, 0, mat.num_indices);
	}
	
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
	int uu = glGetError();
	assert(uu == 0);
}
unsigned int quadVAO = 0;
unsigned int quadVBO;

NA_DRAWCALL_IMPL(SwitchFrameBufferDrawCall) {
	const auto& f = OpenHandle(hframe);
	int cnt = 0;
	switch (trans)
	{
	case nabla::renderer::SwitchFrameBufferDrawCall::kRenderOnFrameBuffer:
		glBindFramebuffer(GL_FRAMEBUFFER, f.fbo);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		break;
	case nabla::renderer::SwitchFrameBufferDrawCall::kSampleFromFrameBuffer:
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ActivateShader(f.attached_shader);
		for (auto t : f.attachments) {
			glActiveTexture(GL_TEXTURE0 + cnt);
			glBindTexture(GL_TEXTURE_2D, t);
			++cnt;
		}
		break;
		
	case nabla::renderer::SwitchFrameBufferDrawCall::kCopyFrameBufferToDefault:
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

		// 2.5. copy content of geometry's depth buffer to default framebuffer's depth buffer
		// ----------------------------------------------------------------------------------
		glBindFramebuffer(GL_READ_FRAMEBUFFER, f.fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
		// blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
		// the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the 		
		// depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
		glBlitFramebuffer(0, 0, f.width, f.height, 0, 0, f.width, f.height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		break;
	default:
		break;
	}
}


}
}