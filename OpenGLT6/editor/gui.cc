#include "gui.h"
#include "core/renderer.h"

#include "GLFW/glfw3.h"
namespace nabla {

void InitGui() {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(renderer::GetWindow()), true);
	ImGui_ImplOpenGL3_Init("#version 330 core");
}

void PrepareGuiFrame()
{
	ImGui::GetIO().WantCaptureMouse = true;
	ImGui::GetIO().WantCaptureKeyboard = true;
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

bool IsUserWorkingOnGui()
{
	return ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::IsAnyItemActive();
}

void EndGuiFrame()
{
	ImGui::Render();
	glfwMakeContextCurrent(static_cast<GLFWwindow*>(renderer::GetWindow()));
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glfwMakeContextCurrent(static_cast<GLFWwindow*>(renderer::GetWindow()));
	renderer::NextFrame();
}

}
