#include "polygon.h"


namespace nabla {

renderer::MeshHandle PopulateCube() {
	constexpr float k = 0.8f;
	float vertices[] = {
		// vertex
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,

		// normal
			1.0f,  1.0f,  1.0f,
			1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
			1.0f,  1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		// tangent
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,

		// bitangent
			0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f,

			// textcoord
		k * 1.0f,  k * 0.0f,
		k * 1.0f,  k * 0.0f,

		k * 0.0f, k * 1.0f,
		k * 0.0f, k * 0.0f,

		k * 1.0f,  k * 1.0f,
		k * 1.0f,  k * 1.0f,
		k * 0.0f, k * 0.0f,
		k * 0.0f, k * 1.0f,
	};
	unsigned indices[] = {
		0, 1, 2, 2, 3, 0,
		6, 7, 4, 4, 5, 6,

		1, 5, 4, 4, 0, 1,
		0, 4, 7, 7, 3, 0,
		2, 3, 7, 7, 6, 2,
		2, 6, 5, 5, 1, 2,
	};
	using namespace nabla::renderer;
	Vector<LayoutInfo> layouts;
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(0, 0));
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(1, 24 * sizeof(float)));
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(2, 48 * sizeof(float)));
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(3, 72 * sizeof(float)));
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec2>(4, 96 * sizeof(float)));
	return NewMesh(MemoryInfo{ vertices , sizeof(vertices) }, MemoryInfo{ indices, sizeof(indices) }, layouts);
}

renderer::MeshHandle PopulateSphere(int axis_division) {
	// ...
}

renderer::MeshHandle PopulateCylinder(int axis_division) {
	// ...
}

renderer::MeshHandle PopulateCone(int axis_division) {
	// ...
}

renderer::MeshHandle PopulatePrism(int n) {
	// ...
}

renderer::MeshHandle PopulateFrustrum(int axis_division) {
	// ...
}

}