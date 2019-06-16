#include "input.h"
#include "GLFW/glfw3.h"
#include "renderer.h"

namespace nabla {
static GLFWwindow* Window() {
	return reinterpret_cast<GLFWwindow*>(renderer::GetWindow());
}

static bool mouse_is_first_click_[10] = { false };
static bool mouse_is_down_[10] = { false };
static bool key_is_down_[1024] = { false };

void PullInput() {
	
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS) {
		if (mouse_is_down_[button]) {
			mouse_is_first_click_[button] = false;
			return;
		}
		else {
			mouse_is_down_[button] = true;
			mouse_is_first_click_[button] = true;
		}
		return;
	}

	mouse_is_first_click_[button] = false;
	mouse_is_down_[button] = false;
	return;
}



bool MouseClick(int button)
{
	return mouse_is_first_click_[button];
}

bool KeyStroke(int button) {
	return 0;
}


}

