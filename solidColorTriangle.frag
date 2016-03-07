#version 430 core

uniform vec4 triangleColor;

in vec4 poscolor;

out vec4 fColor;

void main() 
{
	fColor = triangleColor;
}