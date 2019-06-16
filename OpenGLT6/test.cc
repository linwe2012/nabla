#if 0
#include <iostream>
#include <regex>
#include <thread>

#include "systems/animation.h"

#include "core/entity-manager.h"
#include "core/renderer.h"
#include "core/asset/bootstrap.h"
#include "core/camera.h"

#include "GLFW/glfw3.h"

#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#pragma warning (push)
#pragma warning (disable: 26495)
#pragma warning (disable: 26451)
#include "imgui/imgui.h"
#include "imgui/impl/imgui_impl_glfw.h"
#include "imgui/impl/imgui_impl_opengl3.h"
#pragma warning (pop)
struct Light {
	glm::vec3 position;
	glm::vec3 color;
	float linear;
	float quad;
	float radius;
};
void RenderQuad();
void renderCube();

int main()
{
	using namespace nabla;
	std::ifstream ifs("test/ubershaders/gbuffer.vs");
	Set<std::string> str;
	str.insert("Bitangent");
	str.insert("Tangent");
	std::string res =  renderer::PreprocessShader(ifs, str, 330);
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

	AnimationSystem sys_animation;

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
	auto num_lights = renderer::NewUniform(lightingpass, "num_lights", renderer::MaterialType::kFloat);
	auto hlight1_pos = renderer::NewUniform(lightingpass, "lights[0].Position", renderer::MaterialType::kVec3);
	auto hlight1_color = renderer::NewUniform(lightingpass, "lights[0].Color", renderer::MaterialType::kVec3);
	auto hlight1_linear = renderer::NewUniform(lightingpass, "lights[0].Linear", renderer::MaterialType::kFloat);
	auto hlight1_quad = renderer::NewUniform(lightingpass, "lights[0].Quadratic", renderer::MaterialType::kFloat);
	auto hlight1_rad = renderer::NewUniform(lightingpass, "lights[0].Radius", renderer::MaterialType::kFloat);
	auto hcam_pos = renderer::NewUniform(lightingpass, "viewPos", renderer::MaterialType::kVec3);

	
	renderer::OpenHandle(lightingpass).Use();
	renderer::OpenHandle(lightingpass).SetInt("gPosition", 0);
	renderer::OpenHandle(lightingpass).SetInt("gNormal", 1); 
	renderer::OpenHandle(lightingpass).SetInt("gAlbedoSpec", 2);

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
	Light light1;
	using renderer::OpenHandle;
	float last_frame = glfwGetTime();
	float time_acc = 0.0f;
	bool first_move = true;
	float last_x = 0.0f, last_y = 0.0f;

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(renderer::GetWindow()), true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	light1.linear = 0.014f;
	light1.quad = 0.0007f;
	light1.radius = 10.0f;
	glm::vec3 diffuse(4.0f, 4.0f, 4.0f);
	float specular = 0.5f;
	bool display_box = true;
	while (renderer::IsAlive())
	{
		auto end_time = std::chrono::steady_clock::now() + std::chrono::microseconds(16);
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

		glUniform3fv(OpenHandle(hlight1_pos), 1, &light1.position[0]);
		glUniform3fv(OpenHandle(hlight1_color), 1, &light1.color[0]);
		glUniform1f(OpenHandle(hlight1_linear), light1.linear);
		glUniform1f(OpenHandle(hlight1_quad), light1.quad);
		glUniform1f(OpenHandle(hlight1_rad), light1.radius);
		glUniform3fv(OpenHandle(hcam_pos), 1, &camera.Position[0]);
		glUniform1i(OpenHandle(num_lights), 1);

		RenderQuad();

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

			OpenHandle(postprocess).Use();
			glUniformMatrix4fv(OpenHandle(hbox_proj), 1, GL_FALSE, &projection[0][0]);
			glUniformMatrix4fv(OpenHandle(hbox_view), 1, GL_FALSE, &view[0][0]);
			model = glm::mat4(1.0f);
			model = glm::scale(model, glm::vec3(0.125f));
			model = glm::translate(model, light1.position);

			glUniformMatrix4fv(OpenHandle(hbox_model), 1, GL_FALSE, &model[0][0]);
			glUniform3fv(OpenHandle(hbox_lightColor), 1, &light1.color[0]);

			renderCube();
		}
		

		// Start the Dear ImGui frame
		ImGui::GetIO().WantCaptureMouse = true;
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::ShowDemoWindow();
		
		GLFWwindow* window = static_cast<GLFWwindow*>(renderer::GetWindow());
		if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && !ImGui::IsAnyItemActive()) {
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

			if (ImGui::TreeNode("Light")) {
				ImGui::Checkbox("Display Light Box", &display_box);
				ImGui::ColorPicker3("Color", &light1.color[0]);
				
				ImGui::DragFloat("Position.x", &light1.position.x, 0.5f, -15.0f, 15.0f);
				ImGui::DragFloat("Position.y", &light1.position.y, 0.5f, -15.0f, 15.0f);
				ImGui::DragFloat("Position.z", &light1.position.z, 0.5f, -15.0f, 15.0f);
				ImGui::DragFloat("Linear", &light1.linear, 0.01f, 0.0f, 2.0f, "%.3f");
				ImGui::DragFloat("Quad", &light1.quad, 0.0001f, 0.0f, 1.0f, "%.4f");

				ImGui::TreePop();
				ImGui::Separator();
			}
			
			if (ImGui::TreeNode("Teapot - Material")) {
				ImGui::ColorEdit3("Diffuse", &diffuse.x);

				ImGui::DragFloat("Specular", &specular, 0.01f, 0.0f, 1.0f);

				ImGui::TreePop();
				ImGui::Separator();
			}

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}
		// Rendering
		ImGui::Render();
		glfwMakeContextCurrent(static_cast<GLFWwindow*>(renderer::GetWindow()));
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwMakeContextCurrent(static_cast<GLFWwindow*>(renderer::GetWindow()));
		renderer::NextFrame();
		
		sys_animation.update(0.2f);
		time_acc += 0.0001;
		
		std::this_thread::sleep_until(end_time);
	}
	return 0;
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void RenderQuad()
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