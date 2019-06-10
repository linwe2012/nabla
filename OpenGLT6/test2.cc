#if 1
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
#include <glm/gtx/matrix_decompose.hpp>
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

void renderQuad();
void renderCube();
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
	//auto teapot = assets.LoadModelToGPU("teapot", false);
	auto teapot = assets.LoadModelToGPU("castle", false);
	auto eteapot = entity_manager.Create();
	Set<std::string> macros;
	macros.insert("NormalMap");
	macros.insert("Bitangent");
	macros.insert("Tangent");
	macros.insert("TexCoords");
	macros.insert("Diffuse");
	macros.insert("Specular");

	auto geopass = renderer::NewShader(renderer::ShaderFilePath{ "test/ubershaders/gbuffer.vs", "test/ubershaders/gbuffer.fs" }, macros);
	sys_material.SetUpMaterialHandles(geopass, MatrialSysterm::Uniforms{});

	auto hdiffuse = renderer::NewUniform(geopass, "Diffuse", renderer::MaterialType::kVec3);
	auto hspecular = renderer::NewUniform(geopass, "Specular", renderer::MaterialType::kFloat);
	auto hmodel = renderer::NewUniform(geopass, "model", renderer::MaterialType::kMat4);
	auto hview = renderer::NewUniform(geopass, "view", renderer::MaterialType::kMat4);
	auto hproject = renderer::NewUniform(geopass, "projection", renderer::MaterialType::kMat4);
	auto halbedo = renderer::NewUniform(geopass, "Albedo", renderer::MaterialType::kVec3);
	auto hmetal = renderer::NewUniform(geopass, "Metallic", renderer::MaterialType::kFloat);
	auto hrough = renderer::NewUniform(geopass, "Roughness", renderer::MaterialType::kFloat);
	auto hao = renderer::NewUniform(geopass, "AO", renderer::MaterialType::kFloat);



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

	for (int i = 0; i < 16; ++i) {
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
		eteapot
	};

	for (auto& m : teapot.meshes_) {
		auto e = entity_manager.Create();
		solids.push_back(e);
		RigidBody r;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(m.transform, r.scale, r.quaternion, r.position, skew, perspective);
		sys_renderable.Add(e, m.hMesh, r);
		sys_material.Add(e);
	}
	// sys_renderable.Add(eteapot, teapot.meshes_[0].hMesh);
	// sys_material.Add(eteapot);
	
	solids.push_back(entity_manager.Create()); // desktop
	sys_renderable.Add(solids.back(), hcube, RigidBody{
		glm::vec3(0.0f, -0.8f, 0.0f),
		glm::vec3(1.0f, 0.2f, 1.0f),
		glm::quat()
		});
	sys_material.Add(solids.back());
	// sys_material.GetEdit(solids.back()).diffuse = glm::vec3(1.0f, 0.0f, 0.0f);

	float pos[] = {
		1.0f, 1.0f,
		-1.0f, 1.0f,
		-1.0f, -1.0f,
		1.0f, -1.0f
	};

	float colors[] = {
		0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
	};

	
	for (int i = 0; i < 4; ++i) {
		solids.push_back(entity_manager.Create()); // legs
		sys_renderable.Add(solids.back(), hcube, RigidBody{
			glm::vec3(pos[i*2] *5.0f, -1.0f, pos[i * 2 + 1] * 5.0f),
			glm::vec3(0.12f, 0.8f, 0.12f),
			glm::quat()
			});
		sys_material.Add(solids.back());
		// sys_material.GetEdit(solids.back()).diffuse = glm::vec3(colors[i * 3], colors[i * 3 + 1], colors[i * 3 + 2]);
	}

	auto hMonet = AssetManager::LoadTexture("test/img/Monet.bmp");

	
	while (renderer::IsAlive())
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);
		using namespace renderer;
		

		model = glm::mat4(1.0f);
		UseShader(geopass);

		SetUniform(hproject, projection);
		SetUniform(hview, view);

		/*
		SetUniform(hmodel, model);
		
		SetUniform(hdiffuse, diffuse);
		SetUniform(hspecular, specular);
		SetUniform(halbedo, albedo);
		SetUniform(hmetal, metallic);
		SetUniform(hrough, rough);
		SetUniform(hao, ao);
		*/


		//DrawMesh(teapot.meshes_[0].hMesh);

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

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
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
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}
#endif
