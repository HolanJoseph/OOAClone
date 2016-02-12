#version 430 core

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec2 vTextureCoordinates;

out vec2 textureCoordinates;

void main()
{
	gl_Position = vPosition;
	textureCoordinates = vTextureCoordinates;
}