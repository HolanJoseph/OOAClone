#pragma once
#include "glm/glm.hpp"
//#include "glm/vec2.hpp"

#define PI 3.141592653589793238462643383279502884197169399375105820974944592307816406286f

using glm::vec2;
using glm::vec3;
using glm::dvec3;
using glm::vec4;
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

inline mat4 TranslationMatrix(vec3 translation)
{
	mat4 result = mat4(1,0,0,0,   0,1,0,0,   0,0,1,0,   translation.x,translation.y,translation.z,1);
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