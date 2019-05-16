#ifndef _NABLA_SYSTEM_ANIMATION_H_
#define _NABLA_SYSTEM_ANIMATION_H_

#include <tuple>

#include "core/component-registry.h"
#include "components/skeleton.h"
#include "core/service-locator.h"

#include "glm/gtx/transform.hpp"

namespace nabla {

class AnimationSystem {
public:
	AnimationSystem() 
	{
		component_registery_ = GlobalServices.Locate<ComponentRegistry>("ComponentRegistry");
		skeleton_handle = component_registery_->Expose("animation.localpose", &shared_localpose_);
		animate_handle = component_registery_->Expose("animation.animate", &shared_animate_);

		shared_localpose_.entity_to_component_id = &map_;
		shared_animate_.entity_to_component_id = &map_;

		map_.resize(128, Entity::kInvalidIndex);
	};

	void setParent(Entity child, Entity parent) {

	}

	void add(Entity e, /*LocalPoseComponent skeleton,*/ AnimatableComponent animate_info) {
		// we choose linear sampler by default
		if (animate_info.sampler == nullptr) {
			animate_info.sampler = [](float x) { return x; };
		}

		localpose_.push_back(LocalPoseComponent{});
		animates_.push_back(std::move(animate_info));
		time_used_.push_back(0);
		if (e.index() > map_.size()) {
			auto new_size = e.index();
			new_size = new_size > Entity::kMaxEntities ? Entity::kMaxEntities : new_size;
			map_.resize(new_size, Entity::kInvalidIndex);
		}
		map_[e.index()] = localpose_.size() - 1;
		shared_localpose_.data.set(localpose_.begin());
		shared_animate_.data.set(animates_.begin());
	}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               

	LocalPoseComponent& getSkeleton(Entity e) { return localpose_[e.index()]; }
	AnimatableComponent& getAnimate(Entity e) { return animates_[e.index()]; }

	std::tuple<LocalPoseComponent*, AnimatableComponent*>
		begin() { return std::make_tuple(localpose_.begin(), animates_.begin()); }
	
	std::tuple<LocalPoseComponent*, AnimatableComponent*>
		end() { return std::make_tuple(localpose_.end(), animates_.end()); }

	void remove(Entity e) {
		//TODO
	}

	void update(float dt /* delta time */)
	{
		size_t n = 0;
		for (auto& animate : animates_) {
			++n;

			if (!animate.active) continue;
			if (time_used_[n - 1] > animate.time) {
				if (!animate.repeat) {
					animate.active = false;
					continue;
				}
				else {
					time_used_[n - 1] = 0.0f;
				}
			}

			float delta = time_used_[n - 1] / animate.time;
			float sample = animate.sampler(delta);

			auto& local = localpose_[n - 1].local_pose;

			local = glm::mat4(1.0f);
			local = glm::scale(local, sample * animate.scale);
			local = glm::rotate(local, sample * animate.rad, animate.rotate_axis);
			local = glm::translate(local, sample * animate.translate);

			animate.time += dt;
		}
	}

private:
	SharableComponent<LocalPoseComponent> shared_localpose_;
	SharableComponent<AnimatableComponent> shared_animate_;

	ComponentRegistry::Handle skeleton_handle;
	ComponentRegistry::Handle animate_handle;

	Vector<LocalPoseComponent> localpose_;
	Vector<AnimatableComponent> animates_;

	Vector<float> time_used_;
	Vector<Entity::entity_t> map_;
	ComponentRegistry* component_registery_;
};

}


#endif // !_NABLA_SYSTEM_ANIMATION_H_
