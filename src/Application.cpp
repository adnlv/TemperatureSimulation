#include <iostream>
#include <exception>
#include <string>
#include <string_view>
#include <vector>
#include <random>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Log.h"
#include "Shader.h"
#include "Buffers.h"
#include "Timer.h"

class Application
{
public:
	Application();
	~Application() noexcept;

private:
	GLFWwindow* m_Window;

	void GLFWInit();
	void GLFWTerminate() noexcept;
	void GLFWCreateWindow();
	void GLFWDestroyWindow() noexcept;
	void GLFWSetFramebufferSizeCallback();
	void GLFWSetKeyCallback();

	void GLADLoadGL();

	void GLSetDebugOutputCallback();

	void imgui_init() noexcept;
	void imgui_shutdown() noexcept;

	void Update();
};

Application::Application()
{
	GLFWInit();
	GLFWCreateWindow();

	GLADLoadGL();

	GLFWSetFramebufferSizeCallback();
	GLFWSetKeyCallback();

	GLSetDebugOutputCallback();
	imgui_init();

	Update();
}

Application::~Application()
{
	imgui_shutdown();
	GLFWDestroyWindow();
	GLFWTerminate();
}

void Application::GLFWInit()
{
	if (glfwInit() != GLFW_TRUE)
	{
		throw std::runtime_error("GLFW: Initialization failed");
	}
	Log::Info("GLFW: Initialized");

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
}

void Application::GLFWTerminate() noexcept
{
	glfwTerminate();
	Log::Info("GLFW: Terminated");
}

void Application::GLFWCreateWindow()
{
	auto scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
	auto w = static_cast<int>(960 * scale);
	auto h = static_cast<int>(540 * scale);
	m_Window = glfwCreateWindow(w, h, "Temperature Simulation", nullptr, nullptr);
	if (m_Window == nullptr)
	{
		throw std::runtime_error("GLFW: Window creation failed");
	}
	Log::Info("GLFW: Window created");

	glfwMakeContextCurrent(m_Window);
	glfwSetWindowUserPointer(m_Window, this);
}

void Application::GLFWDestroyWindow() noexcept
{
	glfwDestroyWindow(m_Window);
	Log::Info("GLFW: Window destroyed");
}

void Application::GLFWSetFramebufferSizeCallback()
{
	glfwSetFramebufferSizeCallback(m_Window,
		[](GLFWwindow* window, int width, int height)
		{
			glViewport(0, 0, width, height);
			Log::Info("GL: Viewport resolution changed | Width: {} px, Height: {} px", width, height);
		});
}

void Application::GLFWSetKeyCallback()
{
	glfwSetKeyCallback(m_Window,
		[](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			bool isHandlable = true;
			if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
			{
				glfwSetWindowShouldClose(window, true);
			}
			else
			{
				isHandlable = false;
			}

			if (isHandlable)
			{
				Log::Info("GL: Key action | Key: {:#x}, Scancode: {:#x}, Action: {}, Mods: {:#x}", key, scancode, action, mods);
			}
		});
}

void Application::GLADLoadGL()
{
	if (!gladLoadGL(glfwGetProcAddress))
	{
		throw std::runtime_error("GLAD: GL loading failed");
	}
	Log::Info("GLAD: GL loaded");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int frameBufferWidth;
	int frameBufferHeight;
	glfwGetFramebufferSize(m_Window, &frameBufferWidth, &frameBufferHeight);
	glViewport(0, 0, frameBufferWidth, frameBufferHeight);
}

void Application::GLSetDebugOutputCallback()
{
	int flags = 0;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(
			[](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
			{
				std::string source_str;
				switch (source) {
				case GL_DEBUG_SOURCE_API:
					source_str = "API";
					break;
				case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
					source_str = "WINDOW_SYSTEM";
					break;
				case GL_DEBUG_SOURCE_SHADER_COMPILER:
					source_str = "SHADER_COMPILER";
					break;
				case GL_DEBUG_SOURCE_THIRD_PARTY:
					source_str = "THIRD_PARTY";
					break;
				case GL_DEBUG_SOURCE_APPLICATION:
					source_str = "APPLICATION";
					break;
				case GL_DEBUG_SOURCE_OTHER:
					source_str = "OTHER";
					break;
				}

				std::string type_str;
				switch (type) {
				case GL_DEBUG_TYPE_ERROR:
					type_str = "ERROR";
					break;
				case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
					type_str = "DEPRECATED_BEHAVIOR";
					break;
				case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
					type_str = "UNDEFINED_BEHAVIOR";
					break;
				case GL_DEBUG_TYPE_PORTABILITY:
					type_str = "PORTABILITY";
					break;
				case GL_DEBUG_TYPE_PERFORMANCE:
					type_str = "PERFORMANCE";
					break;
				case GL_DEBUG_TYPE_MARKER:
					type_str = "MARKER";
					break;
				case GL_DEBUG_TYPE_PUSH_GROUP:
					type_str = "PUSH_GROUP";
					break;
				case GL_DEBUG_TYPE_POP_GROUP:
					type_str = "POP_GROUP";
					break;
				case GL_DEBUG_TYPE_OTHER:
					type_str = "OTHER";
					break;
				}

				LogLevel level;
				switch (type) {
				case GL_DEBUG_TYPE_ERROR:
					level = LogLevel::Err;
					break;
				case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
				case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
				case GL_DEBUG_TYPE_PORTABILITY:
				case GL_DEBUG_TYPE_PERFORMANCE:
					level = LogLevel::Warn;
					break;
				default:
					level = LogLevel::Info;
					break;
				}

				std::string severity_str;
				switch (severity) {
				case GL_DEBUG_SEVERITY_HIGH:
					severity_str = "HIGH";
					break;
				case GL_DEBUG_SEVERITY_MEDIUM:
					severity_str = "MEDIUM";
					break;
				case GL_DEBUG_SEVERITY_LOW:
					severity_str = "LOW";
					break;
				case GL_DEBUG_SEVERITY_NOTIFICATION:
					severity_str = "NOTIFICATION";
					break;
				}

				Log::Message(level, "GL: {} severity {} message from {} ({}): {}", severity_str, type_str, source_str, id, message);
			}, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
}

void Application::imgui_init() noexcept
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();

	float scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(scale);
	style.FontScaleDpi = scale;

	ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
	ImGui_ImplOpenGL3_Init("#version 460");
}

void Application::imgui_shutdown() noexcept
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void Application::Update()
{
	struct Particle
	{
		float mass{ 1.0f };
		float radius{ 0 };
		glm::vec2 position;
		glm::vec2 velocity;
		glm::vec3 color;
	};

	std::vector<Particle> particles;
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> vel_dis(-0.01f, 0.01f);
		std::uniform_real_distribution<float> pos_dis(-0.5f, 0.5f);
		std::uniform_real_distribution<float> mass_dis(0.005f, 0.005f);
		std::uniform_real_distribution<float> color_dis(0.2f, 1.0f);
		std::uniform_int_distribution<size_t> n_dis(1000, 2000);

		const size_t n = n_dis(gen);
		for (size_t i = 0; i < n; ++i)
		{
			const float mass = mass_dis(gen);
			Particle particle{
				mass,
				mass * 2,
				{pos_dis(gen), pos_dis(gen)},
				{vel_dis(gen), vel_dis(gen)},
				{color_dis(gen), color_dis(gen), color_dis(gen)},
			};
			particles.push_back(particle);
		}

		Log::Debug("initialized {} particles", n);
	}

	std::vector<float> vertices{
		1.0f,  1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f
	};
	std::vector<int> indices{
		0, 1, 3,
		1, 2, 3
	};

	VertexArray vertexArray;

	Buffer vertexBuffer(BufferType::ArrayBuffer, vertices.size() * sizeof(vertices.at(0)), vertices.data(), BufferUsage::StaticDraw);
	vertexArray.SetAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(vertices.at(0)), nullptr);
	vertexArray.EnableAttribArray(0);

	Buffer elementArray(BufferType::ElementArrayBuffer, indices.size() * sizeof(indices.at(0)), indices.data(), BufferUsage::StaticDraw);

	ShaderProgram program = []()
		{
			auto vertexShaderSource = Shader::LoadFile("shaders/circle.vert");
			Shader vertexShader(ShaderType::VertexShader, vertexShaderSource);

			auto fragmentShaderSource = Shader::LoadFile("shaders/circle.frag");
			Shader fragmentShader(ShaderType::FragmentShader, fragmentShaderSource);

			return ShaderProgram(vertexShader, fragmentShader);
		}();

	GLint resolutionLocation = program.GetUniformLocation("uResolution");
	GLint positionLocation = program.GetUniformLocation("uPosition");
	GLint radiusLocation = program.GetUniformLocation("uRadius");
	GLint colorLocation = program.GetUniformLocation("uColor");

	const int max_active_particles = static_cast<int>(particles.size());
	int active_particles_count = max_active_particles;
	float time_scale = 1.0f;

	Timer timer;
	while (!glfwWindowShouldClose(m_Window))
	{
		timer.start_frame();
		float dt = timer.dt() * time_scale;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		{
			ImGui::Begin("Simulation Panel");

			const size_t fps = timer.fps();
			const float ms_per_frame = fps != 0 ? 1000.0f / fps : 0;

			if (ImGui::CollapsingHeader("Performance Info", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Text("Frame count: %zu", timer.frame_count());
				ImGui::Text("Frames per second: %zu", fps);
				ImGui::Text("Frame time: %.2f ms", ms_per_frame);
			}

			ImGui::Separator();

			if (ImGui::CollapsingHeader("Playback Controls", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::SliderFloat("Time scale", &time_scale, 0.0f, 1.0f, "%.2fx");
				ImGui::SliderInt("Active particles", &active_particles_count, 0, max_active_particles);
			}

			ImGui::End();
		}

		int framebufferWidth, framebufferHeight;
		glfwGetFramebufferSize(m_Window, &framebufferWidth, &framebufferHeight);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		vertexArray.Bind();
		elementArray.Bind();
		program.Use();

		glm::vec2 resolution(framebufferWidth, framebufferHeight);
		glUniform2fv(resolutionLocation, 1, glm::value_ptr(resolution));

		const float time = Timer::time();
		const float aspect_ratio = static_cast<float>(framebufferHeight) / static_cast<float>(framebufferWidth);
		for (size_t i = 0; i < active_particles_count; ++i)
		{
			Particle& p1 = particles.at(i);
			for (size_t j = i + 1; j < active_particles_count; ++j)
			{
				Particle& p2 = particles.at(j);

				glm::vec2 pos1(p1.position.x / aspect_ratio, p1.position.y);
				glm::vec2 pos2(p2.position.x / aspect_ratio, p2.position.y);

				glm::vec2 vel1(p1.velocity.x / aspect_ratio, p1.velocity.y);
				glm::vec2 vel2(p2.velocity.x / aspect_ratio, p2.velocity.y);

				const auto pos_diff = pos1 - pos2;
				const auto distance = glm::length(pos_diff);
				const auto min_dist = p1.radius + p2.radius;

				if (distance > min_dist || distance == 0.0f)
					continue;

				const auto vel_diff = vel1 - vel2;
				const auto dot_prod = glm::dot(vel_diff, pos_diff);
				if (dot_prod >= 0.0f)
					continue;

				const auto dist_sq = distance * distance;
				const auto total_mass = p1.mass + p2.mass;
				const auto impulse_scalar = dot_prod / dist_sq;

				glm::vec2 new_vel1 = vel1 - (2.0f * p2.mass / total_mass) * impulse_scalar * pos_diff;
				glm::vec2 new_vel2 = vel2 + (2.0f * p1.mass / total_mass) * impulse_scalar * pos_diff;

				p1.velocity = glm::vec2(new_vel1.x * aspect_ratio, new_vel1.y);
				p2.velocity = glm::vec2(new_vel2.x * aspect_ratio, new_vel2.y);
			}

			p1.position += p1.velocity * dt;

			const float horizontal_radius = p1.radius * aspect_ratio;
			if (p1.position.x - horizontal_radius < -1.0f)
			{
				p1.position.x = -1.0f + horizontal_radius;
				p1.velocity.x *= -1.0f;
			}
			else if (p1.position.x + horizontal_radius > 1.0f)
			{
				p1.position.x = 1.0f - horizontal_radius;
				p1.velocity.x *= -1.0f;
			}
			if (p1.position.y - p1.radius < -1.0f)
			{
				p1.position.y = -1.0f + p1.radius;
				p1.velocity.y *= -1.0f;
			}
			else if (p1.position.y + p1.radius > 1.0f)
			{
				p1.position.y = 1.0f - p1.radius;
				p1.velocity.y *= -1.0f;
			}

			glUniform2fv(positionLocation, 1, glm::value_ptr(p1.position));
			glUniform3fv(colorLocation, 1, glm::value_ptr(p1.color));
			glUniform1f(radiusLocation, p1.radius);

			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(m_Window);
		glfwPollEvents();

		timer.end_frame();
	}
}

int main()
{
	Log::Init();
	Application app;
	return 0;
}