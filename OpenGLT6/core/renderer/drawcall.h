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
enum struct State : int {
	Filled = 0x0, Line = 0x1,
	DisableDepthTest = 0x2
};


struct StateDrawCall {
	NA_DRAWCALL(StateDrawCall);
	StateDrawCall(State _state) : state(_state) {}
	State state;
};

struct IndexedDrawCall {
	NA_DRAWCALL(IndexedDrawCall);

	IndexedDrawCall() = default;
	IndexedDrawCall(MeshHandle _hmesh)
		:hshader(), hmesh(_hmesh) {}
	IndexedDrawCall(ShaderHandle _hshader, MeshHandle _hmesh)
		:hshader(_hshader), hmesh(_hmesh) {}

	ShaderHandle hshader;
	MeshHandle hmesh;
};

struct UseShaderDrawCall {
	NA_DRAWCALL(UseShaderDrawCall);

	UseShaderDrawCall(ShaderHandle _hshader)
		: hshader(_hshader) {}
	
	ShaderHandle hshader;
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
	using offset_t = uint32_t;
	MaterialDrawCall(MaterialHandle _md, uint32_t _offset_by_bytes)
		:md(_md), offset_by_bytes(_offset_by_bytes), texture_id(0)
	{}
	MaterialDrawCall(MaterialHandle _md, uint32_t _offset_by_bytes, uint32_t _texture_id)
		:md(_md), offset_by_bytes(_offset_by_bytes), texture_id(_texture_id)
	{}

	MaterialHandle md;
	uint32_t offset_by_bytes;
	uint32_t texture_id;
};

struct FrameBufferAttachmentReaderDrawCall {
	NA_DRAWCALL(FrameBufferAttachmentReaderDrawCall);
	FrameBufferAttachmentReaderDrawCall(FrameBufferHandle hf, int _id, const std::function<void()>& cb)
		: hframe(hf), id(_id), callback(cb) {}

	FrameBufferHandle hframe;
	int id;
	std::function<void()> callback;
};

struct SwitchFrameBufferDrawCall {
	NA_DRAWCALL(SwitchFrameBufferDrawCall);

	enum Transition {
		kRenderOnFrameBuffer,
		kSampleFromFrameBuffer,
		kCopyFrameToDefault,
		kCopyFrameBufferToDefault
	};

	SwitchFrameBufferDrawCall(Transition t, FrameBufferHandle hf)
		:trans(t), hframe(hf) {};

	Transition trans;
	FrameBufferHandle hframe;
};

struct UpdateMeshVertexDrawCall {
	NA_DRAWCALL(UpdateMeshVertexDrawCall);

	MeshHandle target;
	int32_t data_offset; //< by bytes
	uint32_t begin;
	uint32_t end;
};

struct SolidPixel {
	unsigned char r;
	unsigned char g;
	unsigned char b;
	// unsigned char a;
};
SolidPixel ReadSolidPixel(int x, int y);

void ScreenShot(int width, int hight, SolidPixel* data);

}
}




#endif