#include "drawcall.h"
#include "core/config.h"

namespace nabla {
namespace renderer {
namespace detail {

static MaterialHeader last_material;
static int material_id;
}


struct GLStateMachine {
	ShaderHandle current_shader_handle;
	Shader current_shader;

#ifdef NA_DEVELOPMENT
	MaterialHeader _2d_texture[32];
	int _2d_texture_id[32];

	MaterialHeader _cubic_texture[32];
	int _cubic_texture_id[32];

#endif // NA_DEVELOPMENT
};

static GLStateMachine state;


static Shader ActivateShader(ShaderHandle hshader) {
	
	if (state.current_shader_handle == hshader || hshader.IsNil()) return state.current_shader;
	state.current_shader = OpenHandle(hshader);
	state.current_shader.Use();
	state.current_shader_handle = hshader;
	assert(glGetError() == 0);
	return state.current_shader;
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
		glDrawArrays(GL_TRIANGLES, 0, mat.num_vertices);
	}
	int uu = glGetError();
	assert(uu == 0);
}




NA_DRAWCALL_IMPL(MaterialDrawCall) {
	auto id = OpenHandle(md);
	auto desc = GetMaterialDecriptor(md);
	auto shader = ActivateShader(desc.hshader);
	void* payload = reinterpret_cast<char*>(this) + offset_by_bytes;
#ifdef NA_DEVELOPMENT
	detail::last_material = desc;
	detail::material_id = id;
#endif
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
	case MaterialType::kVec4:
		glUniform4fv(id, 1, (static_cast<float*>(payload)));
		break;
	case MaterialType::kMat4:
		glUniformMatrix4fv(id, 1, GL_FALSE, (static_cast<float*>(payload)));
		break;
	case MaterialType::kSampler2D:
		glActiveTexture(GL_TEXTURE0 + texture_id);
		glBindTexture(GL_TEXTURE_2D, id);
#ifdef NA_DEVELOPMENT
		state._2d_texture[texture_id] = desc;
		state._2d_texture_id[texture_id] = id;
#endif
		break;
	case MaterialType::kSamplerCubic:
		glActiveTexture(GL_TEXTURE0 + texture_id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, id);
#ifdef NA_DEVELOPMENT
		state._cubic_texture[texture_id] = desc;
		state._cubic_texture_id[texture_id] = id;
#endif
		break;
	default:
		//TODO: log error
		break;
	}
	int uu = glGetError();
	assert(uu == 0);
}
static unsigned int quadVAO = 0;
static unsigned int quadVBO;

FrameBufferHandle hactive_frame;

NA_DRAWCALL_IMPL(FrameBufferAttachmentReaderDrawCall) {
	const auto& f = OpenHandle(hframe);
	if (id < 0) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		callback();
		return;
	}
	glFlush();
	glFinish();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, f.fbo);
	glReadBuffer(f.attach_ids[id]);
	callback();
	int uu = glGetError();
	assert(uu == 0);
}

NA_DRAWCALL_IMPL(InlineDrawCall) {
	callback();
}

NA_DRAWCALL_IMPL(SwitchFrameBufferDrawCall) {
	const auto& f = OpenHandle(hframe);
	int cnt = 0;
	int uu;
	switch (trans)
	{
	case nabla::renderer::SwitchFrameBufferDrawCall::kRenderOnFrameBuffer:
		glBindFramebuffer(GL_FRAMEBUFFER, f.fbo);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (auto t : f.attachments) {
			glActiveTexture(GL_TEXTURE0 + cnt);
			glBindTexture(GL_TEXTURE_2D, t);
			glClearBufferfv(GL_COLOR, cnt, &f.clear_color[cnt].r);
			++cnt;
		}
		break;
	case nabla::renderer::SwitchFrameBufferDrawCall::kSampleFromFrameBuffer:
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// ActivateShader(f.attached_shader);
		// glDrawBuffers(f.attach_ids.size(), &f.attach_ids[0]);
		//for (auto t : f.attachments) {
		 	//glActiveTexture(GL_TEXTURE0 + cnt);
		 	//glBindTexture(GL_TEXTURE_2D, t);
		// 	glClearBufferfv(GL_COLOR, cnt, &f.clear_color[cnt].r);
		// 	++cnt;
		//}
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
		ActivateShader(f.attached_shader);
		for (auto t : f.attachments) {
			glActiveTexture(GL_TEXTURE0 + cnt);
			glBindTexture(GL_TEXTURE_2D, t);
		}
		
		glBindVertexArray(quadVAO);
		//uu = glGetError();
		//assert(uu == 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		uu = glGetError();
		assert(uu == 0);
		glBindVertexArray(0);
		
		ActivateShader(f.blit_shader);
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
	uu = glGetError();
	assert(uu == 0);
}

NA_DRAWCALL_IMPL(StateDrawCall) {
	
	if ((int)state & (int)State::DisableDepthTest) {
		glDisable(GL_DEPTH_TEST);
		
	}
	else {
		glEnable(GL_DEPTH_TEST);
	}

	if ((int)state & (int)State::Line) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
	else  {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0, 1.0);
	}
}


SolidPixel ReadSolidPixel(int x, int y) {
	SolidPixel pixel;
	const auto& f = OpenHandle(hactive_frame);
	glReadPixels(x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &(pixel.r));

	return pixel;
}

void ScreenShot(int width, int height, SolidPixel* data)
{
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
}




}
}