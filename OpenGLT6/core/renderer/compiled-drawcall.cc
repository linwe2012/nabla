#include "compiled-drawcall.h"
#include "containers/map.h"
#pragma warning (push, 0)
#include <thread>
#include <algorithm>
#pragma warning (pop)
#include "utils.h"

namespace nabla {

namespace renderer {

const Vector<Command>* RadixSort(Vector<Command> *cmds, uint64_t max ) {
	static Vector<Command> b;
	b.clear();

	Vector<Command>& a = *cmds;

	int64_t n_cmds = static_cast<int64_t>(a.size());
	
	if (b.size() < a.size()) {
		b.resize(a.size());
	}

	uint64_t bucket[16];
	enum : uint64_t {
		bitmask = 0xF
	};
	//int bucket_max_index[16];
	uint64_t lsd = 0;
	
	while (max > 0)
	{
		memset(bucket, 0, 16 * sizeof(uint64_t));

		uint64_t shift = lsd * 4;

		for (auto& c : a)
		{
			// p->sortkey.sortkey % pow(16, lsd + 1);
			++bucket[(c.sortkey.sortkey >> shift) & bitmask];
		}

		for (int i = 1; i < 16; ++i) {
			bucket[i] += bucket[i - 1];
		}

		for (int64_t i = n_cmds - 1; i >= 0; --i) {
			b[--bucket[(a[i].sortkey.sortkey >> shift) & bitmask]] = a[i];
		}

		a.swap(b);

		max /= 16;
		++lsd;
	}

	return &a;
}
 /*thread_local*/ RenderContext tlsRenderContext;
static RenderResource::resource_t* gResourceBuffer = nullptr;
static Vector<Command> gDefaultCommands;
static FrameBufferHandle gDefaultGBufferHandle;

static RenderContext gReservedContext;
static RenderResource::resource_t* gReservedResourceBuffer = nullptr;

static Vector<Command> gCommandBuffer;


typedef void (*DrawCallFunc)(void*);
void FlushAllDrawCalls(const Vector<Command>& cmds) {
	for (auto cmd : cmds) {
		auto func = reinterpret_cast<DrawCallFunc*>(gResourceBuffer + cmd.offset);
		(*func)(gResourceBuffer + cmd.offset);
	}
}

void RecomputeOffset(const Vector<RenderContext*>& rcs, uint64_t* max, Vector<Command>* cmds) {
	for (auto rc : rcs) {
		int64_t offset = (RenderResource::resource_t*)rc->resources.buffer - (RenderResource::resource_t*)rcs[0]->resources.buffer;
		for (auto cmd : rc->commands) {
			cmd.offset += offset;
			(*cmds).push_back(cmd);
			if (cmd.sortkey.sortkey > * max) {
				*max = cmd.sortkey.sortkey;
			}
		}
	}
}

void FlushAllDrawCalls()
{
	gCommandBuffer.clear();
	
	uint64_t max_sortkey = 0;

	Vector<RenderContext*>rcs;
	rcs.push_back(&tlsRenderContext);
	rcs.push_back(&gReservedContext);

	RecomputeOffset(rcs, &max_sortkey, &gCommandBuffer);

	const auto sorted = RadixSort(&gCommandBuffer, max_sortkey);
	FlushAllDrawCalls(*sorted);
}

void FlushAllDrawCallsWithNoExtraCommands() {
	gCommandBuffer.clear();

	uint64_t max_sortkey = 0;

	Vector<RenderContext*>rcs;
	rcs.push_back(&tlsRenderContext);

	RecomputeOffset(rcs, &max_sortkey, &gCommandBuffer);

	const auto sorted = RadixSort(&gCommandBuffer, max_sortkey);
	FlushAllDrawCalls(*sorted);
}


void DrawMesh(MeshHandle mesh, ShaderHandle shader)
{
	Command cmd;
	cmd.sortkey.set_pass(tlsRenderContext.render_pass);
	//cmd.sortkey.set_chstate(SortKey::Step::kDrawCall);
	auto& rc = tlsRenderContext;
	cmd.offset = tlsRenderContext.ResourcesOffset();
	rc.commands.push_back(cmd);
	rc.resources.Construct<IndexedDrawCall>(shader, mesh);
}

void UseShader(ShaderHandle shader)
{
	Command cmd;
	
	//cmd.sortkey.set_chstate(SortKey::Step::kActivateShader);
	cmd.sortkey.set_pass(tlsRenderContext.render_pass);
	auto& rc = tlsRenderContext;
	cmd.offset = tlsRenderContext.ResourcesOffset();
	rc.commands.push_back(cmd);
	rc.resources.Construct<UseShaderDrawCall>(shader);
}

void UseTexture(MaterialHandle md, int id)
{
	{
		auto type = GetMaterialDecriptor(md).type;
		NA_LEAVE_IF((void)0, type != MaterialType::kSampler2D && type != MaterialType::kSamplerCubic, "");
	}
	
	auto& rc = tlsRenderContext;
	Command cmd;
	cmd.offset = rc.ResourcesOffset();
	cmd.sortkey.set_pass(rc.render_pass);
	rc.commands.push_back(cmd);
	MaterialDrawCall* mdc = rc.resources.Construct<MaterialDrawCall>(md, static_cast<MaterialDrawCall::offset_t>(0), id);
	(void)mdc;
}

void ReadFromDefaultGBufferAttachment(int id, const std::function<void()>& callback)
{
	auto& rc = tlsRenderContext;
	Command cmd;
	cmd.offset = rc.ResourcesOffset();
	cmd.sortkey.set_pass(rc.render_pass);
	cmd.sortkey.set_depth(SortKey::kDepthMax);
	rc.commands.push_back(cmd);
	rc.resources.Construct<FrameBufferAttachmentReaderDrawCall>(gDefaultGBufferHandle, id, callback);
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
	//cmd.sortkey.set_chstate(SortKey::kBindParam);
	cmd.sortkey.set_pass(rc.render_pass);
	rc.commands.push_back(cmd);
	MaterialDrawCall* mdc = rc.resources.Construct<MaterialDrawCall>(md, static_cast<MaterialDrawCall::offset_t>(sizeof(MaterialDrawCall)));
	void* off = rc.resources.Construct<T>(std::move(data));
	(void)off;
	(void)mdc;
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
void SetUniform<glm::vec4>(MaterialHandle md, glm::vec4 data)
{
	NA_LEAVE_IF((void)0, GetMaterialDecriptor(md).type != MaterialType::kVec4, "");
	SetUniformHelper(md, data);
	return;
}

template<>
void SetUniform<glm::mat4>(MaterialHandle md, glm::mat4 data) {
	NA_LEAVE_IF((void)0, GetMaterialDecriptor(md).type != MaterialType::kMat4, "");
	SetUniformHelper(md, data);
	return;
}

void SetDefaultGBuffer(FrameBufferHandle hf) {
	if (gReservedResourceBuffer == nullptr) {
		gReservedResourceBuffer = new RenderResource::resource_t[128];
		gReservedContext.Reset(gReservedResourceBuffer, 128);
	}

	gDefaultGBufferHandle = hf;
	Command cmd;
	cmd.sortkey.set_pass_low_priority(false);
	cmd.sortkey.set_pass(RenderPass::kForward);
	
	gReservedContext.AddCmd<SwitchFrameBufferDrawCall>(cmd, SwitchFrameBufferDrawCall::kRenderOnFrameBuffer, hf);
	
	cmd.sortkey.set_pass(RenderPass::kDeferred);
	gReservedContext.AddCmd<SwitchFrameBufferDrawCall>(cmd, SwitchFrameBufferDrawCall::kSampleFromFrameBuffer, hf);

	cmd.sortkey.set_pass(RenderPass::kPostProc);
	gReservedContext.AddCmd<SwitchFrameBufferDrawCall>(cmd, SwitchFrameBufferDrawCall::kCopyFrameBufferToDefault, hf);
}


namespace detail {

void PrepareRenderContext(void *pointer, size_t size) {
	tlsRenderContext.Reset(pointer, size);
}

void PrepareRenderContext__Temp()
{
	if (gResourceBuffer == nullptr) {
		gResourceBuffer = new RenderResource::resource_t[65536 * 4];
	}
	tlsRenderContext.Reset(gResourceBuffer, 65536 * 4);
}



}

ScopedState::ScopedState(RenderPass _render_pass)
	:last_state(tlsRenderContext.render_step, tlsRenderContext.render_pass)
{
	tlsRenderContext.render_pass = _render_pass;
}

ScopedState::ScopedState(SortKey::Step _render_state, RenderPass _render_pass)
	:last_state(tlsRenderContext.render_step, tlsRenderContext.render_pass)
{
	tlsRenderContext.render_step = _render_state;
	tlsRenderContext.render_pass = _render_pass;
}

ScopedState::ScopedState(State state)
{
	auto& rc = tlsRenderContext;
	last_state.render_pass = rc.render_pass;
	last_state.render_step = rc.render_step;
	
	Command cmd;
	is_state_changed = true;
	cmd.offset = rc.ResourcesOffset();
	cmd.sortkey.set_pass(rc.render_pass);
	//cmd.sortkey.set_chstate(SortKey::Step::kStateChangeBegin);
	rc.resources.Construct<StateDrawCall>(state);
	rc.commands.push_back(cmd);
}

ScopedState::~ScopedState()
{
	if (is_state_changed) {
		auto& rc = tlsRenderContext;
		Command cmd;
		cmd.offset = rc.ResourcesOffset();
		cmd.sortkey.set_pass(rc.render_pass);
		rc.resources.Construct<StateDrawCall>(State(0));
		rc.commands.push_back(cmd);
	}

	tlsRenderContext.render_step = last_state.render_step;
	tlsRenderContext.render_pass = last_state.render_pass;
	
}




}
}