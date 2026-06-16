#include <iostream>
#include <exception>
#include <string>
#include <string_view>
#include <vector>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

#include "Shader.h"

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

	Update();
}

Application::~Application()
{
	GLFWDestroyWindow();
	GLFWTerminate();
}

void Application::GLFWInit()
{
	if (glfwInit() != GLFW_TRUE)
	{
		throw std::runtime_error("GLFW: Initialization failed");
	}
	spdlog::info("GLFW: Initialized");

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
}

void Application::GLFWTerminate() noexcept
{
	glfwTerminate();
	spdlog::info("GLFW: Terminated");
}

void Application::GLFWCreateWindow()
{
	m_Window = glfwCreateWindow(960, 540, "Termperature Simulation", nullptr, nullptr);
	if (m_Window == nullptr)
	{
		throw std::runtime_error("GLFW: Window creation failed");
	}
	spdlog::info("GLFW: Window created");

	glfwMakeContextCurrent(m_Window);
	glfwSetWindowUserPointer(m_Window, this);
}

void Application::GLFWDestroyWindow() noexcept
{
	glfwDestroyWindow(m_Window);
	spdlog::info("GLFW: Window destroyed");
}

void Application::GLFWSetFramebufferSizeCallback()
{
	glfwSetFramebufferSizeCallback(m_Window,
		[](GLFWwindow* window, int width, int height)
		{
			glViewport(0, 0, width, height);
			spdlog::info("GL: Viewport resolution changed | Width: {} px, Height: {} px", width, height);
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
				spdlog::info("GL: Key action | Key: {:#x}, Scancode: {:#x}, Action: {}, Mods: {:#x}", key, scancode, action, mods);
			}
		});
}

void Application::GLADLoadGL()
{
	if (!gladLoadGL(glfwGetProcAddress))
	{
		throw std::runtime_error("GLAD: GL loading failed");
	}
	spdlog::info("GLAD: GL loaded");

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
				spdlog::level::level_enum level = spdlog::level::warn;
				switch (type) {
				case GL_DEBUG_TYPE_ERROR:
					type_str = "ERROR";
					level = spdlog::level::err;
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

				spdlog::log(level, "GL: {} severity {} message from {} ({}): {}", severity_str, type_str, source_str, id, message);
			}, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
}

void Application::Update()
{
	struct Circle {
		glm::vec2 position;
		float radius;
		float speed;
		glm::vec3 color;
	};

	std::vector<Circle> circles = {
		{{ 0.0f,  0.0f}, 0.3f,  1.0f, {0.85f, 0.35f, 0.20f}}, // Orange
		{{-0.6f,  0.5f}, 0.15f, 2.5f, {0.20f, 0.85f, 0.35f}}, // Green
		{{ 0.6f,  0.5f}, 0.15f, 1.5f, {0.20f, 0.35f, 0.85f}}, // Blue
		{{ 0.0f, -0.6f}, 0.2f,  3.0f, {0.85f, 0.85f, 0.20f}}  // Yellow
	};

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

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices.at(0)), vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(vertices.at(0)), nullptr);
	glEnableVertexAttribArray(0);

	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices.at(0)), indices.data(), GL_STATIC_DRAW);

	auto vertexShaderSource = Shader::LoadFile("assets/shaders/circle.vert");
	Shader vertexShader(ShaderType::VertexShader, vertexShaderSource);

	auto fragmentShaderSource = Shader::LoadFile("assets/shaders/circle.frag");
	Shader fragmentShader(ShaderType::FragmentShader, fragmentShaderSource);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader.Id());
	glAttachShader(shaderProgram, fragmentShader.Id());
	glLinkProgram(shaderProgram);
	glDetachShader(shaderProgram, vertexShader.Id());
	glDetachShader(shaderProgram, fragmentShader.Id());

	GLint resolutionLocation = glGetUniformLocation(shaderProgram, "iResolution");
	GLint radiusLocation = glGetUniformLocation(shaderProgram, "uRadius");
	GLint positionLocation = glGetUniformLocation(shaderProgram, "uPosition");
	GLint colorLocation = glGetUniformLocation(shaderProgram, "uColor");
	while (!glfwWindowShouldClose(m_Window))
	{
		int fbW, fbH;
		glfwGetFramebufferSize(m_Window, &fbW, &fbH);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glUseProgram(shaderProgram);

		glm::vec3 resolution(fbW, fbH, 0);
		glUniform3fv(resolutionLocation, 1, glm::value_ptr(resolution));

		float time = static_cast<float>(glfwGetTime());
		for (Circle& circle : circles)
		{
			circle.position.x = sin(time * circle.speed) * (1.0f - circle.radius);
			circle.position.y = cos(time * circle.speed) * (1.0f - circle.radius);

			glUniform2fv(positionLocation, 1, glm::value_ptr(circle.position));
			glUniform3fv(colorLocation, 1, glm::value_ptr(circle.color));
			glUniform1f(radiusLocation, circle.radius);

			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}

		glfwSwapBuffers(m_Window);
		glfwPollEvents();
	}
}

int main()
{
	Application app;
	return 0;
}