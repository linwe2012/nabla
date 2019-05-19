#ifndef _NABLA_MATERIAL_SYSTERM_H_
#define _NABLA_MATERIAL_SYSTERM_H_
#include "isystem.h"
#include "core/renderer.h"

namespace nabla {
#define NA_BUILTIN_MATERIAL_SYS_LIST(V)\
	V(diffuse,           Vec3,  (1.0f))   \
	V(specular,          Float, (0.2f))\
	V(albedo,            Vec3,  (0.0f)) \
	V(metallic,          Float, (0.0f)) \
	V(roughness,         Float, (0.0f)) \
	V(ambient_occulsion, Float, (0.0f))

class MatrialSysterm : public ISystem {
public:
	struct Uniforms {
		/*
#define DEF_UNI(name, ...) const char * name;
		NA_BUILTIN_MATERIAL_SYS_LIST(DEF_UNI)
#undef DEF_UNI */
		//Expands to: 
		const char* diffuse = "Diffuse";
		const char* specular = "Specular";
		const char* albedo = "Albedo";
		const char* metallic = "Metallic";
		const char* roughness = "Roughness";
		const char* ambient_occulsion = "AO";
		
	};

	struct Material {
		using Vec3 = glm::vec3;
		using Float = float;
#define DEF_MAT(name, t, def) t name = t def;
		NA_BUILTIN_MATERIAL_SYS_LIST(DEF_MAT)
#undef DEF_MAT
	};

	void Update(Entity e) override;

	bool Has(Entity e) const override;

	void OnGui(const Vector<Entity>& actives) override;

	const char* name() const override { return "Material"; }

	void SetUpMaterialHandles(renderer::ShaderHandle shader, Uniforms names);

	void Add(Entity, Material material = Material());

	Material& GetEdit(Entity e) {
		NA_ASSERT(Has(e));
		return dense_[sparse_[e.index()]];
	}
	//TODO
	void Remove(Entity) override {}

	void Update(Clock& clock) override {}

	void Initilize() override {}

private:
	Material materials_;
#define DEF_MH(name, ...) renderer::MaterialHandle h##name##_;
	NA_BUILTIN_MATERIAL_SYS_LIST(DEF_MH)
#undef DEF_MH

	Vector<Entity::entity_t> sparse_;
	Vector<Material> dense_;
};

inline bool MatrialSysterm::Has(Entity e) const {
	if (e.index() >= sparse_.size()) {
		return false;
	}
	if (sparse_[e.index()] == Entity::kInvalidIndex) {
		return false;
	}
	return true;
}

}

#endif // !_NABLA_MATERIAL_SYSTERM_H_
