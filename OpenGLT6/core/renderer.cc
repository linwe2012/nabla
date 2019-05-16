#include "renderer.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "utils.h"


namespace nabla {
namespace renderer {

void GBuffer::Create(int width, int height) {
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// position color
	glGenTextures(1, &position);
	glBindTexture(GL_TEXTURE_2D, position);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, position, 0);

	// normal map
	glGenTextures(1, &normal);
	glBindTexture(GL_TEXTURE_2D, normal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normal, 0);

	// albedo & specular
	glGenTextures(1, &albedo_spec);
	glBindTexture(GL_TEXTURE_2D, albedo_spec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, albedo_spec, 0);

	GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);
	// - Create and attach depth buffer (renderbuffer)
		
	glGenRenderbuffers(1, &rbo_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth);

	// - Finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

static GLFWwindow* gDefaultWindow;
GBuffer gGBuffer;

void Init(const InitConfig& cfg) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_REFRESH_RATE, cfg.fps_hint);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif
	GLFWwindow* glfwwin = glfwCreateWindow(cfg.width, cfg.height, cfg.name, NULL, NULL);
	
	glfwMakeContextCurrent(glfwwin);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		NA_LOG_FATAL("Failed to initialize GLAD");
		return;
	}

	auto framecb = 
	glfwSetFramebufferSizeCallback(glfwwin, [](GLFWwindow * window, int width, int height) {
		glViewport(0, 0, width, height);
	});

	glViewport(0, 0, cfg.width, cfg.height);
	gDefaultWindow = glfwwin;
	// gGBuffer.Create(cfg.width, cfg.height);
}
void NextFrame()
{
	NA_ASSERT(gDefaultWindow != nullptr, "unexpected null window");
	glfwPollEvents();
	glfwSwapBuffers(gDefaultWindow);
	
}

bool IsAlive() {
	return !glfwWindowShouldClose(gDefaultWindow);
}

void PrepareFrame()
{

}

void* GetWindow()
{
	return gDefaultWindow;
}

}
}