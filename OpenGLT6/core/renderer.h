#ifndef _NABLA_CORE_RENDERER_H_
#define _NABLA_CORE_RENDERER_H_

#include "renderer/render-resources.h"
#include "renderer/shader.h"
#include "renderer/compiled-drawcall.h"
#include <tuple>

namespace nabla {
namespace renderer {	

	void Init(const InitConfig&);
	void NextFrame();
	bool IsAlive();

	void* GetWindow();

	std::tuple<int, int> GetWindowSize();
}
}

#endif // !_NABLA_CORE_RENDERER_H_

