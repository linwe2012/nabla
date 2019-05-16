#ifndef _NABLA_LIGHTING_SYSTERM_H_
#define _NABLA_LIGHTING_SYSTERM_H_
#include "isystem.h"
#include "core/renderer.h"
#include "containers/map.h"

namespace nabla {
class LightingSystem : public ISysterm {
	using MaterialHandle = renderer::MaterialHandle;
	using MeshHandle = renderer::MeshHandle;
	using ShaderHanlde = renderer::ShaderHandle;
public:

	struct Light {
		enum Type : uint8_t {
			kDirection,
			kPoint,
			kSpot
		}; 
		Light() :Light(kPoint) {}
		Light(Type t) : type(t) {}
		glm::vec3 position = glm::vec3(1.0f);
		float yaw = 0.0f;
		float pitch = 0.0f;
		// glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f); /**< ingnored by point light*/
		glm::vec3 color = glm::vec3(0.0f);
		float linear = 0.5f;
		float quad = 0.05f;
		float radius = 0.1f;
		float cutoff = 0.1f; /**< used only by spot light */
		float outer_cutoff = 0.2f; /**< used only by spot light */
		Type type;
		bool draw_mesh = true;
		MeshHandle hmesh;
		glm::vec3 mesh_scale = glm::vec3(0.125f);
		std::string name = "";
		//display data:

	};

	void Initilize() override {
		num_spot_ = 0;
		num_point_ = 0;
		max_point_ = 32;
		max_spot_ = 32;
	}

	void SetShader(renderer::ShaderHandle lightingpass);

	// activities on gui, note that you can actually do nothing
	virtual void OnGui(const Vector<Entity>& actives);

	//TODO(L)
	virtual void Remove(Entity) override {}

	virtual bool Has(Entity e) override {
		return lights_.count(e);
	}

	// called upon every frame
	virtual void Update(Clock& clock) override;

	const Light* GetLight(Entity e) const {
		auto itr = lights_.find(e);
		if (itr == lights_.end()) {
			return nullptr;
		}
		return &itr->second;
	}

	void SetEyePos(glm::vec3 eye) {
		camera_pos_ = eye;
	}

	void NewLight(Entity e, Light l) {
		switch (l.type)
		{
		case Light::kPoint:
			if (l.name == "") {
				l.name = "PointLight";
				l.name += std::to_string(num_point_);
			}
			num_point_++;
			break;
		case Light::kSpot:
			if (l.name == "") {
				l.name = "SpotLight";
				l.name += std::to_string(num_spot_);
			}
			num_spot_++;
			break;
		default:
			break;
		}
		lights_[e] = l;
	}

private:

	void AddPointHandle(int id);

	void AddSpotLightHandle(int id);

	void DrawGuiPoint(Light&);

	void DrawGuiSpot(Light&);

	struct PointHandle {
		MaterialHandle position;
		MaterialHandle color;
		MaterialHandle linear;
		MaterialHandle quad;
		MaterialHandle radius;
	};

	struct SpotHandle {
		MaterialHandle position;
		MaterialHandle direction;
		MaterialHandle color;
		MaterialHandle linear;
		MaterialHandle quad;
		MaterialHandle cutoff;
		MaterialHandle outer_cutoff;
	};

	void RenderPoint(int id, const Light&);
	void RenderSpot(int id, const Light&);

	
	Map <Entity, Light> lights_;
	Vector<PointHandle> hpoint_;
	Vector<SpotHandle> hspot_;
	MaterialHandle hcamera_;
	glm::vec3 camera_pos_;

	int max_point_ = 0;
	int max_spot_ = 0;

	int num_spot_ = 0;
	int num_point_ = 0;

	MaterialHandle hnum_spot_;
	MaterialHandle hnum_point_;

	ShaderHanlde hshader_;
	renderer::RenderContext* rc_ = nullptr;
	
};
}
#endif // !_NABLA_LIGHTING_SYSTERM_H_
