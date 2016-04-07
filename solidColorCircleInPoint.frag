#version 430 core

uniform vec4 Color;

out vec4 fColor;

void main() 
{
	vec2 coord = gl_PointCoord - vec2(0.5, 0.5);
	float lengthSquared = dot(coord, coord);

	if (lengthSquared > .25)
	{
		discard;
	}

	fColor = Color;
}