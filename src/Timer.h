#pragma once
#include <GLFW/glfw3.h>
#include "Log.h"

class Timer
{
public:
	Timer() { m_frame_time = glfwGetTime(); }
	~Timer() = default;

	[[nodiscard]] static float time() noexcept { return static_cast<float>(glfwGetTime()); }
	[[nodiscard]] static double time_precise() noexcept { return glfwGetTime(); }

	[[nodiscard]] float dt() const noexcept { return static_cast<float>(m_dt); }
	[[nodiscard]] double dt_precise() const noexcept { return m_dt; }
	[[nodiscard]] size_t frame_count() const noexcept { return m_frame_count; }
	[[nodiscard]] size_t fps() const noexcept { return m_fps; }

	void start_frame() noexcept { m_last_frame_time = m_frame_time; }

	void end_frame() noexcept
	{
		m_frame_count++;
		m_frame_time = glfwGetTime();
		m_dt = m_frame_time - m_last_frame_time;

		m_cumulative_dt += m_dt;
		m_cumulative_frame_count++;
		if (m_cumulative_dt >= 1.0)
		{
			m_fps = m_cumulative_frame_count;
			m_cumulative_frame_count = 0;
			m_cumulative_dt -= 1.0;
		}
	}

private:
	double m_dt{ 0 };
	double m_frame_time{ 0 };
	double m_last_frame_time{ 0 };
	size_t m_frame_count{ 0 };
	double m_cumulative_dt{ 0 };
	size_t m_cumulative_frame_count{ 0 };
	size_t m_fps{ 0 };
};
