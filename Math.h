#pragma once

#include "glm/glm.hpp"



#define PI 3.141592653589793238462643383279502884197169399375105820974944592307816406286f


using glm::ivec2;
using glm::vec2;
using glm::vec3;
using glm::dvec3;
using glm::vec4;
using glm::mat2;
using glm::mat3;
using glm::mat4;
using glm::dot;
using glm::length;
using glm::normalize;
using glm::inverse;



inline F32 DegreesToRadians(F32 angleInDegrees)
{
	F32 result = angleInDegrees * (PI/180.0f);
	return result;
}

/*
 * NOTE: GLM Matrices are specified as column matrices
 */
inline mat3 TranslationMatrix(vec2 translation)
{
	mat3 result = mat3(1, 0, 0, 0, 1, 0, translation.x, translation.y, 1);
	return result;
}

inline mat4 TranslationMatrix(vec3 translation)
{
	mat4 result = mat4(1,0,0,0,   0,1,0,0,   0,0,1,0,   translation.x,translation.y,translation.z,1);
	return result;
}

inline mat3 ScaleMatrix(vec2 scale)
{
	mat3 result = mat3(scale.x, 0, 0, 0, scale.y, 0, 0, 0, 1);
	return result;
}

inline mat4 ScaleMatrix(vec3 scale)
{
	mat4 result = mat4(scale.x,0,0,0,   0,scale.y,0,0,   0,0,scale.z,0,   0,0,0,1);
	return result;
}

inline mat4 RotationMatrix_X(F32 angleInDegrees)
{
	F32 angleInRadians = DegreesToRadians(angleInDegrees);
	mat4 result = mat4(1,0,0,0,   0,cos(angleInRadians),sin(angleInRadians),0,   0,-sin(angleInRadians),cos(angleInRadians),0,   0,0,0,1);
	return result;
}

inline mat4 RotationMatrix_Y(F32 angleInDegrees)
{
	F32 angleInRadians = DegreesToRadians(angleInDegrees);
	mat4 result = mat4(cos(angleInRadians),0,-sin(angleInRadians),0,   0,1,0,0,   sin(angleInRadians),0,cos(angleInRadians),0,   0,0,0,1);
	return result;
}

inline mat4 RotationMatrix_Z(F32 angleInDegrees)
{
	F32 anglueInRadians = DegreesToRadians(angleInDegrees);
	mat4 result = mat4(cos(anglueInRadians),sin(anglueInRadians),0,0,   -sin(anglueInRadians),cos(anglueInRadians),0,0,   0,0,1,0,   0,0,0,1);
	return result;
}

inline mat3 RotationMatrix_2D(F32 angleInDegrees)
{
	F32 anglueInRadians = DegreesToRadians(angleInDegrees);
	mat3 result = mat3(cos(anglueInRadians), sin(anglueInRadians), 0, -sin(anglueInRadians), cos(anglueInRadians), 0, 0, 0, 1);
	return result;
}

inline mat2 RotationMatrix2x2_2D(F32 angleInDegrees)
{
	F32 anglueInRadians = DegreesToRadians(angleInDegrees);
	mat2 result = mat2(cos(anglueInRadians), sin(anglueInRadians), -sin(anglueInRadians), cos(anglueInRadians));
	return result;
}

inline vec2 Perpendicular_2D(vec2 v)
{
	vec2 result;

	// NOTE: This matrix is a 2D rotation by -90 degrees
	result = mat2(0.0f, -1.0f, 1.0f, 0.0f) * v;

	return result;
}

inline vec2 VVM(vec2 lhs, vec2 rhs)
{
	vec2 result;

	result.x = lhs.x * rhs.x;
	result.y = lhs.y * rhs.y;

	return result;
}