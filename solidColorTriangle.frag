#version 430 core

uniform vec4 triangleColor;

out vec4 fColor;

void main() 
{
	fColor = triangleColor;
}