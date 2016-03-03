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