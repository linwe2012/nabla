#ifndef _NABLA_CORE_RENDERER_H_
#define _NABLA_CORE_RENDERER_H_

#include "renderer/render-resources.h"
#include "renderer/shader.h"

namespace nabla {
namespace renderer {

	struct InitConfig {
		const char* name = "Nabla";
		int width = 800;
		int height = 600;
		int fps_hint = 60;
	};

	

	class Window {
		enum 
		{
			kMaxBuffer = 4096
		};
	public:
		static Window* Create(const InitConfig&);
		bool IsAlive() { return is_alive_; }
		void NextFrame();

		// VertexBufferHandle NewVertexBuffer(void *data, uint32_t size);
		// 
		// ShaderHandle NewShader(const char* vertexPath,
		//	                   const char* fragmentPath,
		//	                   const char* geometryPath = nullptr);

	private:

		void* glfw_window_ = nullptr;
		bool is_alive_ = true;
	};

	

	void Init(const InitConfig&);
	void NextFrame();
	bool IsAlive();
	VertexBufferHandle NewVertexBuffer(void* data, uint32_t size);
	ShaderHandle NewShader(const char* vertexPath,
		                   const char* fragmentPath,
		                   const char* geometryPath = nullptr);

	ShaderHandle NewShader(ShaderInfo info);
	

	void DrawIndexed(VertexBufferHandle vertex, IndexBufferHandle index, ShaderHandle shader);
}
}

#endif // !_NABLA_CORE_RENDERER_H_

