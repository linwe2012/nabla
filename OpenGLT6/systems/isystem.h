#ifndef _NABLA_SYSTERM_INTERFACE_H_
#define _NABLA_SYSTERM_INTERFACE_H_
#include "containers/vector.h"
#include "core/entity.h"
#include <chrono>



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
		current_ = begin_ = steady_clock::now();
		total_frame_count_ = 0;
	}

	void NextFrame() {
		last_ = current_;
		current_ = steady_clock::now();
		++total_frame_count_;
	}

	double NanoSecond() const {
		return std::chrono::duration_cast<nano>(current_ - begin_).count();
	}

	double MilliSecond() const {
		return std::chrono::duration_cast<micro>(current_ - begin_).count() * 0.001;
	}

	double Second() const {
		return std::chrono::duration_cast<milli>(current_ - begin_).count() * 0.001;
	}

	double Time() const {
		return Second();
	}

	uint64_t CountFrames() {
		return total_frame_count_;
	}

	double GetLastFrameDuration() {
		return std::chrono::duration_cast<milli>(current_ - last_).count() * 0.001;
	}

	double GetLastFrameFps() {
		return 1.0 / GetLastFrameDuration();
	}

	steady_time_point GetCurrentTimeRaw() {
		return current_;
	}

private:
	uint64_t total_frame_count_ = 0;

	steady_time_point current_;
	steady_time_point last_;
	steady_time_point begin_;
};

class ISystem {
public:
	// called upon system first registered
	virtual void Initilize() = 0;

	// activities on gui, note that you can actually do nothing
	virtual void OnGui(const Vector<Entity>& actives) = 0;

	// remove enitiy from system
	virtual void Remove(Entity) = 0;

	virtual bool Has(Entity) const = 0;

	virtual void Update(Entity) = 0;

	// called upon every frame
	virtual void Update(Clock& clock) = 0;

	virtual const char* name() const = 0;
};
}


#endif // !_NABLA_SYSTERM_INTERFACE_H_
