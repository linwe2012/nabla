#ifndef _NABLA_SYSTEM_PLAYBACK_H_
#define _NABLA_SYSTEM_PLAYBACK_H_
#include "isystem.h"
#include <thread>
#include <future>
#include "core/renderer.h"

namespace nabla {
class PlaybackSystem : public ISystem {
public:

	// activities on gui, note that you can actually do nothing
	virtual void OnGui(const Vector<Entity>& actives) override;

	// remove enitiy from system
	virtual void Remove(Entity) override {};

	virtual bool Has(Entity) const override { return false; };

	virtual void Update(Entity) override {};

	// called upon every frame, update collide object
	void Update(Clock& clock) override;

	void Add(Entity) override {};

	const char* name() const override  {
		return "playback";
	}

	void Initialize([[maybe_unused]] SystemContext&);
	
	bool IsRecording() {
		return hidden_video_streamer_ != nullptr;
	}

private:

	bool screenshot_button_ = false;
	bool record_button_ = false;
	std::future<void> write_screenshot_promise_;
	std::future<void> write_video_streamer_promise_;
	Vector<renderer::SolidPixel> screenshot_buffer_;
	int width_ = 0;
	int height_ = 0;
	void* hidden_video_streamer_ = nullptr;
	
};
}
#endif // !_NABLA_SYSTEM_PLAYBACK_H_

