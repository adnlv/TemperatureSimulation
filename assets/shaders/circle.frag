#version 460 core

in vec2 localPos;

out vec4 fragColor;

uniform vec3 uResolution;
uniform float uRadius;
uniform vec3 uColor;

void main()
{
	float fade = 2.0 / (uResolution.y * uRadius);
	float distance = length(localPos);
	float circleAlpha = 1.0 - smoothstep(1.0 - fade, 1.0, distance);
	fragColor = vec4(uColor, circleAlpha);
}