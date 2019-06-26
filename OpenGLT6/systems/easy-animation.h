#pragma once
#include "isystem.h"
#include "core/sparse-map.h"
#include "components/skeleton.h"
#include "editor/gui.h"
#include "components/primitive.h"
#include "renderable.h"

namespace nabla {

class EasyAnimationSystem : public ISystem {
public:
	void Add(Entity e) override {
		AnimatableComponent a;
		a.sampler = [](float dt) -> float{
			return dt;
		};
		animates_.Add(e, a);
	}

	// called upon system first registered
	void Initialize(SystemContext& ctx) override {
		ctx_ = &ctx;
	}

	// activities on gui, note that you can actually do nothing
	virtual void OnGui(const Vector<Entity>& actives) override {
		if (actives.size() == 0) {
			return;
		}

		
		if (ImGui::CollapsingHeader("Animation")) {
			for (auto e : actives) {
				if (!animates_.count(e)) {
					if (ImGui::Button("Add Animation")) {
						Add(e);
					}
					continue;
				}

				auto& a = animates_[e].component;
				ImGui::DragFloat3("Scale", &a.scale.x);
				ImGui::DragFloat3("Trans", &a.translate.x);
				ImGui::DragFloat3("Euler", &a.euler.x);
				ImGui::DragFloat("Time", &a.time);
				ImGui::Checkbox("Repeat", &a.repeat);
				ImGui::Checkbox("Active", &a.active);
			}
		}
	}

	// remove enitiy from system
	virtual void Remove(Entity) override {};

	virtual bool Has(Entity) const override { return false; };

	virtual void Update(Entity) override {
		
	};

	// called upon every frame
	void Update(Clock& clock) override {
		auto dt = clock.GetLastFrameDurationFloat();

		for (auto& r : animates_) {
			auto& a = r.component;
			if (!a.active) {
				continue;
			}

			if (a.time == 0.f) {
				continue;
			}

			if (a.time_used > a.time) {
				a.time_used = 0.f;
				if (!a.repeat) {
					a.active = false;
					continue;
				}
				else {
					a.RepeatReverse();
				}
			}
			Transform* trans = ctx_->render->GetTransformEdit(r.entity);
			if (trans == nullptr) {
				continue;
			}
			
			a.time_used += dt;
			float dv = a.sampler(dt / a.time);
			trans->scale += a.scale * dv;
			trans->position += a.translate * dv;
			glm::quat QuatAroundX = glm::quat(1.0, 0.0, 0.0, a.euler.x);
			glm::quat QuatAroundY = glm::quat(0.0, 1.0, 0.0, a.euler.y);
			trans->quaternion += glm::quat(a.euler) * dv;
		}
	};

	const char* name() const override {
		return "easy animation";
	}



private:
	SparseBindirectMap<AnimatableComponent> animates_;

	SystemContext* ctx_ = nullptr;

};

}
