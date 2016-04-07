#pragma once

#include "Types.h"
#include "Math.h"

struct Transform
{
	vec2 position;
	F32  rotationAngle;
	F32  scale;

	Transform()
	{
		position = vec2(0, 0);
		rotationAngle = 0;
		scale = 1;
	}

	mat3 LocalToWorldTransform()
	{
		mat3 result = TranslationMatrix(position) * RotationMatrix_2D(rotationAngle) * ScaleMatrix(vec2(scale, scale));
		return result;
	}

	mat3 WorldToLocalTransform()
	{
		mat3 result = inverse(LocalToWorldTransform());
		return result;
	}
};