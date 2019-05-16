#ifndef _NA_CORE_RENDERER_DRAWCALL_H_
#define _NA_CORE_RENDERER_DRAWCALL_H_

#include "platform.h"
#include "render-resources.h"
#include "shader.h"

// auxiliary macros to define draw calls, so that drawcall compiler will be able to
// use these drawcalls
#define NA_DRAWCALL(type)                                           \
	void (*_m__drawer__)(void *) = _M_Render__;                     \
	static void _M_Render__(void *);                                \
	static type* Cast(void * rhs) {                                 \
		if (static_cast<type*>(rhs)->_m__drawer__ == _M_Render__) { \
			return static_cast<type*>(rhs);                         \
		} else {                                                    \
			return nullptr;                                         \
		}                                                           \
	}                                                               \
	void Render();


#define NA_DRAWCALL_IMPL(type)\
	void type::_M_Render__(void *self) { static_cast<type*>(self)->Render(); }\
	FORCE_INLINE void type::Render()

namespace nabla {
namespace renderer {
enum struct State {
	Filled = 0x0, Line = 0x1,
};

struct StateDrawCall {
	NA_DRAWCALL(StateDrawCall);
};

struct IndexedDrawCall {
	NA_DRAWCALL(IndexedDrawCall);

	IndexedDrawCall() = default;
	IndexedDrawCall(MeshHandle _hmesh)
		:hshader(), hmesh(_hmesh), model(1.0f) {}

	ShaderHandle hshader;
	MeshHandle hmesh;
	glm::mat4 model;
};

struct ShaderDescript {
	NA_DRAWCALL(ShaderDescript);

	ShaderDescript(): shader() {}
	ShaderDescript(glm::mat4 _model, Shader _shader)
		: shader(_shader) {}
	
	Shader shader;
};

struct PBRDrawCall {
	NA_DRAWCALL(PBRDrawCall);

	union MapOrNum
	{
		uint32_t texture;
		float num;
	};

	union MapOrVec {
		uint32_t texture;
		glm::vec3 num;
	};

	ShaderInfo info;
	MapOrNum Metallic;
	MapOrNum Roughness;
	MapOrNum AmbientOcclusion;
	MapOrVec Albedo;
};

struct MaterialDrawCall {
	NA_DRAWCALL(MaterialDrawCall);
	MaterialDrawCall(MaterialHandle _md, uint32_t _offset_by_bytes)
		:md(_md), offset_by_bytes(_offset_by_bytes)
	{}


	MaterialHandle md;
	uint32_t offset_by_bytes;
};



}
}




#endif