#pragma once
#include <glad/gl.h>

class VertexArray
{
public:
	VertexArray();
	~VertexArray();

	VertexArray(const VertexArray&) = delete;
	VertexArray& operator=(const VertexArray&) = delete;

	VertexArray(VertexArray&& other) noexcept;
	VertexArray& operator=(VertexArray&& other) noexcept;

	GLuint Id() const noexcept;

	void Bind() const noexcept;
	void EnableAttribArray(GLuint index) const noexcept;
	void DisableAttribArray(GLuint index) const noexcept;
	void SetAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer) const noexcept;

	static void Unbind() noexcept;

private:
	GLuint m_Id = 0;
};

inline VertexArray::VertexArray()
{
	glGenVertexArrays(1, &m_Id);
}

inline VertexArray::~VertexArray()
{
	glDeleteVertexArrays(1, &m_Id);
}

inline VertexArray::VertexArray(VertexArray&& other) noexcept
	: m_Id(other.m_Id)
{
	other.m_Id = 0;
}

inline VertexArray& VertexArray::operator=(VertexArray&& other) noexcept
{
	if (this == &other)
	{
		return *this;
	}

	if (m_Id != 0)
	{
		glDeleteVertexArrays(1, &m_Id);
	}

	m_Id = other.m_Id;

	other.m_Id = 0;
	return *this;
}

inline GLuint VertexArray::Id() const noexcept
{
	return m_Id;
}

inline void VertexArray::Bind() const noexcept
{
	glBindVertexArray(m_Id);
}

inline void VertexArray::EnableAttribArray(GLuint index) const noexcept
{
	Bind();
	glEnableVertexAttribArray(index);
}

inline void VertexArray::DisableAttribArray(GLuint index) const noexcept
{
	Bind();
	glDisableVertexAttribArray(index);
}

inline void VertexArray::SetAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer) const noexcept
{
	Bind();
	glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}

inline void VertexArray::Unbind() noexcept
{
	glBindVertexArray(0);
}
