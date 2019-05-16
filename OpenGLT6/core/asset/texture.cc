#include "texture.h"

#include "stb_image.h"

#include <map>

#include "filesystem.h"

namespace nabla {
std::map<std::string, TextureAsset> known;
using renderer::TextureFormat;

TextureAsset TextureAsset::LoadTexture(const std::string& absolute_path, BuiltinMaterial texture_type)
{
	std::string path = absolute_path;

	if (!fs::exists(path)) {
		NA_LOG_ERROR("Unable to load texture, File not exits: %s", path.c_str());
		return TextureAsset();
	}
	
	auto itr = known.find(path);
	if (itr != known.end()) {
		return itr->second;
	}

	int width, height, nrComponents;
	
	unsigned char * data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
	if (data == nullptr) {
		NA_LOG_ERROR("Unable to load texture: file %s", path.c_str());
		return TextureAsset();
	}

	TextureFormat format;
	if (nrComponents == 1)
		format = TextureFormat::kRed;
	else if (nrComponents == 3)
		format = renderer::TextureFormat::kRGB;
	else if (nrComponents == 4)
		format = renderer::TextureFormat::kRGBA;

	TextureAsset asset;
	asset.material_ = renderer::NewTexture(data, width, height, format);
	stbi_image_free(data);

	known[path] = asset;
	return asset;
}

}