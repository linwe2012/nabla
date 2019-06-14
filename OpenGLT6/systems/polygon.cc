#include "polygon.h"
#include "editor/gui.h"

namespace nabla {

renderer::MeshHandle PopulateCube(int dummy) {
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
	float R = 0.7f;//球的半径
	int stack = 20;//切片，把球体横向切成20部分
	float stackstep = (float)(PI / stack);//单位角度值
	int slice = 50;//把球体纵向切成50部分
	float slicestep = (float)(PI / slice);//水平圆递增的角度值
	float r0, r1, x0, x1, y0, y1, z0, z1;//以r0,r1为圆心
	float alpha0 = 0, alpha1 = 0;//前后两个角度
	float beta = 0;//切片平面上的角度
	Vector<float> coordslist; coordslist.resize(12000); //float coordslist[12000];
	Vector<float> vertices; vertices.resize(56000);     //float vertices[56000];

	int k = 0;
	for (int i = 0; i < stack; i++)
	{
		alpha0 = (float)(-PI / 2 + (i * stackstep));
		alpha1 = (float)(-PI / 2 + ((i + 1) * stackstep));
		y0 = (float)(R * sin(alpha0));
		r0 = (float)(R * cos(alpha0));
		y1 = (float)(R * sin(alpha1));
		r1 = (float)(R * cos(alpha1));

		for (int j = 0; j <= 100; j++)
		{
			beta = j * slicestep;
			x0 = (float)(r0 * cos(beta));
			z0 = -(float)(r0 * sin(beta));
			x1 = (float)(r1 * cos(beta));
			z1 = -(float)(r1 * sin(beta));
			coordslist[k++] = x0;
			coordslist[k++] = y0;
			coordslist[k++] = z0;
			coordslist[k++] = x1;
			coordslist[k++] = y1;
			coordslist[k++] = z1;
		}
	}

	for (int i = 0; i < 12000; i++)
	{
		vertices[i] = coordslist[i];
		vertices[i + 12000] = coordslist[i];
	}
	for (int i = 24000; i < 48000; i++)
	{
		vertices[i] = 0.0f;
	}
	for (int i = 0; i < 8000; i++)
	{
		vertices[i + 48000] = i * 1.0 / 8000;
	}

	return MeshHandle();
}

renderer::MeshHandle PopulateCylinder(int axis_division) {
	float R = 0.7f;//圆柱体的半径
	int stack = 20;//切片，把圆柱体体横向切成20部分
	float stackstep = (float)(PI / stack);//单位角度值
	int slice = 50;//把圆柱体纵向切成50部分
	float slicestep = (float)(PI / slice);//水平圆递增的角度值
	float x0, x1, y0, y1, z0, z1;//以r0,r1为圆心
	float alpha0 = 0, alpha1 = 0;//前后两个角度
	float beta = 0;//切片平面上的角度
	Vector<float> coordslist; coordslist.resize(12000); //float coordslist[12000];
	Vector<float> vertices; vertices.resize(56000);     //float vertices[56000];

	int k = 0;
	for (int i = 0; i < stack; i++)
	{
		alpha0 = (float)(-PI / 2 + (i * stackstep));
		alpha1 = (float)(-PI / 2 + ((i + 1) * stackstep));
		if (i < stack / 2)
		{
			y0 = -(float)(R * 1.0 / stack);
			y1 = -(float)(R * 1.0 / stack);
		}
		else
		{
			y0 = (float)(R * 1.0 / stack);
			y1 = (float)(R * 1.0 / stack);
		}

		for (int j = 0; j <= 100; j++)
		{
			beta = j * slicestep;
			x0 = (float)(R * cos(beta));
			z0 = -(float)(R * sin(beta));
			x1 = (float)(R * cos(beta));
			z1 = -(float)(R * sin(beta));
			coordslist[k++] = x0;
			coordslist[k++] = y0;
			coordslist[k++] = z0;
			coordslist[k++] = x1;
			coordslist[k++] = y1;
			coordslist[k++] = z1;
		}
	}

	for (int i = 0; i < 12000; i++)
	{
		vertices[i] = coordslist[i];
		vertices[i + 12000] = coordslist[i];
	}
	for (int i = 24000; i < 48000; i++)
	{
		vertices[i] = 0.0f;
	}
	for (int i = 0; i < 8000; i++)
	{
		vertices[i + 48000] = i * 1.0 / 8000;
	}
	return MeshHandle();
}

renderer::MeshHandle PopulateCone(int axis_division) {
	// ...
	return MeshHandle();
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
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		2.0f, 2.0f, 0.0f,
		2.0f, -2.0f, 0.0f,
		-2.0f, -2.0f, 0.0f,
		-2.0f, 2.0f, 0.0f,

		// normal
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		2.0f, 2.0f, 0.0f,
		2.0f, -2.0f, 0.0f,
		-2.0f, -2.0f, 0.0f,
		-2.0f, 2.0f, 0.0f,

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
	return MeshHandle();
}
#define ELEMENT_LIST(V)    \
V(Cube, "")			       \
V(Sphere, "Axis")		   \
V(Cylinder, "Axis")		   \
V(Cone, "Axis")			   \
V(Prism, "Axis")	       \
V(Frustrum, "Axis")		   

#define FLAG(t, d) bool flag_##t = ImGui::Button("New " #t); ImGui::SameLine(); ImGui::DragInt("Num " d, &data_->num_##t);
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
		auto itr = objects_.find(UniqueObject{ #t, data_->num_##t });                    \
		if (itr == objects_.end()) {                                                     \
			MeshHandle mesh = PopulateCube(data_->num_Cube);                             \
			objects_[UniqueObject{ #t, data_->num_##t }] = Vector<MeshHandle>({ mesh }); \
			itr = objects_.find(UniqueObject{ #t, data_->num_##t });                     \
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