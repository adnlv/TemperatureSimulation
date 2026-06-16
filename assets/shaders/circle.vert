#version 460 core

layout (location = 0) in vec3 aQuad;

out vec2 localPos;

uniform vec2 uResolution;
uniform vec2 uPosition;
uniform float uRadius;

void main()
{
	float aspectRatio = uResolution.y / uResolution.x;
	vec2 scaledPos = aQuad.xy * uRadius;
	vec2 finalPos = vec2(scaledPos.x * aspectRatio, scaledPos.y) + uPosition;
	gl_Position = vec4(finalPos.x, finalPos.y, aQuad.z, 1.0);
	localPos = aQuad.xy;
}