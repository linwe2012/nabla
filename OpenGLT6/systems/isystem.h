#ifndef _NABLA_SYSTERM_INTERFACE_H_
#define _NABLA_SYSTERM_INTERFACE_H_
#include "containers/vector.h"
#include "core/entity.h"
#include <chrono>
#include <thread>
#include <mutex>


namespace nabla {

// simple wrapper over stl's chrono
class Clock {
public:
	using steady_clock = std::chrono::steady_clock;
	using steady_time_point = std::chrono::time_point<steady_clock>;
	using nano = std::chrono::nanoseconds;
	using milli = std::chrono::milliseconds;
	using micro = std::chrono::microseconds;
	using second = std::chrono::seconds;
	using minute = std::chrono::minutes;

	void Gensis() {
		last_ = begin_ = gensis_ = steady_clock::now();
		total_frame_count_ = 0;
	}

	void NextFrame() {
		last_ = begin_;
		begin_ = steady_clock::now();
		++total_frame_count_;
	}

	double NanoSecond() const {
		return static_cast<double>(std::chrono::duration_cast<nano>(begin_ - gensis_).count());
	}

	double MilliSecond() const {
		return std::chrono::duration_cast<micro>(begin_ - gensis_).count() * 0.001;
	}

	double Second() const {
		return std::chrono::duration_cast<milli>(begin_ - gensis_).count() * 0.001;
	}

	double Time() const {
		return Second();
	}

	uint64_t CountFrames() const {
		return total_frame_count_;
	}

	double GetLastFrameDuration() const {
		return std::chrono::duration_cast<milli>(begin_ - last_).count() * 0.001;
	}

	float GetLastFrameDurationFloat() const {
		return static_cast<float>(GetLastFrameDuration());
	}

	double GetLastFrameFps() const {
		return 1.0 / GetLastFrameDuration();
	}

	steady_time_point GetCurrentTimeRaw() const {
		return begin_;
	}

	void set_fps(int fps) { fps_ = fps; sec_per_frame_ = 1.0 / fps_; }

	int fps() const { return fps_; }

	double ThisFrameElapse() const {
		return std::chrono::duration_cast<micro>(begin_ - std::chrono::steady_clock::now()).count() * 0.001 * 0.001;
	}

	bool IsTimeout() const {
		return ThisFrameElapse() >= sec_per_frame_;
	}

private:
	uint64_t total_frame_count_ = 0;

	steady_time_point begin_; //< begin of the frame
	steady_time_point last_; 
	steady_time_point gensis_; //< begin of the FIRST Frame
	int fps_; //< desired fps
	double sec_per_frame_;
};

class RenderableSystem;
class EntityManager;
class AssetManager;
class MatrialSysterm;

struct BootstrapStatus {
	bool done = false;
	std::mutex render_job;
};

struct SystemContext {
	RenderableSystem* render;
	MatrialSysterm* material;
	EntityManager* entity_manager;
	AssetManager* assets;
	BootstrapStatus* status;
	Clock* clock;
};

class ISystem {
public:
	
	virtual void Initilize() {};

	virtual void Initialize(SystemContext&) {}

	// activities on gui, note that you can actually do nothing
	virtual void OnGui(const Vector<Entity>& actives) = 0;

	// remove enitiy from system
	virtual void Remove(Entity) = 0;

	virtual bool Has(Entity) const = 0;

	virtual void Update(Entity) = 0;

	// called upon every frame
	virtual void Update(Clock& clock) = 0;

	virtual void Add(Entity) = 0;

	virtual const char* name() const = 0;
};
}


#endif // !_NABLA_SYSTERM_INTERFACE_H_
