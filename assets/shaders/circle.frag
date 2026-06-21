#version 460 core

in float radius;
in vec2 tex_coords;
in vec3 color;

uniform vec2 u_resolution;

out vec4 frag_color;

void main()
{
	float fade = 2.0 / (u_resolution.y * radius);
	float dist = length(tex_coords);
	float alpha = 1.0 - smoothstep(1.0 - fade, 1.0, dist);
	
	frag_color = vec4(color, alpha);
}