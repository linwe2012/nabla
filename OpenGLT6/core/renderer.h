#ifndef _NABLA_CORE_RENDERER_H_
#define _NABLA_CORE_RENDERER_H_

#include "renderer/render-resources.h"
#include "renderer/shader.h"
#include "renderer/compiled-drawcall.h"

namespace nabla {
namespace renderer {

	struct InitConfig {
		const char* name = "Nabla";
		int width = 800;
		int height = 600;
		int fps_hint = 60;
	};

	struct GBuffer {
		void Create(int width, int height);
		uint32_t rbo_depth;
		uint32_t fbo;
		uint32_t position;
		uint32_t normal;
		uint32_t albedo_spec;

	};

	void Init(const InitConfig&);
	void NextFrame();
	bool IsAlive();

	void* GetWindow();
}
}

#endif // !_NABLA_CORE_RENDERER_H_

