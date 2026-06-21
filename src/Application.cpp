#include <iostream>
#include <exception>
#include <string>
#include <string_view>
#include <vector>
#include <random>
#include <cmath>
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

class ParticleBuffer
{
public:
	std::vector<float> mass;
	std::vector<float> radii;
	std::vector<glm::vec2> positions;
	std::vector<glm::vec2> velocities;
	std::vector<glm::vec3> colors;

	int num_max_particles;
	int num_active_particles;

	ParticleBuffer() : num_max_particles(2000), num_active_particles(num_max_particles / 2)
	{
		mass.resize(num_max_particles);
		radii.resize(num_max_particles);
		positions.resize(num_max_particles);
		velocities.resize(num_max_particles);
		colors.resize(num_max_particles);

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> mass_dis(0.001f, 0.005f);
		std::uniform_real_distribution<float> pos_dis(-0.5f, 0.5f);
		std::uniform_real_distribution<float> vel_dis(-1.0f, 1.0f);
		std::uniform_real_distribution<float> color_dis(0.2f, 1.0f);

		for (size_t i = 0; i < num_max_particles; ++i)
		{
			constexpr float r = 0.008f;

			radii[i] = r;
			mass[i] = mass_dis(gen);
			positions[i] = glm::vec2(pos_dis(gen), pos_dis(gen));
			velocities[i] = glm::vec2(vel_dis(gen), vel_dis(gen));
			colors[i] = glm::vec3(color_dis(gen), color_dis(gen), color_dis(gen));
		}
	}
	~ParticleBuffer() = default;
};

void Application::Update()
{
	ParticleBuffer particles;

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

	float time_scale = 1.0f;

	float temperature = 0.0f;
	float total_kinetic_energy = 0.0f;
	glm::vec2 total_momentum{ 0 };

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
				ImGui::Text("Frames per second: %zu", fps);
				ImGui::Text("Frame time: %.2f ms", ms_per_frame);
				ImGui::Text("Frame count: %zu", timer.frame_count());
			}

			ImGui::Separator();

			if (ImGui::CollapsingHeader("Playback Controls", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::SliderFloat("Time scale", &time_scale, 0.0f, 1.0f, "%.2fx");
				ImGui::SliderInt("Active particles", &particles.num_active_particles, 0, particles.num_max_particles);
			}

			ImGui::Separator();

			if (ImGui::CollapsingHeader("Physical Properties", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Text("Temperature (Kelvins): %.10f", temperature);
				ImGui::Text("Total kinetic energy (Joules): %.10f", total_kinetic_energy);
				ImGui::Text("Total momentum (kg * m/s): (x: %.10f; y: %.10f)", total_momentum.x, total_momentum.y);
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

		glm::vec2 current_total_momentum{ 0 };
		float current_total_kinetic_energy = 0.0f;

		const float time = Timer::time();
		const float aspect_ratio = static_cast<float>(framebufferHeight) / static_cast<float>(framebufferWidth);
		for (size_t i = 0; i < particles.num_active_particles; ++i)
		{
			for (size_t j = i + 1; j < particles.num_active_particles; ++j)
			{
				glm::vec2 pos1(particles.positions[i].x / aspect_ratio, particles.positions[i].y);
				glm::vec2 pos2(particles.positions[j].x / aspect_ratio, particles.positions[j].y);

				glm::vec2 vel1(particles.velocities[i].x / aspect_ratio, particles.velocities[i].y);
				glm::vec2 vel2(particles.velocities[j].x / aspect_ratio, particles.velocities[j].y);

				const auto pos_diff = pos1 - pos2;
				const auto distance = glm::length(pos_diff);
				const auto min_dist = particles.radii[i] + particles.radii[j];

				if (distance > min_dist || distance == 0.0f)
					continue;

				const auto vel_diff = vel1 - vel2;
				const auto dot_prod = glm::dot(vel_diff, pos_diff);
				if (dot_prod >= 0.0f)
					continue;

				const auto dist_sq = distance * distance;
				const auto total_mass = particles.mass[i] + particles.mass[j];
				const auto impulse_scalar = dot_prod / dist_sq;

				glm::vec2 new_vel1 = vel1 - (2.0f * particles.mass[j] / total_mass) * impulse_scalar * pos_diff;
				glm::vec2 new_vel2 = vel2 + (2.0f * particles.mass[i] / total_mass) * impulse_scalar * pos_diff;

				particles.velocities[i] = glm::vec2(new_vel1.x * aspect_ratio, new_vel1.y);
				particles.velocities[j] = glm::vec2(new_vel2.x * aspect_ratio, new_vel2.y);
			}

			particles.positions[i] += particles.velocities[i] * dt;

			const float horizontal_radius = particles.radii[i] * aspect_ratio;
			if (particles.positions[i].x - horizontal_radius < -1.0f)
			{
				particles.positions[i].x = -1.0f + horizontal_radius;
				particles.velocities[i].x *= -1.0f;
			}
			else if (particles.positions[i].x + horizontal_radius > 1.0f)
			{
				particles.positions[i].x = 1.0f - horizontal_radius;
				particles.velocities[i].x *= -1.0f;
			}
			if (particles.positions[i].y - particles.radii[i] < -1.0f)
			{
				particles.positions[i].y = -1.0f + particles.radii[i];
				particles.velocities[i].y *= -1.0f;
			}
			else if (particles.positions[i].y + particles.radii[i] > 1.0f)
			{
				particles.positions[i].y = 1.0f - particles.radii[i];
				particles.velocities[i].y *= -1.0f;
			}

			glUniform2fv(positionLocation, 1, glm::value_ptr(particles.positions[i]));
			glUniform3fv(colorLocation, 1, glm::value_ptr(particles.colors[i]));
			glUniform1f(radiusLocation, particles.radii[i]);

			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			{
				glm::vec2 true_vel(particles.velocities[i].x / aspect_ratio, particles.velocities[i].y);

				const float velocity_sq = glm::dot(true_vel, true_vel);
				current_total_kinetic_energy += particles.mass[i] * velocity_sq / 2.0f;

				const glm::vec2 linear_momentum = true_vel * particles.mass[i];
				current_total_momentum += linear_momentum;
			}
		}

		total_kinetic_energy = current_total_kinetic_energy;
		temperature = particles.num_active_particles != 0 ? total_kinetic_energy / particles.num_active_particles : 0.0f;
		total_momentum = current_total_momentum;

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