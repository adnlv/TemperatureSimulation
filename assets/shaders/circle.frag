#version 460 core

in vec2 LocalPos;
out vec4 FragColor;
uniform float uRadius;
uniform vec2 uResolution;
uniform vec3 uColor;

void main()
{
	float fade = 2.0 / (uResolution.y * uRadius);
	float distance = length(LocalPos);
	float circleAlpha = 1.0 - smoothstep(1.0 - fade, 1.0, distance);
	FragColor = vec4(uColor, circleAlpha);
}