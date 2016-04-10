#version 430 core

uniform mat4 PCM;

layout(location = 0) in vec4 vPosition;

void main()
{
	vec4 pos = PCM * vec4(vPosition.x, vPosition.y, vPosition.z, 1.0f);
	gl_Position = pos;
}