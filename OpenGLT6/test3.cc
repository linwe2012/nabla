#if 0
#include <iostream>
#include <regex>
#include <thread>



#include "core/entity-manager.h"
#include "core/renderer.h"
#include "core/asset/bootstrap.h"
#include "core/camera.h"

#include "GLFW/glfw3.h"
#include "editor/gui.h"

#include "systems/animation.h"
#include "systems/lighting.h"
#include "systems/renderable.h"
#include "systems/material.h"

// extern "C" {
// 	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
// }

nabla::renderer::MeshHandle GetCubeMesh() {
	float vertices[] = {
		// back face
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
		 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
		 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
		-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
		// front face
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
		 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
		 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
		// left face
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
		-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		// right face
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
		// bottom face
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
		 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		// top face
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
		 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
	};
	using namespace nabla;
	using namespace nabla::renderer;
	Vector<LayoutInfo> layouts;
	layouts.push_back(LayoutInfo{
		0, 3, LayoutInfo::kFloat, false, 8 * sizeof(float), 0
		});
	layouts.push_back(LayoutInfo{
		1, 3, LayoutInfo::kFloat, false, 8 * sizeof(float), 3 * sizeof(float)
		});

	layouts.push_back(LayoutInfo{
		2, 2, LayoutInfo::kFloat, false, 8 * sizeof(float), 6 * sizeof(float)
		});

	return NewMesh(MemoryInfo{ vertices , sizeof(vertices) }, MemoryInfo{ nullptr, 0 }, layouts);
}


nabla::renderer::MeshHandle GetCubeIndexedMesh() {
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
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,

		// 1.0f, 1.0f, 1.0f,
		// 0.0f, 0.0f, 1.0f,
		// 0.0f, 0.0f, 1.0f,
		// 0.0f, 0.0f, 1.0f,
		//  0.0f, 0.0f, -1.0f,
		//  0.0f, 0.0f, -1.0f,
		//  0.0f, 0.0f, -1.0f,
		//  0.0f, 0.0f, -1.0f,

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
		 k * 0.0f, k *  0.0f,
		 k * 0.0f, k *  1.0f,

		  // k * 1.0f,  k * 1.0f,
		  // k * 1.0f,  k * -1.0f,
		  //
		  // k * -1.0f, k * 1.0f,
		  // k * -1.0f, k * -1.0f,

		//   k *  0.5f,  k * 0.5f,
		//   k * 0.5f,   k * -0.5f,
		//   k * -0.5f,  k * -0.5f,
		//   k * -0.5f,  k * 0.5f,

		// k * 1.0f,  k * 1.0f,
		//   k * 1.0f,  k * -1.0f,
		//
		//   k * -1.0f, k * 1.0f,
		//   k * -1.0f, k * -1.0f,
		//
		// k * 1.0f,  k * -1.0f,
		// k * 1.0f,  k * 1.0f,
		// k * -1.0f, k *  -1.0f,
		// k * -1.0f, k *  1.0f,
	};

	unsigned indices[] = {
		0, 1, 2, 2, 3, 0, 
		6, 7, 4, 4, 5, 6,

		1, 5, 4, 4, 0, 1,
		0, 4, 7, 7, 3, 0,
		2, 3, 7, 7, 6, 2,
		2, 6, 5, 5, 1, 2,


	};
	using namespace nabla;
	using namespace nabla::renderer;
	Vector<LayoutInfo> layouts;
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(0, 0));
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(1, 24 * sizeof(float)));
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(2, 48 * sizeof(float)));
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec3>(3, 72 * sizeof(float)));
	layouts.push_back(LayoutInfo::CreatePacked<glm::vec2>(4, 96 * sizeof(float)));

	return NewMesh(MemoryInfo{ vertices , sizeof(vertices) }, MemoryInfo{ indices, sizeof(indices) }, layouts);
}

nabla::renderer::MaterialHandle GenChessBoardTexture() {
	nabla::Vector<unsigned char> colors;
	int w = 1600;
	int h = 1600;
	int cnt = 2;

	colors.reserve(w * h * 3);
	for (int i = 0; i < w; ++i) {
		int inc = ((i % 64) < 32) * 32;
		for (int j = 0; j < h * 2; ++j) {

			if ((inc + j) % 64 <  32) {
				colors.push_back(255);
				colors.push_back(0);
				colors.push_back(0);
			}
			else {
				colors.push_back(0);
				colors.push_back(0);
				colors.push_back(0);
				
			}
		}
	}
	return nabla::renderer::NewTexture(&colors[0], w, h, nabla::renderer::TextureFormat::kRGB);
}

int main()
{
	using namespace nabla;

	int SCR_WIDTH = 1200, SCR_HEIGHT = 600;
	{
		renderer::InitConfig cfg;
		cfg.width = SCR_WIDTH;
		cfg.height = SCR_HEIGHT;
		renderer::Init(cfg);
	}
	GlobalServices.launch<ComponentRegistry>("ComponentRegistry");

	EntityManager entity_manager;

	LightingSystem sys_lighting;
	AnimationSystem sys_animation;
	RenderableSystem sys_renderable;
	MatrialSysterm sys_material;

	

	sys_lighting.Initilize();
	sys_renderable.Initilize();
	sys_material.Initilize();

	sys_renderable.AttachBeforeRender(&sys_material);

	Vector<Entity> lights;

	AssetManager assets;
	assets.ParseAssetsFromFile("./test/assets.yml");
	auto teapot = assets.LoadModelToGPU("teapot", false);
	// auto teapot = assets.LoadModelToGPU("castle", false);
	auto eteapot = entity_manager.Create();
	Set<std::string> macros;
	macros.insert("NormalMap");
	macros.insert("Bitangent");
	macros.insert("Tangent");
	macros.insert("TexCoords");
	macros.insert("DiffuseMap");
	macros.insert("Specular");

	auto geopass = renderer::NewShader(renderer::ShaderFilePath{ "test/ubershaders/gbuffer.vs", "test/ubershaders/gbuffer.fs" }, macros);
	sys_material.SetUpMaterialHandles(geopass, MatrialSysterm::Uniforms{});

	auto hdiffuse = renderer::NewUniform(geopass, "DiffuseMap", renderer::MaterialType::kInt);
	// auto hdiffuse = renderer::NewUniform(geopass, "Diffuse", renderer::MaterialType::kVec3);
	auto hspecular = renderer::NewUniform(geopass, "Specular", renderer::MaterialType::kFloat);
	auto hmodel = renderer::NewUniform(geopass, "model", renderer::MaterialType::kMat4);
	auto hview = renderer::NewUniform(geopass, "view", renderer::MaterialType::kMat4);
	auto hproject = renderer::NewUniform(geopass, "projection", renderer::MaterialType::kMat4);
	auto halbedo = renderer::NewUniform(geopass, "Albedo", renderer::MaterialType::kVec3);
	auto hmetal = renderer::NewUniform(geopass, "Metallic", renderer::MaterialType::kFloat);
	auto hrough = renderer::NewUniform(geopass, "Roughness", renderer::MaterialType::kFloat);
	auto hao = renderer::NewUniform(geopass, "AO", renderer::MaterialType::kFloat);

	renderer::OpenHandle(geopass).Use();
	renderer::OpenHandle(geopass).SetInt("DiffuseMap", 0);
	// renderer::OpenHandle()


	auto postprocess = renderer::NewShader(renderer::ShaderFilePath{ "test/ubershaders/deferred-light-box.vs", "test/ubershaders/deferred-light-box.fs" }, macros);
	auto hbox_proj = renderer::NewUniform(postprocess, "projection", renderer::MaterialType::kMat4);
	auto hbox_view = renderer::NewUniform(postprocess, "view", renderer::MaterialType::kMat4);
	auto hbox_model = renderer::NewUniform(postprocess, "model", renderer::MaterialType::kMat4);
	auto hbox_lightColor = renderer::NewUniform(postprocess, "lightColor", renderer::MaterialType::kVec3);

	auto hcube = GetCubeMesh();

	auto lightingpass = renderer::NewShader(renderer::ShaderFilePath{ "test/ubershaders/deferred-shading.vs", "test/ubershaders/deferred-shading.fs" }, macros);
	sys_lighting.SetShader(lightingpass, postprocess);

	sys_renderable.SetRenderPassShader(renderer::RenderPass::kPostProc, postprocess, hbox_model);
	sys_renderable.SetRenderPassShader(renderer::RenderPass::kForward, geopass, hmodel);

	for (int i = 0; i < 4; ++i) {
		lights.push_back(entity_manager.Create());
		sys_lighting.NewLight(lights.back(), LightingSystem::Light{
		LightingSystem::Light::kPoint, hcube
			});
	}

	lights.push_back(entity_manager.Create());
	sys_lighting.NewLight(lights.back(), LightingSystem::Light{
		LightingSystem::Light::kSpot, hcube
		});

	glEnable(GL_DEPTH_TEST);
	Camera camera;

	using renderer::OpenHandle;
	float last_frame = glfwGetTime();
	bool first_move = true;
	float last_x = 0.0f, last_y = 0.0f;

	InitGui();
	std::string textipt;


	Clock clock;

	renderer::detail::PrepareRenderContext__Temp();
	{
		using namespace renderer;
		using std::make_pair;
		Vector<std::pair<TextureFormat, const char*>> textures{
			make_pair(TextureFormat::kRGB, "gPosition"),
			make_pair(TextureFormat::kRGB, "gNormal"),
			make_pair(TextureFormat::kRGBA, "gDiffuseSpec"),
			make_pair(TextureFormat::kRGB, "gAlbedo"),
			make_pair(TextureFormat::kRGB, "gMetaRoughAO")
		};
		renderer::SetDefaultGBuffer(renderer::NewGBuffer(
			SCR_WIDTH, SCR_HEIGHT, lightingpass, textures
		));
	}
	
	Vector<Entity> solids{
		// eteapot
	};

	// for (auto& m : teapot.meshes_) {
	// 	auto e = entity_manager.Create();
	// 	solids.push_back(e);
	// 	sys_renderable.Add(e, m.hMesh);
	// 	sys_material.Add(e);
	// }
	// sys_renderable.Add(eteapot, teapot.meshes_[0].hMesh);
	// sys_material.Add(eteapot);
	
	// solids.push_back(entity_manager.Create()); // desktop
	// sys_renderable.Add(solids.back(), hcube, Transform{
	// 	glm::vec3(0.0f, -0.8f, 0.0f),
	// 	glm::vec3(1.0f, 0.2f, 1.0f),
	// 	glm::quat()
	// 	});
	// sys_material.Add(solids.back());
	// sys_material.GetEdit(solids.back()).diffuse = glm::vec3(1.0f, 0.0f, 0.0f);

	float pos[] = {
		1.0f, 1.0f,
		-1.0f, 1.0f,
		-1.0f, -1.0f,
		1.0f, -1.0f
	};

	// float colors[] = {
	// 	0.0f, 1.0f, 0.0f,
	// 	1.0f, 1.0f, 0.0f,
	// 	0.0f, 1.0f, 1.0f,
	// 	0.0f, 0.0f, 1.0f,
	// };
	// 
	// 
	// for (int i = 0; i < 4; ++i) {
	// 	solids.push_back(entity_manager.Create()); // legs
	// 	sys_renderable.Add(solids.back(), hcube, Transform{
	// 		glm::vec3(pos[i*2] *5.0f, -1.0f, pos[i * 2 + 1] * 5.0f),
	// 		glm::vec3(0.12f, 0.8f, 0.12f),
	// 		glm::quat()
	// 		});
	// 	sys_material.Add(solids.back());
	// 	sys_material.GetEdit(solids.back()).diffuse = glm::vec3(colors[i * 3], colors[i * 3 + 1], colors[i * 3 + 2]);
	// }

	auto hMonet = AssetManager::LoadTexture("test/img/Monet.bmp");
	auto hCrack = AssetManager::LoadTexture("test/img/Crack.bmp");
	auto hChess = GenChessBoardTexture();
	auto hBlend = AssetManager::LoadTexture("test/img/Crack.bmp", "test/img/Spot.bmp");

	auto hindexedcube = GetCubeIndexedMesh();
	
	bool usewhat = 0;
	bool usewhat2 = 0;
	
	while (renderer::IsAlive())
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);
		using namespace renderer;
		

		
		UseShader(geopass);
		model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
		SetUniform(hproject, projection);
		SetUniform(hview, view);
		SetUniform(hmodel, model);

		// OpenHandle(geopass).Use();
		// OpenHandle(geopass).SetMat4("projection", projection);
		// OpenHandle(geopass).SetMat4("view", view);
		// OpenHandle(geopass).SetMat4("model", model);
		// 
		// OpenHandle(geopass).SetFloat("Specular", view);
		// OpenHandle(geopass).SetMat4("model", model);
		// 
		// glActiveTexture(GL_TEXTURE0);
		// glBindTexture(GL_TEXTURE_2D, OpenHandle(hMonet));
		if (usewhat == 0) {
			UseTexture(hMonet);
		}
		else {
			UseTexture(hChess);
		}
		
		// auto halbedo = renderer::NewUniform(geopass, "Albedo", renderer::MaterialType::kVec3);
		// auto hmetal = renderer::NewUniform(geopass, "Metallic", renderer::MaterialType::kFloat);
		// auto hrough = renderer::NewUniform(geopass, "Roughness", renderer::MaterialType::kFloat);
		// auto hao = renderer::NewUniform(geopass, "AO", renderer::MaterialType::kFloat);

		SetUniform(hspecular, 0.5f);
		SetUniform(halbedo, glm::vec3(0.5f));
		SetUniform(hmetal, 1.0f);
		SetUniform(hrough, 0.3f);
		SetUniform(hao, 0.2f);

		// SetUniform(hdiffuse, diffuse);
		// SetUniform(hspecular, specular);
		// SetUniform(halbedo, albedo);
		// SetUniform(hmetal, metallic);
		// SetUniform(hrough, rough);
		// SetUniform(hao, ao);
		
		
		DrawMesh(teapot.meshes_[0].hMesh);

		if (usewhat2 == 0) {
			UseTexture(hCrack);
		}
		else {
			UseTexture(hBlend);
		}
		
		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3{ 1.0f, 0.2f, 1.0f });
		model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
		SetUniform(hmodel, model);
		DrawMesh(hindexedcube);

		sys_renderable.Update(clock);

		sys_lighting.SetEyePos(camera.Position, projection, view);
		sys_lighting.Update(clock);
		sys_material.Update(clock);
		

		renderer::FlushAllDrawCalls();
		renderer::GetRenderContext()->Reset(renderer::GetRenderContext()->resources.buffer, renderer::GetRenderContext()->resources.end_of_storage - renderer::GetRenderContext()->resources.buffer);

		PrepareGuiFrame();
		ImGui::ShowDemoWindow();

		GLFWwindow* window = static_cast<GLFWwindow*>(renderer::GetWindow());
		if (!IsUserWorkingOnGui()) {
			int left_button = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

			if (left_button == GLFW_PRESS) {
				double xpos, ypos;
				glfwGetCursorPos(window, &xpos, &ypos);
				if (first_move != 0) {
					float xoffset = static_cast<float>(xpos - last_x);
					float yoffset = static_cast<float>(last_y - ypos);
					camera.ProcessMouseMovement(xoffset, yoffset);
				}
				last_x = static_cast<float>(xpos);
				last_y = static_cast<float>(ypos);
				first_move = 1;

			}
			else
				first_move = 0;
		}


		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera.ProcessKeyboard(FORWARD, clock.GetLastFrameDurationFloat());
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera.ProcessKeyboard(BACKWARD, clock.GetLastFrameDurationFloat());
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.ProcessKeyboard(LEFT, clock.GetLastFrameDurationFloat());
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera.ProcessKeyboard(RIGHT, clock.GetLastFrameDurationFloat());
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
			camera.ProcessKeyboard(UP, clock.GetLastFrameDurationFloat());
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
			camera.ProcessKeyboard(DOWN, clock.GetLastFrameDurationFloat());
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			camera.BudgeCamera(0.0f, 0.5f);

		
		camera.PassDeltaTime(clock.GetLastFrameDurationFloat());

		{

			ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_MenuBar);

			if (ImGui::BeginMenuBar()) {
			
				if (ImGui::BeginMenu("Options"))
				{
					ImGui::MenuItem("(dummy menu)", NULL, false, false);
					if(ImGui::BeginMenu("Camara")) {
						ImGui::DragFloat("Move Speed", &camera.MovementSpeed, 0.01f);
						ImGui::DragFloat("Mouse Sensitivity", &camera.MouseSensitivity, 0.001f);
						ImGui::EndMenu();
					}
					ImGui::EndMenu();
				}
			
				ImGui::EndMenuBar();
			}

			if (ImGui::CollapsingHeader(sys_lighting.name())) {
				sys_lighting.OnGui(lights);
			}

			ImGui::Checkbox("chess", &usewhat);
			ImGui::Checkbox("blend", &usewhat2);

			int cnt = 0;
			Vector<Entity> tmp;
			for (auto solid : solids) {
				tmp.clear();
				tmp.push_back(solid);

				if (ImGui::CollapsingHeader(std::to_string(cnt).c_str())) {
					
					if (ImGui::TreeNode("Material")) {
						sys_material.OnGui(tmp);
						ImGui::TreePop();
						ImGui::Separator();
					}

					if (ImGui::TreeNode("Transform")) {
						sys_renderable.OnGui(tmp);
						ImGui::TreePop();
						ImGui::Separator();
					}
				}
				++cnt;
			}
			
			
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", clock.GetLastFrameDuration() * 1000.0f, clock.GetLastFrameFps());
			ImGui::End();
		}

		EndGuiFrame();

		sys_animation.update(0.2f);

		std::this_thread::sleep_until(clock.GetCurrentTimeRaw() + std::chrono::microseconds(16));
		clock.NextFrame();
	}
	return 0;
}

#endif
