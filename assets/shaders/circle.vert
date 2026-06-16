#version 460 core

layout (location = 0) in vec3 aPos;

out vec2 localPos;

uniform vec3 iResolution;
uniform float uRadius;
uniform vec2 uPosition;

void main()
{
	float aspectRatio = iResolution.y / iResolution.x;
	vec2 scaledPos = aPos.xy * uRadius;
	vec2 finalPos = vec2(scaledPos.x * aspectRatio, scaledPos.y) + uPosition;
	gl_Position = vec4(finalPos.x, finalPos.y, aPos.z, 1.0);
	localPos = aPos.xy;
}