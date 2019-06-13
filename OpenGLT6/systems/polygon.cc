#include "polygon.h"
#include "editor/gui.h"

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
using namespace renderer;

renderer::MeshHandle PopulateSphere(int axis_division) {
	// ...
	return MeshHandle();
}

renderer::MeshHandle PopulateCylinder(int axis_division) {
	// ...
	return MeshHandle();
}

renderer::MeshHandle PopulateCone(int axis_division) {
	// ...
	return MeshHandle();
}

renderer::MeshHandle PopulatePrism(int n) {
	// ...
	return MeshHandle();
}

renderer::MeshHandle PopulateFrustrum(int axis_division) {
	// ...
	return MeshHandle();
}

void PolygonSystem::OnGui([[maybe_unused]]const Vector<Entity>& actives) {
	bool flag_cube = ImGui::Button("New Cube");

	// ....


	if (flag_cube) {
		auto itr = objects_.find(UniqueObject{ "Cubic", 0 });
		if (itr == objects_.end()) {
			MeshHandle mesh = PopulateCube();
			objects_[UniqueObject{ "Cubic", 0 }] = Vector<MeshHandle>({ mesh });
			itr = objects_.find(UniqueObject{ "Cubic", 0 });
		}
		MeshHandle hmesh = itr->second[0];
		render->Add(em_->Create(), hmesh);
	}
}


}