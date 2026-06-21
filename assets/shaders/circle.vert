#version 460 core

layout (location = 0) in float a_radius;
layout (location = 1) in vec2 a_center;
layout (location = 2) in vec3 a_pos;
layout (location = 3) in vec3 a_color;

uniform vec2 u_resolution;

out float radius;
out vec2 tex_coords;
out vec3 color;

void main()
{
	float inv_aspect_ratio = u_resolution.y / u_resolution.x;
	vec2 local_pos = a_pos.xy * a_radius;
	vec2 clip_pos = vec2((local_pos.x + a_center.x) * inv_aspect_ratio, local_pos.y + a_center.y);
	gl_Position = vec4(clip_pos.x, clip_pos.y, a_pos.z, 1.0);
	
	tex_coords = a_pos.xy;
	radius = a_radius;
	color = a_color;
}