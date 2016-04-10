#version 430 core

uniform vec4 Color;

in vec4 poscolor;

out vec4 fColor;

void main() 
{
	fColor = Color;
}