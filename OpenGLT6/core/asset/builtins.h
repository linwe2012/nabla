#ifndef _NABLA_CORE_ASSETS_BUILTINS_H_
#define _NABLA_CORE_ASSETS_BUILTINS_H_

#include "core/renderer.h"
namespace nabla {

// property  |   IsMap      IsUniform    CanToggle     detail
//   I           Maybe       Maybe          Yes         it could be a texture map or just a uniform value applied on every vertices
//   M            Yes          No           Yes         it must be a map
//   P           Maybe       Maybe          -->         All PBR material must be Toggled on/off simultanously
#define NA_BUILTIN_TEXTURE_LIST(I /*interchangele*/, M /*map*/, P /*PBR*/) \
	I(Diffuse,  glm::vec3, DIFFUSE)   \
	I(Specular, glm::vec3, SPECULAR)   \
	I(Ambient,  glm::vec3, AMBIENT)   \
	I(Emissive, glm::vec3, EMISSIVE)   \
	I(Height,   glm::vec3, HEIGHT)   \
	M(Normal,   glm::vec3, NORMALS)   \
	I(Shininess, float,     SHININESS)       \
	I(Opacity,  float,     OPACITY)       \
	I(Displacement, glm::vec3, DISPLACEMENT) \
	I(Reflection, glm::vec3, REFLECTION)    \
	P(Albedo,   glm::vec3,   ALBEDO)      \
	P(AmbientOcclusion, float, AMBIENT_OCCLUSION) \
	P(Metallic,  float, METALLIC)         \
	P(Roughness, float, ROUGHNESS)


enum struct BuiltinMaterial : uint16_t {
#define ENUM_BUITINS(name, ...) k##name,
	NA_BUILTIN_TEXTURE_LIST(ENUM_BUITINS, ENUM_BUITINS, ENUM_BUITINS)
#undef ENUM_BUITINS
	kMapMask = 1 << 15,
#define ENUM_BUILTIN_MAPS(name, ...) k##name##Map = k##name | kMapMask,
#define DO_NOTHING(...)
	NA_BUILTIN_TEXTURE_LIST(ENUM_BUILTIN_MAPS, ENUM_BUILTIN_MAPS, ENUM_BUILTIN_MAPS)
#undef DO_NOTHING
#undef ENUM_BUILTIN_MAPS
};

struct BuiltinTextureCombo {
	renderer::MeshHandle hMesh;

#define DEF_MATERIAL_HANDLE(name, ...) renderer::MaterialHandle h##name;
#define DO_NOTHING(...)
	NA_BUILTIN_TEXTURE_LIST(DEF_MATERIAL_HANDLE, DEF_MATERIAL_HANDLE, DO_NOTHING);
	struct PBR {
		NA_BUILTIN_TEXTURE_LIST(DO_NOTHING, DO_NOTHING, DEF_MATERIAL_HANDLE);
	};
#undef DEF_MATERIAL_HANDLE
#undef DO_NOTHING
	PBR pbr;
	bool HasPBR() { return !pbr.hAlbedo.IsNil(); }
	glm::mat4 transform;
};

}


#endif // !_NABLA_CORE_ASSETS_BUILTINS_H_
