#include "polygon.h"
#include "editor/gui.h"

namespace nabla {

renderer::MeshHandle PopulateCube(int dummy) {
	printf("hi cube");
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
constexpr float PI = 3.14159;

renderer::MeshHandle PopulateSphere(int axis_division) {
	constexpr float k = 0.8f;
	Vector<float> sphere;
	Vector<unsigned> sphere_row;
	Vector<unsigned> sphere_last_row;
	Vector<unsigned> indices;

	for (int i = 0; i < axis_division - 1; ++i) {
		float rad = (float)i / axis_division * PI * 2;
		float y = std::sin(rad);
		sphere_row.swap(sphere_last_row);
		sphere_row.clear();

		for (int j = 0; j < axis_division; ++j) {
			float plan_rad = (float)j / axis_division * PI * 2;
			float x = std::cos(plan_rad) * std::cos(rad);
			float z = std::sin(plan_rad) * std::cos(rad);
			sphere.multi_push_back(x, y, z);

			unsigned current = (sphere.size() - 1) / 3;
			sphere_row.push_back(current);

			if (sphere_last_row.size() == 0) {
				continue;
			}

			indices.multi_push_back(sphere_last_row[j], sphere_last_row[(j + 1) % axis_division], current);
			if (j != 0) {
				indices.multi_push_back(current, current - 1, sphere_last_row[j]);
			}
		}
	}

	{
		sphere.multi_push_back(0.0f, -1.0f, 0.0f);
		unsigned current = (sphere.size() - 1) / 3;
		for (int i = 0; i < axis_division; ++i) {
			indices.multi_push_back(current, sphere_row[i], sphere_row[(i + 1) % axis_division]);
		}
	}

	sphere_row.clear();
	sphere_last_row.clear();

	

	for (int i = 0; i < axis_division - 1; ++i) {
		float rad = (float)i / axis_division * PI * 2;
		float y = -std::sin(rad);
		sphere_row.swap(sphere_last_row);
		sphere_row.clear();

		for (int j = 0; j < axis_division; ++j) {
			float plan_rad = (float)j / axis_division * PI * 2;
			float x = std::cos(plan_rad) * std::cos(rad);
			float z = std::sin(plan_rad) * std::cos(rad);
			sphere.multi_push_back(x, y, z);

			unsigned current = (sphere.size() - 1) / 3;
			sphere_row.push_back(current);

			if (sphere_last_row.size() == 0) {
				continue;
			}

			indices.multi_push_back(sphere_last_row[j], sphere_last_row[(j + 1) % axis_division], current);
			if (j != 0) {
				indices.multi_push_back(current, sphere_row[j - 1], sphere_last_row[j]);
			}
		}
	}

	{
		sphere.multi_push_back(0.0f, -1.0f, 0.0f);
		unsigned current = (sphere.size() - 1) / 3;
		for (int i = 0; i < axis_division; ++i) {
			indices.multi_push_back(current, sphere_row[i], sphere_row[(i + 1) % axis_division]);
			
		}
	}
	
	size_t size = sphere.size();
	sphere.insert(sphere.end(), sphere.begin(), sphere.end());
	sphere.resize(size * 4 + size / 3 * 2, 0);

	using namespace nabla::renderer;
	Vector<LayoutInfo> layouts;
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(0, 0));
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(1, size * sizeof(float)));
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(2, size * 2 * sizeof(float)));
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(3, size * 3 * sizeof(float)));
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec2>(4, size * 4 * sizeof(float)));
	return NewMesh(MemoryInfo{ sphere.begin() , sphere.size() * sizeof(float) }, MemoryInfo{ indices.begin(), indices.size() * sizeof(unsigned) }, layouts);
}

/*
	*/


renderer::MeshHandle PopulateCylinder(int axis_division) {
	constexpr float k = 0.8f;
	float vertices[] = {
		// vertex
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, -1.0f,
		0.8f, 0.5f, -1.0f,
		0.5f, 0.8f, -1.0f,
		0.0f, 1.0f, -1.0f,
		-0.5f, 0.8f, -1.0f,
		-0.8f, 0.5f, -1.0f,
		-1.0f, 0.0f, -1.0f,
		-0.8f, -0.5f, -1.0f,
		-0.5f, -0.8f, -1.0f,
		0.0f, -1.0f, -1.0f,
		0.5f, -0.8f, -1.0f,
		0.8f, -0.5f, -1.0f,
		0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		0.8f, 0.5f, 1.0f,
		0.5f, 0.8f, 1.0f,
		0.0f, 1.0f, 1.0f,
		-0.5f, 0.8f, 1.0f,
		-0.8f, 0.5f, 1.0f,
		-1.0f, 0.0f, 1.0f,
		-0.8f, -0.5f, 1.0f,
		-0.5f, -0.8f, 1.0f,
		0.0f, -1.0f, 1.0f,
		0.5f, -0.8f, 1.0f,
		0.8f, -0.5f, 1.0f,

		// normal
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, -1.0f,
		0.8f, 0.5f, -1.0f,
		0.5f, 0.8f, -1.0f,
		0.0f, 1.0f, -1.0f,
		-0.5f, 0.8f, -1.0f,
		-0.8f, 0.5f, -1.0f,
		-1.0f, 0.0f, -1.0f,
		-0.8f, -0.5f, -1.0f,
		-0.5f, -0.8f, -1.0f,
		0.0f, -1.0f, -1.0f,
		0.5f, -0.8f, -1.0f,
		0.8f, -0.5f, -1.0f,
		0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		0.8f, 0.5f, 1.0f,
		0.5f, 0.8f, 1.0f,
		0.0f, 1.0f, 1.0f,
		-0.5f, 0.8f, 1.0f,
		-0.8f, 0.5f, 1.0f,
		-1.0f, 0.0f, 1.0f,
		-0.8f, -0.5f, 1.0f,
		-0.5f, -0.8f, 1.0f,
		0.0f, -1.0f, 1.0f,
		0.5f, -0.8f, 1.0f,
		0.8f, -0.5f, 1.0f,

		// tangent
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
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
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
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
		k * 0.0f, k * 1.0f,
		k * 1.0f,  k * 1.0f,
		k * 0.0f, k * 0.0f,
		k * 1.0f,  k * 0.0f,
		k * 0.0f, k * 1.0f,
		k * 1.0f,  k * 1.0f,
		k * 0.0f, k * 0.0f,
		k * 1.0f,  k * 0.0f,
		k * 0.0f, k * 1.0f,
		k * 1.0f,  k * 1.0f,
		k * 0.0f, k * 0.0f,
		k * 1.0f,  k * 1.0f,
		k * 0.0f, k * 0.0f,
		k * 1.0f, k * 0.0f,
		k * 0.0f, k * 1.0f,
		k * 1.0f, k * 1.0f,
		k * 0.0f, k * 0.0f,
		k * 1.0f, k * 0.0f,
		k * 0.0f, k * 1.0f,
		k * 1.0f, k * 1.0f,
		k * 0.0f, k * 0.0f,
		k * 1.0f, k * 0.0f,
		k * 0.0f, k * 1.0f,
		k * 1.0f, k * 1.0f,
		k * 0.0f, k * 0.0f,
	};

	unsigned indices[] = {
		0, 1, 2, 2, 3, 0,
		0, 3, 4, 4, 5, 0,
		0, 5, 6, 6, 7, 0,
		0, 7, 8, 8, 9, 0,
		0, 9, 10, 10, 11, 0,
		0, 11, 12, 12, 1, 0,
		13, 14, 15, 15, 16, 13,
		13, 16, 17, 17, 18, 13,
		13, 18, 19, 19, 20, 13,
		13, 20, 21, 21, 22, 13,
		13, 22, 23, 23, 24, 13,
		13, 24, 25, 25, 14, 13,
		1, 14, 2, 2, 14, 15,
		15, 2, 3, 3, 15, 16,
		16, 3, 4, 4, 16, 17,
		17, 4, 5, 5, 17, 18,
		18, 5, 6, 6, 18, 19,
		19, 6, 7, 7, 19, 20,
		20, 7, 8, 8, 20, 21,
		21, 8, 9, 9, 21, 22,
		22, 9, 10, 10, 22, 23,
		23, 10, 11, 11, 23, 24,
		24, 11, 12, 12, 24, 25,
		25, 12, 1, 1, 25, 14,
	};
	using namespace nabla::renderer;
	Vector<LayoutInfo> layouts;
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(0, 0));
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(1, 78 * sizeof(float)));
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(2, 156 * sizeof(float)));
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(3, 234 * sizeof(float)));
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec2>(4, 312 * sizeof(float)));
	return NewMesh(MemoryInfo{ vertices , sizeof(vertices) }, MemoryInfo{ indices, sizeof(indices) }, layouts);
}



renderer::MeshHandle PopulateCone(int axis_division) {
	constexpr float k = 0.8f;
	float vertices[] = {
		// vertex
		0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, -1.0f,
		0.8f, 0.5f, -1.0f,
		0.5f, 0.8f, -1.0f,
		0.0f, 1.0f, -1.0f,
		-0.5f, 0.8f, -1.0f,
		-0.8f, 0.5f, -1.0f,
		-1.0f, 0.0f, -1.0f,
		-0.8f, -0.5f, -1.0f,
		-0.5f, -0.8f, -1.0f,
		0.0f, -1.0f, -1.0f,
		0.5f, -0.8f, -1.0f,
		0.8f, -0.5f, -1.0f,
		0.0f, 0.0f, -1.0f,

		// normal
		0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, -1.0f,
		0.8f, 0.5f, -1.0f,
		0.5f, 0.8f, -1.0f,
		0.0f, 1.0f, -1.0f,
		-0.5f, 0.8f, -1.0f,
		-0.8f, 0.5f, -1.0f,
		-1.0f, 0.0f, -1.0f,
		-0.8f, -0.5f, -1.0f,
		-0.5f, -0.8f, -1.0f,
		0.0f, -1.0f, -1.0f,
		0.5f, -0.8f, -1.0f,
		0.8f, -0.5f, -1.0f,
		0.0f, 0.0f, -1.0f,

		// tangent
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
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
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,

		// textcoord
		k * 1.0f,  k * 0.0f,
		k * 0.0f, k * 1.0f,
		k * 1.0f,  k * 1.0f,
		k * 0.0f, k * 0.0f,
		k * 1.0f,  k * 0.0f,
		k * 0.0f, k * 1.0f,
		k * 1.0f,  k * 1.0f,
		k * 0.0f, k * 0.0f,
		k * 1.0f,  k * 0.0f,
		k * 0.0f, k * 1.0f,
		k * 1.0f,  k * 1.0f,
		k * 0.0f, k * 0.0f,
		k * 1.0f,  k * 1.0f,
		k * 0.0f, k * 0.0f,
	};

	unsigned indices[] = {
		0, 1, 2, 2, 3, 0,
		0, 3, 4, 4, 5, 0,
		0, 5, 6, 6, 7, 0,
		0, 7, 8, 8, 9, 0,
		0, 9, 10, 10, 11, 0,
		0, 11, 12, 12, 1, 0,
		13, 1, 2, 2, 3, 13,
		13, 3, 4, 4, 5, 13,
		13, 5, 6, 6, 7, 13,
		13, 7, 8, 8, 9, 13,
		13, 9, 10, 10, 11, 13,
		13, 11, 12, 12, 1, 13,
	};
	using namespace nabla::renderer;
	Vector<LayoutInfo> layouts;
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(0, 0));
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(1, 42 * sizeof(float)));
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(2, 84 * sizeof(float)));
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(3, 126 * sizeof(float)));
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec2>(4, 168 * sizeof(float)));
	return NewMesh(MemoryInfo{ vertices , sizeof(vertices) }, MemoryInfo{ indices, sizeof(indices) }, layouts);
}

renderer::MeshHandle PopulatePrism(int n) {
	constexpr float k = 0.8f;
	float vertices[] = {
		// vertex
		0.0f, 0.0f, 0.6f,
		0.0f, 1.1f, 0.2f,
		0.5f, -0.6f, 0.2f,
		-0.5f, -0.6f, 0.2f,

		// normal
		0.0f, 0.0f, 0.6f,
		0.0f, 1.1f, 0.2f,
		0.5f, -0.6f, 0.2f,
		-0.5f, -0.6f, 0.2f,

		// tangent
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,

		// bitangent
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,

		// textcoord
		k * 1.0f,  k * 0.0f,
		k * 0.0f, k * 1.0f,
		k * 1.0f,  k * 1.0f,
		k * 0.0f, k * 0.0f,
	};

	unsigned indices[] = {
		0, 1, 2, 2, 3, 0,
		0, 1, 3, 3, 1, 2,
	};
	using namespace nabla::renderer;
	Vector<LayoutInfo> layouts;
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(0, 0));
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(1, 12 * sizeof(float)));
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(2, 24 * sizeof(float)));
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(3, 36 * sizeof(float)));
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec2>(4, 48 * sizeof(float)));
	return NewMesh(MemoryInfo{ vertices , sizeof(vertices) }, MemoryInfo{ indices, sizeof(indices) }, layouts);
}

renderer::MeshHandle PopulateFrustum(int axis_division) {
	constexpr float k = 0.8f;
	float vertices[] = {
		// vertex
		-2.0f, -2.0f, 0.0f,
		2.0f, -2.0f, 0.0f,
		2.0f, 2.0f, 0.0f,
		-2.0f, 2.0f, 0.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,

		// normal
		-2.0f, -2.0f, 0.0f,
		2.0f, -2.0f, 0.0f,
		2.0f, 2.0f, 0.0f,
		-2.0f, 2.0f, 0.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,

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
		k * 0.0f, k * 1.0f,
		k * 1.0f,  k * 1.0f,
		k * 0.0f, k * 0.0f,
		k * 0.25f,  k * 0.75f,
		k * 0.25f, k * 0.25f,
		k * 0.75f,  k * 0.75f,
		k * 0.75f, k * 0.25f,
	};
	unsigned indices[] = {
		0, 1, 2, 2, 3, 0,
		0, 1, 5, 5, 4, 0,
		0, 3, 4, 4, 3, 7,
		7, 3, 6, 6, 3, 2,
		2, 1, 5, 5, 2, 6,
		6, 5, 4, 4, 6, 7,
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


#define ELEMENT_LIST(V)    \
V(Cube, "")			       \
V(Sphere, "Axis")		   \
V(Cylinder, "Axis")		   \
V(Cone, "Axis")			   \
V(Prism, "Axis")	       \
V(Frustum, "Axis")		   

#define FLAG(t, d) bool flag_##t = ImGui::Button("New " #t); ImGui::SameLine(); ImGui::PushID(#t); ImGui::DragInt("Num " d, &data_->num_##t); ImGui::PopID();
#define DEF_DATA_ELEM(t, ...) int num_##t = 0;

struct PolygonSystem::Data {
	ELEMENT_LIST(DEF_DATA_ELEM)
};

void PolygonSystem::Initialize(SystemContext& ctx)
{
	em_ = ctx.entity_manager;
	render = ctx.render;
	data_ = new Data;
}

void PolygonSystem::OnGui([[maybe_unused]]const Vector<Entity>& actives) {
	if (!ImGui::CollapsingHeader("Primitives")) {
		return;
	}

	ELEMENT_LIST(FLAG);

#define HANDLE_MESH(t, ...)                                                              \
	if (flag_##t) {                                                                      \
		auto itr = objects_.find(#t);                                               \
		if (itr == objects_.end()) {                          \
			MeshHandle mesh = Populate##t(data_->num_##t);                             \
			objects_[#t] = Vector<MeshHandle>({ mesh }); \
			itr = objects_.find(#t);                     \
		}                                                                                \
		MeshHandle hmesh = itr->second[0];                                               \
		render->Add(em_->Create(), hmesh);                                               \
	}

	ELEMENT_LIST(HANDLE_MESH)
}

#undef DEF_DATA_ELEM
#undef FLAG
#undef HANDLE_MESH

}