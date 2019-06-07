#ifndef _NABLA_CORE_ASSET_TEXTURE_H_
#define _NABLA_CORE_ASSET_TEXTURE_H_
#include "utils.h"

#include <string>

#include "core/renderer/render-resources.h"
#include "builtins.h"

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
