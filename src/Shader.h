#pragma once
#include <string>
#include <string_view>
#include <exception>
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <glad/gl.h>

enum class ShaderType
{
	VertexShader = GL_VERTEX_SHADER,
	FragmentShader = GL_FRAGMENT_SHADER
};

class Shader
{
public:
	Shader(ShaderType type, const std::string_view source);
	~Shader();

	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

	Shader(Shader&& other) noexcept;
	Shader& operator=(Shader&& other) noexcept;

	GLuint Id() const noexcept;
	ShaderType Type() const noexcept;

	static std::string LoadFile(const std::filesystem::path& filepath);

private:
	GLuint m_Id;
	ShaderType m_Type;
};

inline Shader::Shader(ShaderType type, const std::string_view source)
	: m_Type(type)
{
	m_Id = glCreateShader(static_cast<GLenum>(type));

	const GLchar* sourceData = source.data();
	const GLint sourceLength = static_cast<GLint>(source.size());
	glShaderSource(m_Id, 1, &sourceData, &sourceLength);

	glCompileShader(m_Id);

	int success;
	glGetShaderiv(m_Id, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		GLint logLength = 0;
		glGetShaderiv(m_Id, GL_INFO_LOG_LENGTH, &logLength);

		std::string infoLog(logLength, '\0');
		glGetShaderInfoLog(m_Id, logLength, nullptr, infoLog.data());
		throw std::runtime_error("GL: Shader compilation failed: " + std::string(infoLog));
	}
}

inline Shader::~Shader()
{
	glDeleteShader(m_Id);
}

inline Shader::Shader(Shader&& other) noexcept
	: m_Id(other.m_Id),
	m_Type(other.m_Type)
{
	other.m_Id = 0;
}

inline Shader& Shader::operator=(Shader&& other) noexcept
{
	if (this == &other)
	{
		return *this;
	}

	if (m_Id != 0)
	{
		glDeleteShader(m_Id);
	}

	m_Id = other.m_Id;
	m_Type = other.m_Type;

	other.m_Id = 0;
}

inline GLuint Shader::Id() const noexcept
{
	return m_Id;
}

inline ShaderType Shader::Type() const noexcept
{
	return m_Type;
}

inline std::string Shader::LoadFile(const std::filesystem::path& filepath)
{
	std::ifstream file;
	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	
	try
	{
		file.open(filepath);
		
		std::stringstream source;
		source << file.rdbuf();
		
		return source.str();
	}
	catch (const std::ifstream::failure& e)
	{
		throw std::runtime_error("GL: Failed to load shader source from " + filepath.string() + ": " + e.what());
	}
}
