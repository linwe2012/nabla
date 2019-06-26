#ifndef _NABLA_CORE_COMPILED_DRAW_H_
#define _NABLA_CORE_COMPILED_DRAW_H_

#include "render-resources.h"
#include "drawcall.h"
#include "containers/vector.h"
#include <memory>

namespace nabla {
namespace renderer {

struct SortKey {
	/** |<----depth bit---->|
	(MSB)                (48)                  (32)                  (16)           (LSB)
	00000000  0000 0000  0000 0000  0000 0000  0000 0000  0000 0000  0000 0000  0000 0000
	                                    ^|<----depth bit---->| |<----target id---->| ^^^^
								        |                                              |
								        +---- 1: deferred 0: forward render                +---- change state bit
	*/

	enum Step {
		kStateChangeBegin,
		kActivateShader,
		kBindParam,
		kDrawCall,
		kStateChangeEnd
	};

	uint64_t sortkey = 0;

	bool operator<(const SortKey& rhs) const { return sortkey < rhs.sortkey; }
	bool operator>(const SortKey& rhs) const { return sortkey > rhs.sortkey; }
	bool operator==(const SortKey& rhs) const { return sortkey == rhs.sortkey; }
	bool operator!=(const SortKey& rhs) const { return !operator==(rhs); }

	void set_depth(uint16_t depth) {
		set_bits<20, 4>(depth);
	}

	void set_target(uint16_t handle_id) {
		set_bits<36, 20>(handle_id);
	}

	void set_pass(RenderPass rp) {
		set_bits<40, 36>(static_cast<uint8_t>(rp));
	}
	void set_chstate(Step state) {
		set_bits<4, 0>(state);
	}

	template <int high , int low>
	void set_bits(uint64_t key) {
		constexpr uint64_t mask = get_bitmask<high, low>();
		sortkey &= ~mask;
		key <<= low;
		key &= mask;
		sortkey |= key;
	}

	template <int high, int low>
	uint64_t get_bits() {
		constexpr uint64_t mask = get_bitmask<high, low>();
		return (sortkey << low) & mask;
	} 

	template<int high, int low>
	constexpr static uint64_t get_bitmask() {
		constexpr uint64_t uint64_max = static_cast<uint64_t>(-1);
		uint64_t res = uint64_max;
		constexpr int len = high - low;
		constexpr int sar = sizeof(uint64_t) * 8 - len;
		res >>= sar;
		res <<= low;
		return res;
	}
};

struct Command{
	SortKey sortkey;
	uint64_t offset = 0;
	bool operator<(const Command& rhs) const { return sortkey < rhs.sortkey; }
};


struct RenderResource {
	using resource_t = uint64_t;
	enum
	{
		kAlign = sizeof(resource_t),
	};
	RenderResource(void* buffer_, size_t size_by_bytes) {
		Reset(buffer_, size_by_bytes);
	}

	RenderResource() : buffer(nullptr), next(nullptr), end_of_storage(nullptr){}

	void SetBuffer(void* buffer_, size_t size_by_bytes) {
		buffer = buffer_;
		next = static_cast<uint64_t*>(buffer);
		end_of_storage = next + size_by_bytes / kAlign;
	}

	void WriteRaw(void *data, size_t size_by_char) {
		size_t aligned = (size_by_char - 1 + kAlign) / kAlign;
		memmove(next, data, size_by_char);
		next += aligned;
	}

	void Reset(void *new_buffer, size_t size_by_bytes) {
		buffer = new_buffer;
		next = static_cast<resource_t*>(buffer);
		end_of_storage = next + size_by_bytes / kAlign;
	}

	template<typename T>
	void Write(T&& data) {
		static_assert(alignof(T) <= kAlign, "alignment exceeds limit");
		constexpr size_t aligned = (sizeof(T) - 1 + kAlign) / kAlign;
		::new (next) T(data);
		// *(reinterpret_cast<T*>(next)) = data;
		next += aligned;
	}

	template<typename T, typename... Args>
	T* Construct(Args... args) {
		constexpr size_t aligned = (sizeof(T) - 1 + kAlign) / kAlign;
		::new (next) T( args... );
		next += aligned;
		return reinterpret_cast<T*>(next - aligned);
	}

	template<typename T, typename... Args>
	void ListCtor(Args... args) {
		constexpr size_t aligned = (sizeof(T) - 1 + kAlign) / kAlign;
		::new (next) T{ args... };
		next += aligned;
	}

	size_t Offset() {
		return (uint64_t*)next - (uint64_t*)buffer;
	}

	void* GetCurrentAddress() {
		return next;
	}

	template<typename T>
	T* GetOffset(size_t off) {
		void* pos = static_cast<uint64_t*>(buffer) + off;
		return static_cast<T*>(pos);
	}

	void* buffer;
	resource_t* next;
	resource_t* end_of_storage;
};

struct SmartContext;

struct RenderState {
	RenderState(SortKey::Step _render_step, RenderPass _render_pass)
		:render_step(_render_step), render_pass(_render_pass), is_valid(true) {}
	RenderState() : is_valid (false), render_step(SortKey::Step::kDrawCall), render_pass(RenderPass::KForward) {}
	SortKey::Step render_step;
	RenderPass render_pass;
	bool is_valid;
};

struct ScopedState {

	RenderState last_state;

	ScopedState(SortKey::Step _render_step, RenderPass _render_pass);
	~ScopedState();
};

struct RenderContext {
	Vector<Command> commands;
	RenderResource resources;
	bool initialized;
	RenderContext(): initialized(false){}
	RenderContext(void* buffer_, size_t size_by_bytes): resources(buffer_, size_by_bytes), initialized(true){}
	
	bool Isinitialized() { return initialized; }
	
	template<typename T, typename... Resources>
	void AddCmd(Command cmd, Resources... src) {
		cmd.offset = resources.Offset();
		commands.push_back(std::move(cmd));
		resources.Construct(src...);
	}

	void Reset(void *new_resource, size_t size_by_bytes) {
		commands.clear();
		resources.Reset(new_resource, size_by_bytes);
	}

	size_t ResourcesOffset() {
		return resources.Offset();
	}

	SmartContext NewTarget(MeshHandle hmesh);
	uint16_t base = 10;
	uint16_t tid = base;
	SortKey::Step render_step = SortKey::Step::kDrawCall;
	RenderPass render_pass = RenderPass::KForward;
};

template <typename T>
void SetUniform(MaterialHandle md, T unknown_type) {
	static_assert(false, "Unsupported Type");
}

template <>
void SetUniform<float>(MaterialHandle md, float data);


template <>
void SetUniform<int>(MaterialHandle md, int data);

template <>
void SetUniform<glm::vec3>(MaterialHandle md, glm::vec3 data);

template <>
void SetUniform<glm::mat4>(MaterialHandle md, glm::mat4 data);


class SmartContext {
public:

	SmartContext(RenderContext* rc, MeshHandle target, uint16_t _target_id)
		:rc_(rc), target_id(_target_id)
	{
		Command cmd;
		cmd.sortkey.set_target(target_id);
		cmd.sortkey.set_chstate(SortKey::kDrawCall);
		rc->commands.push_back(cmd);
		rc->resources.Construct<IndexedDrawCall>(target);
		init_offset = rc->ResourcesOffset();
	}

	SmartContext(RenderContext* rc) 
		:rc_(rc)
	{
		init_offset = 0;
		target_id = 0;
	}

	SmartContext& Attach(ShaderHandle hshader) {
		rc_->resources.GetOffset<IndexedDrawCall>(init_offset)->hshader = hshader;
		return *this;
	}

	SmartContext& ModelMat(glm::mat4 mat) {
		rc_->resources.GetOffset<IndexedDrawCall>(init_offset)->model = std::move(mat);
		return *this;
	}


	SmartContext& SetTexture(MaterialHandle md) {
		//TODO(L) Write msg
		NA_LEAVE_IF(*this, GetMaterialDecriptor(md).type != MaterialType::kSampler, "");
		Command cmd;
		cmd.offset = rc_->ResourcesOffset();
		cmd.sortkey.set_target(target_id);
		cmd.sortkey.set_chstate(SortKey::kBindParam);
		
		rc_->resources.Construct<MaterialDrawCall>(md, 0);
	}

	SmartContext& SetUniform(MaterialHandle md, float data) {
		//TODO(L) Write msg
		NA_LEAVE_IF(*this, GetMaterialDecriptor(md).type != MaterialType::kFloat, "");
		return SetUniformHelper(md, data);
	}

	SmartContext& SetUniform(MaterialHandle md, int data) {
		//TODO(L) Write msg
		NA_LEAVE_IF(*this, GetMaterialDecriptor(md).type != MaterialType::kInt, "");
		return SetUniformHelper(md, data);
	}

	SmartContext& SetUniform(MaterialHandle md, glm::vec3 data) {
		//TODO(L) Write msg
		NA_LEAVE_IF(*this, GetMaterialDecriptor(md).type != MaterialType::kVec3, "");
		return SetUniformHelper(md, data);
	}

	SmartContext& SetUniform(MaterialHandle md, glm::mat4 data) {
		//TODO(L) Write msg
		NA_LEAVE_IF(*this, GetMaterialDecriptor(md).type != MaterialType::kMat4, "");
		return SetUniformHelper(md, data);
	}

	// SmartContext& EnbalePBR(MaterialHandle Metallic,
	// 	                    MaterialHandle Roughness,
	// 	                    MaterialHandle AmbientOcclusion,
	// 	                    MaterialHandle Albedo) {
	// 	auto pbr = rc_->resources.Construct<PBRDrawCall>();
	// 	return *this;
	// }
	

private:

	template<typename T>
	SmartContext& SetUniformHelper(MaterialHandle md, T data) {
		Command cmd;
		cmd.offset = rc_->ResourcesOffset();
		cmd.sortkey.set_chstate(SortKey::kBindParam);
		cmd.sortkey.set_target(target_id);
		rc_->commands.push_back(cmd);
		MaterialDrawCall* mdc = rc_->resources.Construct<MaterialDrawCall>(md, sizeof(MaterialDrawCall));
		void* off = rc_->resources.Construct<T>(std::move(data));
		// mdc->offset_by_bytes = sizeof(MaterialDrawCall);
		return *this;
	}

	RenderContext* rc_;
	uint32_t init_offset;
	uint16_t target_id;
};

inline SmartContext RenderContext::NewTarget(MeshHandle hmesh) {
	return SmartContext(this, hmesh, tid++);
};

// this should be called once per frame
void FlushAllDrawCalls();

// RenderContext* GetRenderContext();

namespace detail {
	void PrepareRenderContext(void* pointer, size_t size);
}

}



}


#endif // !_NABLA_CORE_COMPILED_DRAW_H_
