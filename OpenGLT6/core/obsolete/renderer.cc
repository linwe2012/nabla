#include "renderer.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "logger.h"
#include "containers/vector.h"

#include "core/renderer/compiled-drawcall.h"
#include <thread>

namespace nabla {

namespace renderer {

	template <typename Buffer, typename Handle, int kMax>
	struct BufferManger {
		Buffer buffer[kMax];
		//typename Handle::index_t handles[kMax];
		uint16_t  handles[kMax];
		int current;
		BufferManger() : current(0) {
			for (int i = 0; i < kMax; ++i) {
				handles[i] = i + 1;
			}
		}

		Handle NewHandle() {
			if (current == kMax) {
				return Handle::MakeNil();
			}

			int cur = current;
			current = handles[cur];

			return Handle(cur);
		}
	};

	static BufferManger<VertexBuffer, VertexBufferHandle, 4096> vertex_buffer_;
	static BufferManger<IndexBuffer, IndexBufferHandle, 4096> index_buffer_;
	static BufferManger<Shader, ShaderHandle, 4096> shader_buffer_;
	static BufferManger<TextureBuffer, TextureHandle, 4096> texture_buffer_;
	// static BufferManger<UniformHanlde, UniformBuffer, 4096> texture_buffer_;

	RenderContext render_context_;

	Window* gDefaultWindow = nullptr;
	SafeVector<Window*> windows_;

	void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
	}

	Window* Window::Create(const InitConfig& cfg)
	{
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_REFRESH_RATE, cfg.fps_hint);
#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif
		GLFWwindow* glfwwin = glfwCreateWindow(cfg.width, cfg.height, cfg.name, NULL, NULL);
		Window* window = new Window();

		window->glfw_window_ = glfwwin;
		window->is_alive_ = true;
		glfwMakeContextCurrent(glfwwin);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			NA_LOG_FATAL("Failed to initialize GLAD");
			return nullptr;
		}

		
		glfwSetFramebufferSizeCallback(glfwwin, framebuffer_size_callback);
		glViewport(0, 0, cfg.width, cfg.height);
		return window;
	}

	void Window::NextFrame() {
		NA_ASSERT(glfw_window_ != nullptr, "unexpected null window");
		GLFWwindow* win = static_cast<GLFWwindow*>(glfw_window_);
		glClear(GL_COLOR_BUFFER_BIT);
		glfwPollEvents();
		glfwSwapBuffers(win);
		is_alive_ = !glfwWindowShouldClose(win);
	}

	//using Drawcall = Handle<12, 20>;
	//static BufferManger<Drawcall, DrawPrimivePack, 4096> primitive;

	void Init(const InitConfig& cfg)
	{
		windows_.push_back(Window::Create(cfg));
		gDefaultWindow = windows_.back();
		render_context_.resources.SetBuffer(new uint64_t[4096], 4096 * sizeof(uint64_t));
	}

	void NextFrame()
	{
		render_context_.Reset();
		gDefaultWindow->NextFrame();
	}
	bool IsAlive()
	{
		return gDefaultWindow->IsAlive();
	}

	VertexBufferHandle NewVertexBuffer(void* data, uint32_t size)
	{
		VertexBufferHandle h = vertex_buffer_.NewHandle();
		if (h.IsNil()) {
			return h;
		}
		
		vertex_buffer_.buffer[h.index()].Create(data, size);
		return h;
	}

	ShaderHandle NewShader(const char* vertexPath, const char* fragmentPath, const char* geometryPath)
	{
		ShaderHandle h = shader_buffer_.NewHandle();
		if (h.IsNil()) {
			return h;
		}
		
		shader_buffer_.buffer[h.index()].CompileShader(vertexPath, fragmentPath, geometryPath);
		return h;
	}

	namespace backend {

		void DrawIndexed(void* rdata) {
			IndexedDrawCall* idx = static_cast<IndexedDrawCall*>(rdata);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			// glBindBuffer(GL_ARRAY_BUFFER, )

		}
	}

	struct DrawPrimivePack {
		uint32_t vao;
		ShaderInfo shaderinfo;
		void* unifroms;
		glm::mat4 model;
	};

	/*
	Vector<DrawPrimivePack> primitives;
	static BufferManger<MeshBuffer, MeshHandle, 4096> mesh_buffer_;
	struct Drawcall {
		DrawPrimivePack* prim;
		static Drawcall New(MeshHandle);
		void Attach(ShaderInfo shader);
		void Attach(glm::mat4 model);
		void Uniform(ShaderInfo flags, uint32_t val);
		void Uniform(ShaderInfo flags, float val);
		void Uniform(ShaderInfo flags, glm::vec3);
	};*/

	/*
	Drawcall NewDrawcall(VertexBufferHandle, IndexBufferHandle) {

	}

	void Attach(Drawcall dc, ShaderInfo shader) {

	}

	void Attach(Drawcall dc, glm::mat4 uniform) {

	}*/
	
	

	void DrawIndexed(VertexBufferHandle vertex, IndexBufferHandle index, ShaderInfo shader)
	{
		static uint16_t draw_id = 0;
		Command cmd;
		cmd.dispatch = backend::DrawIndexed;
		cmd.offset = render_context_.ResourcesOffset();
		cmd.sortkey.set_handle(draw_id++);
		//render_context_.resources.ListCtor<>
	}

	void SetUniform() {

	}

	void SetTransform() {

	}

	
	



}
}

