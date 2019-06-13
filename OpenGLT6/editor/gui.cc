#include "gui.h"
#include "core/renderer.h"

#include "GLFW/glfw3.h"
namespace nabla {

void InitGui() {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// io.Fonts->AddFontDefault();

	ImFontConfig config;
	config.MergeMode = false;
	// config.GlyphMinAdvanceX = 13.0f; // Use if you want to make the icon monospaced
	config.OversampleH = 6;
	config.OversampleV = 6;
	static const ImWchar icon_ranges[] = { 0, 255, 0 };
	
	io.Fonts->AddFontFromFileTTF("assets/fonts/Proxima-Nova-Reg.ttf", 16.0f, &config, nullptr);
	io.Fonts->AddFontDefault();
	io.Fonts->AddFontFromFileTTF("assets/fonts/Proxima-Nova-Bold.ttf", 16.0f, nullptr, nullptr);
	io.Fonts->AddFontFromFileTTF("assets/fonts/DroidSans.ttf", 16.0f, nullptr, nullptr);
	io.Fonts->AddFontFromFileTTF("assets/fonts/Proxima-Nova-Thin.ttf", 18.0f, nullptr, nullptr);
	io.Fonts->AddFontFromFileTTF("assets/fonts/GoogleSans-Medium.ttf", 18.0f, nullptr, nullptr);
	io.Fonts->AddFontFromFileTTF("assets/fonts/ProggyTiny.ttf", 13.0f, nullptr, nullptr);
	io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Medium.ttf", 16.0f, nullptr, nullptr);
	// io.Fonts->Build();

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
	// ImGuiIO& io = ImGui::GetIO();
	// ImFontAtlas* atlas = io.Fonts;
	// ImFont* font = atlas->Fonts[0];
	// font->Scale = 0.9;
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
