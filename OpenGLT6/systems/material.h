#ifndef _NABLA_MATERIAL_SYSTERM_H_
#define _NABLA_MATERIAL_SYSTERM_H_
#include "isystem.h"
#include "core/renderer.h"
#include "containers/map.h"
#include <array>

namespace nabla {
#define NA_BUILTIN_MATERIAL_SYS_LIST(V)\
	V(diffuse,           Vec3,  (0.0f))   \
	V(specular,          Float, (0.0f))\
	V(albedo,            Vec3,  (1.0f)) \
	V(metallic,          Float, (1.0f)) \
	V(roughness,         Float, (0.07f)) \
	V(ambient_occulsion, Float, (0.9f))

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

	struct MaterialScope;
	struct MaterialPrototype {
		MaterialPrototype()
			: super(nullptr)
		{}

		template<typename T>
		MaterialPrototype Add(std::string name, renderer::MaterialHandle h, T def) {
			defaults.insert(defaults.end(), (char*)& def, ((ch bar*)& def) + sizeof(T));
			names.push_back(name);
			handles.push_back(h);
		}

		MaterialPrototype(Vector<std::string> _names, 
			              Vector<renderer::MaterialHandle> _handles) 
			: names(std::move(_names)), handles(_handles), super(nullptr)
		{}

		MaterialScope* super;
		Vector<std::string> names;
		Vector<renderer::MaterialHandle> handles;
		Vector<char> defaults;
	};

	struct PBRMaterial {
		renderer::MaterialHandle albedo_map;
		renderer::MaterialHandle metallic_map;
		renderer::MaterialHandle roughness_map;
		renderer::MaterialHandle ao_map;
	};

	struct Material {
		using Vec3 = glm::vec3;
		using Float = float;
#define DEF_MAT(name, t, def) t name = t def;
#define DEF_MAT_H(name, t, def) renderer::MaterialHandle h_##name;
		NA_BUILTIN_MATERIAL_SYS_LIST(DEF_MAT);
		// NA_BUILTIN_MATERIAL_SYS_LIST(DEF_MAT_H);
#undef DEF_MAT

		PBRMaterial pbr_map;

		renderer::MaterialHandle texture;
	};

	void Update(Entity e) override;

	bool Has(Entity e) const override;

	void OnGui(const Vector<Entity>& actives) override;

	const char* name() const override { return "Material"; }

	void SetUpMaterialHandles(renderer::ShaderHandle shader, Uniforms names);

	void RegisterMaterial(std::string scoped_name, MaterialPrototype proto);

	void Add(Entity, Material material);

	/*
	Material& GetEdit(Entity e) {
		NA_ASSERT(Has(e));
		return dense_[sparse_[e.index()]];
	}*/
	//TODO
	void Remove(Entity) override {}

	void Update(Clock& clock) override {}

	void Initialize(SystemContext&) override;

	void Add(Entity e) override { Add(e, Material()); }

	void AttachTexture(Entity e, renderer::MaterialHandle t);

	void AttachPBRTexture(Entity e, PBRMaterial pbrs);

private:
	// Material materials_;
#define DEF_MH(name, ...) renderer::MaterialHandle h##name##_;
	NA_BUILTIN_MATERIAL_SYS_LIST(DEF_MH)
#undef DEF_MH

	Vector<Entity::entity_t> sparse_;
	Vector<Material> dense_;

	

	struct MaterialScope {
		MaterialScope* super = nullptr;
		std::string name;
		Map<std::string, MaterialScope> subs;
		Map<std::string, MaterialPrototype> protos_;
	};

	struct MaterialInstance{
		MaterialPrototype* proto;
		void* data;
	};

	// Vector<MaterialInstance> dense_;
	
	MaterialScope root_mat_scope_;
	Map<std::string, Map<std::string, MaterialPrototype>> prototypes_;

	struct Data;
	Data* data_;
	SystemContext* ctx_;
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
