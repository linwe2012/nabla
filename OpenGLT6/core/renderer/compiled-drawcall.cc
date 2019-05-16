#include "compiled-drawcall.h"
#include "containers/map.h"
#include <thread>
#include <algorithm>
#include "utils.h"

namespace nabla {

namespace renderer {

const Vector<Command>* RadixSort(Vector<Vector<Command>*> cmds_set ) {
	static Vector<Command> a;
	static Vector<Command> b;
	a.clear();
	b.clear();

	uint64_t max = 0;
	int n_cmds = 0;

	for (auto cmds : cmds_set) {
		for (auto cmd : *(cmds)) {
			if (cmd.sortkey.sortkey > max) {
				max = cmd.sortkey.sortkey;
			}
			a.push_back(cmd);
		}
		n_cmds += cmds->size();
	}

	if (b.size() < n_cmds) {
		// a.resize(n_cmds);
		b.resize(n_cmds);
	}
	uint64_t bucket[16];
	enum : uint64_t {
		bitmask = 0xF
	};
	int bucket_max_index[16];
	uint64_t lsd = 0;
	
	while ((max >> (lsd * 4)) > 0)
	{
		memset(bucket, 0, 16 * sizeof(int));
		memset(bucket_max_index, 0, 16 * sizeof(int));

		uint64_t shift = lsd * 4;

		for (auto& c : a)
		{
			// p->sortkey.sortkey % pow(16, lsd + 1);
			++bucket[(c.sortkey.sortkey >> shift) & bitmask];
		}

		for (int i = 1; i < 16; ++i) {
			bucket[i] += bucket[i - 1];
		}

		for (int i = n_cmds - 1; i >= 0; --i) {
			b[--bucket[(a[i].sortkey.sortkey >> shift) & bitmask]] = a[i];
		}

		a.swap(b);

		max /= 16;
		++lsd;
	}

	return &a;
}
thread_local RenderContext tlsRenderContext;
RenderResource::resource_t* gResourceBuffer;

typedef void (*DrawCallFunc)(void*);
void FlushAllDrawCalls(const Vector<Command>& cmds) {
	for (auto cmd : cmds) {
		auto func = reinterpret_cast<DrawCallFunc*>(gResourceBuffer + cmd.offset);
		(*func)(gResourceBuffer + cmd.offset);
	}
}

void DrawMesh(MeshHandle mesh, ShaderHandle shader)
{
	Command cmd;
	cmd.sortkey.set_pass(tlsRenderContext.render_pass);
	cmd.sortkey.set_chstate(SortKey::Step::kDrawCall);
}

void UseShader(ShaderHandle shader)
{

}


void FlushAllDrawCalls()
{
	Vector<Vector<Command>*> cmds_set;
	cmds_set.push_back(&tlsRenderContext.commands);
	const auto sorted = RadixSort(cmds_set);
	FlushAllDrawCalls(*sorted);
}

RenderContext* GetRenderContext()
{
	return &tlsRenderContext;
}

template<typename T>
void SetUniformHelper(MaterialHandle md, T data) {
	auto& rc = tlsRenderContext;
	Command cmd;
	cmd.offset = rc.ResourcesOffset();
	cmd.sortkey.set_chstate(SortKey::kBindParam);
	cmd.sortkey.set_pass(rc.render_pass);
	rc.commands.push_back(cmd);
	MaterialDrawCall* mdc = rc.resources.Construct<MaterialDrawCall>(md, sizeof(MaterialDrawCall));
	void* off = rc.resources.Construct<T>(std::move(data));
}
template<>
void SetUniform<float>(MaterialHandle md, float data) {
	//TODO(L) Write msg
	NA_LEAVE_IF((void)0, GetMaterialDecriptor(md).type != MaterialType::kFloat, "");
	SetUniformHelper(md, data);
	return;
}

template<>
void SetUniform<int>(MaterialHandle md, int data) {
	NA_LEAVE_IF((void)0, GetMaterialDecriptor(md).type != MaterialType::kInt, "");
	SetUniformHelper(md, data);
	return;
}

template<>
void SetUniform<glm::vec3>(MaterialHandle md, glm::vec3 data) {
	NA_LEAVE_IF((void)0, GetMaterialDecriptor(md).type != MaterialType::kVec3, "");
	SetUniformHelper(md, data);
	return;
}

template<>
void SetUniform<glm::mat4>(MaterialHandle md, glm::mat4 data) {
	NA_LEAVE_IF((void)0, GetMaterialDecriptor(md).type != MaterialType::kMat4, "");
	SetUniformHelper(md, data);
	return;
}



namespace detail {

void PrepareRenderContext(void *pointer, size_t size) {
	tlsRenderContext.Reset(pointer, size);
}

}

ScopedState::ScopedState(SortKey::Step _render_state, RenderPass _render_pass)
	:last_state(tlsRenderContext.render_step, tlsRenderContext.render_pass)
{
	tlsRenderContext.render_step = _render_state;
	tlsRenderContext.render_pass = _render_pass;
}

ScopedState::~ScopedState()
{
	tlsRenderContext.render_step = last_state.render_step;
	tlsRenderContext.render_pass = last_state.render_pass;
}




}
}