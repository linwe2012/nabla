#ifndef _NABLA_CORE_ASSET_TEXTURE_H_
#define _NABLA_CORE_ASSET_TEXTURE_H_
#include "utils.h"

#include <string>

#include "core/renderer/render-resources.h"
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

namespace nabla {
class TextureAsset {
public:
	bool operator<(const TextureAsset rhs) {
		return material_ < rhs.material_;
	}
	TextureAsset() : material_() {}
	TextureAsset(renderer::MaterialHandle md) : material_(md) {}

	/** @note that must be absolute path */
	static TextureAsset LoadTexture(const std::string& absolute_path, BuiltinMaterial);

	static TextureAsset MakeNil() { return TextureAsset(); }

	renderer::MaterialHandle GetMaterialHandle() {
		return material_;
	}
	renderer::MaterialHandle material_;
};

}

#endif // !_NABLA_CORE_ASSET_TEXTURE_H_
