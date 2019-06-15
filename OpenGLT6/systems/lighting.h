#ifndef _NABLA_LIGHTING_SYSTERM_H_
#define _NABLA_LIGHTING_SYSTERM_H_
#include "isystem.h"
#include "core/renderer.h"
#include "containers/map.h"

namespace nabla {
class LightingSystem : public ISystem {
public:
	using MaterialHandle = renderer::MaterialHandle;
	using MeshHandle = renderer::MeshHandle;
	using ShaderHanlde = renderer::ShaderHandle;

	struct Light {
		enum Type : uint8_t {
			kDirection,
			kPoint,
			kSpot
		}; 

		using vec3 = glm::vec3;

		Light() :Light(kPoint) {}
		Light(Type t) : type(t) {}
		Light(Type t, MeshHandle _hmesh) : type(t), hmesh(_hmesh) {}
		Light(Type t, MeshHandle _hmesh, vec3 _color) 
			: type(t), hmesh(_hmesh), color(_color){}

		vec3 position = glm::vec3(1.0f);
		vec3 color    = glm::vec3(0.0f);

		// euler angles
		float yaw    = 0.0f;
		float pitch  = 0.0f;

		// attenuation infos
		float linear = 0.5f;
		float quad   = 0.05f;
		float radius = 0.1f;

		// the inner & outter cone of spot light
		float cutoff       = 0.1f; /**< used only by spot light */
		float outer_cutoff = 0.2f; /**< used only by spot light */

		// describe light type, which may result in some feature disabled
		Type type;
		bool draw_mesh = false;
		MeshHandle hmesh;
		glm::vec3 mesh_scale = glm::vec3(0.125f);
		std::string name;
	};

	void Initialize(SystemContext&) override;

	void SetShader(renderer::ShaderHandle lightingpass, renderer::ShaderHandle prostproc, MaterialHandle hskybox);

	// activities on gui, note that you can actually do nothing
	virtual void OnGui(const Vector<Entity>& actives);

	virtual bool Has(Entity e) const override {
		return lights_.count(e);
	}

	void Add(Entity) override {}

	// called upon every frame
	virtual void Update(Clock& clock) override;

	const char* name() const override { return "Lighting"; }

	const Light* GetLight(Entity e) const;

	void SetEyePos(glm::vec3 eye, glm::mat4 project, glm::mat4 view) {
		camera_pos_ = eye;
		project_ = project;
		view_ = view;
	}

	void NewLight(Entity e, Light l);

	//TODO(L)
	virtual void Remove(Entity) override {}

	void Update(Entity) override {}

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
	MaterialHandle hirradiance_;
	MaterialHandle hskybox_texture_;
	glm::vec3 camera_pos_;
	glm::mat4 project_;
	glm::mat4 view_;

	int max_point_ = 0;
	int max_spot_ = 0;

	int num_spot_ = 0;
	int num_point_ = 0;

	MaterialHandle hnum_spot_;
	MaterialHandle hnum_point_;

	ShaderHanlde hshader_;
	renderer::ShaderHandle hpostprocess_;
	renderer::RenderContext* rc_ = nullptr;

	MaterialHandle hbox_proj_;
	MaterialHandle hbox_view_;
	MaterialHandle hbox_model_;
	MaterialHandle hbox_lightcolor_;
	renderer::IBLMapComputResult hibls_;
};

inline const LightingSystem::Light* LightingSystem::GetLight(Entity e) const {
	auto itr = lights_.find(e);
	if (itr == lights_.end()) {
		return nullptr;
	}
	return &itr->second;
}


}
#endif // !_NABLA_LIGHTING_SYSTERM_H_
