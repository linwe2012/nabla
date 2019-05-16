#if 1
#include <iostream>
#include <regex>
#include <thread>

#include "systems/animation.h"

#include "core/entity-manager.h"
#include "core/renderer.h"
#include "core/asset/bootstrap.h"
#include "core/camera.h"

#include "GLFW/glfw3.h"
#include "editor/gui.h"

#include "systems/lighting.h"


void renderQuad();
void renderCube();

int main()
{
	using namespace nabla;
	std::ifstream ifs("test/ubershaders/gbuffer.vs");
	Set<std::string> str;
	str.insert("Bitangent");
	str.insert("Tangent");
	std::string res = renderer::PreprocessShader(ifs, str, 330);
	std::cout << res;
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
	sys_lighting.Initilize();
	Vector<Entity> lights;
	lights.push_back(entity_manager.Create());
	lights.push_back(entity_manager.Create());

	AssetManager assets;
	assets.ParseAssetsFromFile("./test/assets.yml");
	auto teapot = assets.LoadModelToGPU("teapot", false);
	// renderer::NewShader(renderer::ShaderFilePath{"test", "test"});
	Set<std::string> macros;
	macros.insert("NormalMap");
	macros.insert("Bitangent");
	macros.insert("Tangent");
	macros.insert("TexCoords");
	macros.insert("Diffuse");
	macros.insert("Specular");

	auto geopass = renderer::NewShader(renderer::ShaderFilePath{ "test/ubershaders/gbuffer.vs", "test/ubershaders/gbuffer.fs" }, macros);
	auto hdiffuse = renderer::NewUniform(geopass, "Diffuse", renderer::MaterialType::kVec3);
	auto hspecular = renderer::NewUniform(geopass, "Specular", renderer::MaterialType::kFloat);
	auto hmodel = renderer::NewUniform(geopass, "model", renderer::MaterialType::kMat4);
	auto hview = renderer::NewUniform(geopass, "view", renderer::MaterialType::kFloat);
	auto hproject = renderer::NewUniform(geopass, "projection", renderer::MaterialType::kFloat);

	auto lightingpass = renderer::NewShader(renderer::ShaderFilePath{ "test/ubershaders/deferred-shading.vs", "test/ubershaders/deferred-shading.fs" }, macros);
	sys_lighting.SetShader(lightingpass);
	sys_lighting.NewLight(lights[0], LightingSystem::Light{
		LightingSystem::Light::kPoint
		});
	sys_lighting.NewLight(lights[1], LightingSystem::Light{
		LightingSystem::Light::kSpot
		});

	auto postprocess = renderer::NewShader(renderer::ShaderFilePath{ "test/ubershaders/deferred-light-box.vs", "test/ubershaders/deferred-light-box.fs" }, macros);
	auto hbox_proj = renderer::NewUniform(postprocess, "projection", renderer::MaterialType::kMat4);
	auto hbox_view = renderer::NewUniform(postprocess, "view", renderer::MaterialType::kMat4);
	auto hbox_model = renderer::NewUniform(postprocess, "model", renderer::MaterialType::kMat4);
	auto hbox_lightColor = renderer::NewUniform(postprocess, "lightColor", renderer::MaterialType::kVec3);

	renderer::GBuffer gbuffer;
	gbuffer.Create(SCR_WIDTH, SCR_HEIGHT);
	glEnable(GL_DEPTH_TEST);
	Camera camera;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	using renderer::OpenHandle;
	float last_frame = glfwGetTime();
	float time_acc = 0.0f;
	bool first_move = true;
	float last_x = 0.0f, last_y = 0.0f;

	InitGui();

	glm::vec3 diffuse(4.0f, 4.0f, 4.0f);
	float specular = 0.5f;
	bool display_box = true;
	Clock clock;

	while (renderer::IsAlive())
	{
		auto end_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(16);
		float current_frame = static_cast<float>(glfwGetTime());
		float delta_time = current_frame - last_frame;
		last_frame = current_frame;

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, gbuffer.fbo);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);

		
		OpenHandle(geopass).Use();

		glUniformMatrix4fv(OpenHandle(hproject), 1, GL_FALSE, &projection[0][0]);
		glUniformMatrix4fv(OpenHandle(hview), 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(OpenHandle(hmodel), 1, GL_FALSE, &model[0][0]);

		glUniform3fv(OpenHandle(hdiffuse), 1, &diffuse[0]);
		glUniform1f(OpenHandle(hspecular), specular);

		glBindVertexArray(OpenHandle(teapot.meshes_[0].hMesh).vao);
		glDrawElements(GL_TRIANGLES, OpenHandle(teapot.meshes_[0].hMesh).num_indices, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);



		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		OpenHandle(lightingpass).Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gbuffer.position);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gbuffer.normal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gbuffer.albedo_spec);
		
		sys_lighting.SetEyePos(camera.Position);
		sys_lighting.Update(clock);
		renderer::FlushAllDrawCalls();
		renderer::GetRenderContext()->Reset(renderer::GetRenderContext()->resources.buffer, renderer::GetRenderContext()->resources.end_of_storage - renderer::GetRenderContext()->resources.buffer);
		renderQuad();

		if (display_box) {
			// 2.5. copy content of geometry's depth buffer to default framebuffer's depth buffer
			 // ----------------------------------------------------------------------------------
			glBindFramebuffer(GL_READ_FRAMEBUFFER, gbuffer.fbo);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
			// blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
			// the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the 		
			// depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
			glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			
			auto l = sys_lighting.GetLight(lights[0]);
			OpenHandle(postprocess).Use();
			glUniformMatrix4fv(OpenHandle(hbox_proj), 1, GL_FALSE, &projection[0][0]);
			glUniformMatrix4fv(OpenHandle(hbox_view), 1, GL_FALSE, &view[0][0]);
			model = glm::mat4(1.0f);
			model = glm::scale(model, glm::vec3(0.125f));
			model = glm::translate(model, l->position);
		
			glUniformMatrix4fv(OpenHandle(hbox_model), 1, GL_FALSE, &model[0][0]);
			glUniform3fv(OpenHandle(hbox_lightColor), 1, &l->color[0]);
			
			renderCube();

			l = sys_lighting.GetLight(lights[1]);
			
			model = glm::mat4(1.0f);
			model = glm::scale(model, glm::vec3(0.125f));
			model = glm::translate(model, l->position);

			glUniformMatrix4fv(OpenHandle(hbox_model), 1, GL_FALSE, &model[0][0]);
			glUniform3fv(OpenHandle(hbox_lightColor), 1, &l->color[0]);

			renderCube();
		}


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
				last_x = xpos;
				last_y = ypos;
				first_move = 1;

			}
			else
				first_move = 0;
		}


		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera.ProcessKeyboard(FORWARD, delta_time);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera.ProcessKeyboard(BACKWARD, delta_time);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.ProcessKeyboard(LEFT, delta_time);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera.ProcessKeyboard(RIGHT, delta_time);
		if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
			camera.ProcessKeyboard(UP, delta_time);
		if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
			camera.ProcessKeyboard(DOWN, delta_time);
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			camera.BudgeCamera(0.0f, 0.5f);


		camera.PassDeltaTime(delta_time);

		{
			ImGui::Begin("Editor");

			if (ImGui::TreeNode("Lighting")) {
				sys_lighting.OnGui(lights);
				ImGui::TreePop();
				ImGui::Separator();
			}


			if (ImGui::TreeNode("Teapot - Material")) {
				ImGui::ColorEdit3("Diffuse", &diffuse.x);

				ImGui::DragFloat("Specular", &specular, 0.01f, 0.0f, 1.0f);

				ImGui::TreePop();
				ImGui::Separator();
			}
			clock.NextFrame();
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", clock.GetLastFrameDuration(), 1000.0f / clock.GetLastFrameDuration());
			ImGui::End();
		}

		EndGuiFrame();

		sys_animation.update(0.2f);
		time_acc += 0.0001;

		std::this_thread::sleep_until(end_time);
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
