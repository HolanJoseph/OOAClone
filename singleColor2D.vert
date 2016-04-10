#version 430 core

uniform mat3 PCM;

layout(location = 0) in vec4 vPosition;

void main()
{
	vec3 pos = PCM * vec3(vPosition.x, vPosition.y, 1.0f);
	gl_Position = vec4(pos.x, pos.y, 0.0f, 1.0f);
}