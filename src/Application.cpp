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
		std::uniform_real_distribution<float> mass_dis(6.0e-26f, 7.0e-26f); // Mass of an Argon atom (~6.6e-26 kg)
		std::uniform_real_distribution<float> pos_dis(-0.5f, 0.5f);
		std::uniform_real_distribution<float> vel_dis(-400.0f, 400.0f); // m/s
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

class Temperature
{
public:
	[[nodiscard]] float kelvin() const noexcept { return m_kelvin; }
	[[nodiscard]] float celsius() const noexcept { return m_kelvin + k_C_ABS_ZERO; }
	[[nodiscard]] float fahrenheit() const noexcept { return m_kelvin * k_K_TO_F_RATIO + k_F_ABS_ZERO; }

	void update_from_kinetic_energy(float energy) noexcept { m_kelvin = energy / k_B; }

private:
	// Boltzmann constant
	static constexpr float k_B{ 1.380649e-23f };
	
	static constexpr float k_C_ABS_ZERO{ -273.15f };
	static constexpr float k_F_ABS_ZERO{ -459.67f };
	static constexpr float k_K_TO_F_RATIO{ 1.8f };

	float m_kelvin{ 0.0f };
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

	Buffer vbo(BufferType::ArrayBuffer, vertices.size() * sizeof(vertices.at(0)), vertices.data(), BufferUsage::StaticDraw);
	vertexArray.SetAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(vertices.at(0)), nullptr);
	vertexArray.EnableAttribArray(2);

	Buffer radius_vbo(BufferType::ArrayBuffer, particles.num_max_particles * sizeof(particles.radii.at(0)), nullptr, BufferUsage::DynamicDraw);
	vertexArray.SetAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(particles.radii.at(0)), nullptr);
	vertexArray.EnableAttribArray(0);
	vertexArray.SetAttribDivisor(0, 1);

	Buffer center_vbo(BufferType::ArrayBuffer, particles.num_max_particles * sizeof(particles.positions.at(0)), nullptr, BufferUsage::DynamicDraw);
	vertexArray.SetAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(particles.positions.at(0)), nullptr);
	vertexArray.EnableAttribArray(1);
	vertexArray.SetAttribDivisor(1, 1);

	Buffer color_vbo(BufferType::ArrayBuffer, particles.num_max_particles * sizeof(particles.colors.at(0)), nullptr, BufferUsage::DynamicDraw);
	vertexArray.SetAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(particles.colors.at(0)), nullptr);
	vertexArray.EnableAttribArray(3);
	vertexArray.SetAttribDivisor(3, 1);

	Buffer elementArray(BufferType::ElementArrayBuffer, indices.size() * sizeof(indices.at(0)), indices.data(), BufferUsage::StaticDraw);

	ShaderProgram program = []()
		{
			auto vertexShaderSource = Shader::LoadFile("shaders/circle.vert");
			Shader vertexShader(ShaderType::VertexShader, vertexShaderSource);

			auto fragmentShaderSource = Shader::LoadFile("shaders/circle.frag");
			Shader fragmentShader(ShaderType::FragmentShader, fragmentShaderSource);

			return ShaderProgram(vertexShader, fragmentShader);
		}();

	GLint resolution_location = program.GetUniformLocation("u_resolution");

	bool vsync_enabled = false;
	glfwSwapInterval(vsync_enabled);

	float time_scale = 1.0f;
	float avg_kinetic_energy = 0.0f;
	float total_kinetic_energy = 0.0f;
	glm::vec2 total_momentum{ 0 };

	Temperature temperature;

	Timer timer;
	while (!glfwWindowShouldClose(m_Window))
	{
		timer.start_frame();

		// Multiply by a small time scale (e.g. 2e-4) so molecules don't cross the box in 1 frame
		constexpr float physical_time_scale = 2.0e-4f;

		float dt = timer.dt() * time_scale * physical_time_scale;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		{
			ImGui::Begin("Simulation Panel");

			const size_t fps = timer.fps();
			const float ms_per_frame = fps != 0 ? 1000.0f / fps : 0;

			if (ImGui::CollapsingHeader("Performance", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Text("Frames per second: %zu", fps);
				ImGui::Text("Frame time: %.2f ms", ms_per_frame);
				ImGui::Text("Frame count: %zu", timer.frame_count());
			}

			ImGui::Separator();

			if (ImGui::CollapsingHeader("Playback Controls", ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (ImGui::Checkbox("VSync", &vsync_enabled))
				{
					glfwSwapInterval(vsync_enabled);
				}

				ImGui::SliderFloat("Time scale", &time_scale, 0.0f, 1.0f, "%.4fx");
				ImGui::SliderInt("Active particles", &particles.num_active_particles, 0, particles.num_max_particles);
			}

			ImGui::Separator();

			if (ImGui::CollapsingHeader("Physical Properties", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Text("Temperature: %.2f (K), %.2f (C), %.2f (F)", temperature.kelvin(), temperature.celsius(), temperature.fahrenheit());
				ImGui::Text("Average kinetic energy: %.4e (J/particle)", avg_kinetic_energy);
				ImGui::Text("Total kinetic energy: %.4e (J)", total_kinetic_energy);
				ImGui::Text("Total momentum: (x: %.4e; y: %.4e) (kg * m/s)", total_momentum.x, total_momentum.y);
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
		glUniform2fv(resolution_location, 1, glm::value_ptr(resolution));

		std::vector<glm::vec2> pending_impulses(particles.num_active_particles, glm::vec2(0.0f));
		for (size_t i = 0; i < particles.num_active_particles; ++i)
		{
			auto& p1{ particles.positions[i] };
			auto& v1{ particles.velocities[i] };
			auto& r1{ particles.radii[i] };
			auto& m1{ particles.mass[i] };

			for (size_t j = i + 1; j < particles.num_active_particles; ++j)
			{
				auto& p2{ particles.positions[j] };
				auto& v2{ particles.velocities[j] };
				auto& r2{ particles.radii[j] };
				auto& m2{ particles.mass[j] };

				const auto pos_diff = p1 - p2;
				const auto min_dist = r1 + r2;
				const auto dist = glm::length(pos_diff);

				if (dist > min_dist || dist == 0.0f)
					continue;

				const auto vel_diff = v1 - v2;
				const auto dot_prod = glm::dot(vel_diff, pos_diff);
				if (dot_prod >= 0.0f)
					continue;

				const auto dist_sq = dist * dist;
				const auto total_mass = m1 + m2;
				const auto impulse_scalar = dot_prod / dist_sq;
				const glm::vec2 impulse = impulse_scalar * pos_diff;

				pending_impulses[i] -= (2.0f * m2 / total_mass) * impulse;
				pending_impulses[j] += (2.0f * m1 / total_mass) * impulse;
			}
		}

		glm::vec2 current_total_momentum{ 0 };
		float current_total_kinetic_energy = 0.0f;

		const float aspect_ratio = static_cast<float>(framebufferHeight) / static_cast<float>(framebufferWidth);
		for (size_t i = 0; i < particles.num_active_particles; ++i)
		{
			auto& p1{ particles.positions[i] };
			auto& v1{ particles.velocities[i] };
			auto& r1{ particles.radii[i] };
			auto& m1{ particles.mass[i] };

			v1 += pending_impulses[i];
			p1 += v1 * dt;

			const glm::vec2 limit(1.0f / aspect_ratio, 1.0f);
			if (p1.x - r1 < -limit.x)
			{
				p1.x = -limit.x + r1;
				v1.x *= -1.0f;
			}
			else if (p1.x + r1 > limit.x)
			{
				p1.x = limit.x - r1;
				v1.x *= -1.0f;
			}

			if (p1.y - r1 < -limit.y)
			{
				p1.y = -limit.y + r1;
				v1.y *= -1.0f;
			}
			else if (p1.y + r1 > limit.y)
			{
				p1.y = limit.y - r1;
				v1.y *= -1.0f;
			}

			const float velocity_sq = glm::dot(v1, v1);
			current_total_kinetic_energy += m1 * velocity_sq / 2.0f;
			current_total_momentum += v1 * m1;
		}

		total_kinetic_energy = current_total_kinetic_energy;
		avg_kinetic_energy = particles.num_active_particles != 0
			? total_kinetic_energy / particles.num_active_particles
			: 0.0f;
		total_momentum = current_total_momentum;

		temperature.update_from_kinetic_energy(avg_kinetic_energy);

		radius_vbo.SubData(0, particles.num_active_particles * sizeof(particles.radii.at(0)), particles.radii.data());
		center_vbo.SubData(0, particles.num_active_particles * sizeof(particles.positions.at(0)), particles.positions.data());
		color_vbo.SubData(0, particles.num_active_particles * sizeof(particles.colors.at(0)), particles.colors.data());

		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, particles.num_active_particles);

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