#version 430 core

uniform sampler2D spriteSampler;

in vec2 textureCoordinates;

out vec4 fColor;

void main() 
{
	fColor = texture(spriteSampler, textureCoordinates);
}