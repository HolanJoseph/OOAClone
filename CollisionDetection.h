#pragma once

#include "Types.h"
#include "Math.h"

#include "DebugAPI.h"

struct Rectangle
{
	mat3 transform;

	vec2 origin; // NOTE: this is in "model" space
	vec2 halfDim;
};

inline vec2 Support(Rectangle* A, vec2 direction)
{
	vec3 AiPoints[4] = {
		A->transform * vec3(A->origin.x - A->halfDim.x, A->origin.y - A->halfDim.y, 1.0f),
		A->transform * vec3(A->origin.x - A->halfDim.x, A->origin.y + A->halfDim.y, 1.0f),
		A->transform * vec3(A->origin.x + A->halfDim.x, A->origin.y - A->halfDim.y, 1.0f),
		A->transform * vec3(A->origin.x + A->halfDim.x, A->origin.y + A->halfDim.y, 1.0f)
	};
	F32 AiDots[4] = {
		dot(direction, vec2(AiPoints[0].x, AiPoints[0].y)),
		dot(direction, vec2(AiPoints[1].x, AiPoints[1].y)),
		dot(direction, vec2(AiPoints[2].x, AiPoints[2].y)),
		dot(direction, vec2(AiPoints[3].x, AiPoints[3].y))
	};

	U32 maxPositionAi = 0;
	F32 maxDotAi = -1000000.0f;
	for (U32 i = 0; i < 4; ++i)
	{
		if (AiDots[i] > maxDotAi)
		{
			maxPositionAi = i;
			maxDotAi = AiDots[i];
		}
	}

	vec2 maxA = vec2(AiPoints[maxPositionAi].x, AiPoints[maxPositionAi].y);

	return maxA;
}



struct Circle
{
	vec2 origin;
	F32  radius;
};

inline vec2 Support(Circle* A, vec2 direction)
{
	direction = normalize(direction);
	vec2 maxA = A->origin + (A->radius * direction);

	return maxA;
}



struct Triangle
{
	mat3 transform;

	vec2 origin; // NOTE: this is in "model" space
	vec2 points[3];
};

inline vec2 Support(Triangle* A, vec2 direction)
{
	vec3 AiPoints[3] = {
		A->transform * vec3(A->origin.x + A->points[0].x, A->origin.y + A->points[0].y, 1.0f),
		A->transform * vec3(A->origin.x + A->points[1].x, A->origin.y + A->points[1].y, 1.0f),
		A->transform * vec3(A->origin.x + A->points[2].x, A->origin.y + A->points[2].y, 1.0f)
	};
	F32 AiDots[3] = {
		dot(direction, vec2(AiPoints[0].x, AiPoints[0].y)),
		dot(direction, vec2(AiPoints[1].x, AiPoints[1].y)),
		dot(direction, vec2(AiPoints[2].x, AiPoints[2].y))
	};

	U32 maxPositionAi = 0;
	F32 maxDotAi = -1000000.0f;
	for (U32 i = 0; i < 3; ++i)
	{
		if (AiDots[i] > maxDotAi)
		{
			maxPositionAi = i;
			maxDotAi = AiDots[i];
		}
	}

	vec2 maxA = vec2(AiPoints[maxPositionAi].x, AiPoints[maxPositionAi].y);

	return maxA;
}



struct OrientedBoundingBox
{
	mat4 transform;

	vec3 origin; // NOTE: this is in "model" space
	vec3 halfDim;
};

inline vec3 Support(OrientedBoundingBox* A, vec3 direction)
{
	vec4 AiPoints[8] = {
		A->transform * vec4(A->origin.x - A->halfDim.x, A->origin.y - A->halfDim.y, A->origin.z + A->halfDim.z, 1.0f),
		A->transform * vec4(A->origin.x - A->halfDim.x, A->origin.y + A->halfDim.y, A->origin.z + A->halfDim.z, 1.0f),
		A->transform * vec4(A->origin.x + A->halfDim.x, A->origin.y - A->halfDim.y, A->origin.z + A->halfDim.z, 1.0f),
		A->transform * vec4(A->origin.x + A->halfDim.x, A->origin.y + A->halfDim.y, A->origin.z + A->halfDim.z, 1.0f),
		A->transform * vec4(A->origin.x - A->halfDim.x, A->origin.y - A->halfDim.y, A->origin.z - A->halfDim.z, 1.0f),
		A->transform * vec4(A->origin.x - A->halfDim.x, A->origin.y + A->halfDim.y, A->origin.z - A->halfDim.z, 1.0f),
		A->transform * vec4(A->origin.x + A->halfDim.x, A->origin.y - A->halfDim.y, A->origin.z - A->halfDim.z, 1.0f),
		A->transform * vec4(A->origin.x + A->halfDim.x, A->origin.y + A->halfDim.y, A->origin.z - A->halfDim.z, 1.0f),
	};
	F32 AiDots[8] = {
		dot(direction, vec3(AiPoints[0].x, AiPoints[0].y, AiPoints[0].z)),
		dot(direction, vec3(AiPoints[1].x, AiPoints[1].y, AiPoints[1].z)),
		dot(direction, vec3(AiPoints[2].x, AiPoints[2].y, AiPoints[2].z)),
		dot(direction, vec3(AiPoints[3].x, AiPoints[3].y, AiPoints[3].z)),
		dot(direction, vec3(AiPoints[4].x, AiPoints[4].y, AiPoints[4].z)),
		dot(direction, vec3(AiPoints[5].x, AiPoints[5].y, AiPoints[5].z)),
		dot(direction, vec3(AiPoints[6].x, AiPoints[6].y, AiPoints[6].z)),
		dot(direction, vec3(AiPoints[7].x, AiPoints[7].y, AiPoints[7].z))
	};

	U32 maxPositionAi = 0;
	F32 maxDotAi = -1000000.0f;
	for (U32 i = 0; i < 8; ++i)
	{
		if (AiDots[i] > maxDotAi)
		{
			maxPositionAi = i;
			maxDotAi = AiDots[i];
		}
	}

	vec3 maxA = vec3(AiPoints[maxPositionAi].x, AiPoints[maxPositionAi].y, AiPoints[maxPositionAi].z);

	return maxA;
}



struct Sphere
{
	vec3 origin;
	F32 radius;
};

inline vec3 Support(Sphere* A, vec3 direction)
{
	direction = normalize(direction);
	vec3 maxA = A->origin + (A->radius * direction);

	return maxA;
}



struct Capsule
{
	mat4 transform;

	vec3 points[2]; // NOTE: these are in "model" space
	F32 radius;
};

inline vec3 Support(Capsule* A, vec3 direction)
{
	vec4 AiPoints[2] = {
		A->transform * vec4(A->points[0].x, A->points[0].y, A->points[0].z, 1.0f),
		A->transform * vec4(A->points[1].x, A->points[1].y, A->points[1].z, 1.0f)
	};
	F32 AiDots[2] = {
		dot(direction, vec3(AiPoints[0])),
		dot(direction, vec3(AiPoints[1]))
	};

	U32 maxPositionAi = 0;
	F32 maxDotAi = -1000000.0f;
	for (U32 i = 0; i < 3; ++i)
	{
		if (AiDots[i] > maxDotAi)
		{
			maxPositionAi = i;
			maxDotAi = AiDots[i];
		}
	}

	vec3 maxLinePointA = vec3(AiPoints[maxPositionAi].x, AiPoints[maxPositionAi].y, AiPoints[maxPositionAi].z);
	vec3 maxA = maxLinePointA + (normalize(direction) * A->radius);

	return maxA;
}



/*
2D COMBO SUPPORTS
*/
template<typename S1, typename S2>
inline vec2 Support(S1* A, S2* B, vec2* direction)
{
	vec2 maxA = Support(A, *direction);

	// find the point in B that has the largest value with dot(Bj, Direction)
	vec2 maxB = Support(B, -*direction);
	vec2 result = maxA - maxB;
	return result;
}

// inline vec2 Support(Rectangle* A, Rectangle* B, vec2* direction)
// {
// 	// find the point in A that has the largest value with dot(Ai, Direction)
// 	vec2 maxA = Support(A, *direction);
// 
// 	// find the point in B that has the largest value with dot(Bj, Direction)
// 	vec2 maxB = Support(B, -*direction);
// 	vec2 result = maxA - maxB;
// 	return result;
// }
// 
// inline vec2 Support(Rectangle* A, Circle* B, vec2* direction)
// {
// 	// find the point in A that has the largest value with dot(Ai, Direction)
// 	vec2 maxA = Support(A, *direction);
// 
// 	// find the point in B that has the largest value with dot(Bj, Direction)
// 	vec2 maxB = Support(B, -*direction);
// 	vec2 result = maxA - maxB;
// 	return result;
// }
// 
// inline vec2 Support(Rectangle* A, Triangle* B, vec2* direction)
// {
// 	// find the point in A that has the largest value with dot(Ai, Direction)
// 	vec2 maxA = Support(A, *direction);
// 
// 	// find the point in B that has the largest value with dot(Bj, Direction)
// 	vec2 maxB = Support(B, -*direction);
// 	vec2 result = maxA - maxB;
// 	return result;
// }
// 
// inline vec2 Support(Circle* A, Circle* B, vec2* direction)
// {
// 	// find the point in A that has the largest value with dot(Ai, Direction)
// 	vec2 maxA = Support(A, *direction);
// 
// 	// find the point in B that has the largest value with dot(Bj, Direction)
// 	vec2 maxB = Support(B, -*direction);
// 	vec2 result = maxA - maxB;
// 	return result;
// }
// 
// inline vec2 Support(Circle* A, Rectangle* B, vec2* direction)
// {
// 	// find the point in A that has the largest value with dot(Ai, Direction)
// 	vec2 maxA = Support(A, *direction);
// 
// 	// find the point in B that has the largest value with dot(Bj, Direction)
// 	vec2 maxB = Support(B, -*direction);
// 	vec2 result = maxA - maxB;
// 	return result;
// }
// 
// inline vec2 Support(Circle* A, Triangle* B, vec2* direction)
// {
// 	// find the point in A that has the largest value with dot(Ai, Direction)
// 	vec2 maxA = Support(A, *direction);
// 
// 	// find the point in B that has the largest value with dot(Bj, Direction)
// 	vec2 maxB = Support(B, -*direction);
// 	vec2 result = maxA - maxB;
// 	return result;
// }
// 
// inline vec2 Support(Triangle* A, Triangle* B, vec2* direction)
// {
// 	// find the point in A that has the largest value with dot(Ai, Direction)
// 	vec2 maxA = Support(A, *direction);
// 
// 	// find the point in B that has the largest value with dot(Bj, Direction)
// 	vec2 maxB = Support(B, -*direction);
// 	vec2 result = maxA - maxB;
// 	return result;
// }
// 
// inline vec2 Support(Triangle* A, Rectangle* B, vec2* direction)
// {
// 	// find the point in A that has the largest value with dot(Ai, Direction)
// 	vec2 maxA = Support(A, *direction);
// 
// 	// find the point in B that has the largest value with dot(Bj, Direction)
// 	vec2 maxB = Support(B, -*direction);
// 	vec2 result = maxA - maxB;
// 	return result;
// }
// 
// inline vec2 Support(Triangle* A, Circle* B, vec2* direction)
// {
// 	// find the point in A that has the largest value with dot(Ai, Direction)
// 	vec2 maxA = Support(A, *direction);
// 
// 	// find the point in B that has the largest value with dot(Bj, Direction)
// 	vec2 maxB = Support(B, -*direction);
// 	vec2 result = maxA - maxB;
// 	return result;
// }



/*
3D COMBO SUPPORTS
*/
template<typename S1, typename S2>
inline vec3 Support(S1* A, S2* B, vec3* direction)
{
	// find the point in A that has the largest value with dot(Ai, Direction)
	vec3 maxA = Support(A, *direction);

	// find the point in B that has the largest value with dot(Bj, Direction)
	vec3 maxB = Support(B, -*direction);
	vec3 result = maxA - maxB;
	return result;
}

// inline vec3 Support(OrientedBoundingBox* A, OrientedBoundingBox* B, vec3* direction)
// {
// 	// find the point in A that has the largest value with dot(Ai, Direction)
// 	vec3 maxA = Support(A, *direction);
// 
// 	// find the point in B that has the largest value with dot(Bj, Direction)
// 	vec3 maxB = Support(B, -*direction);
// 	vec3 result = maxA - maxB;
// 	return result;
// }
// 
// inline vec3 Support(OrientedBoundingBox* A, Sphere* B, vec3* direction)
// {
// 	// find the point in A that has the largest value with dot(Ai, Direction)
// 	vec3 maxA = Support(A, *direction);
// 
// 	// find the point in B that has the largest value with dot(Bj, Direction)
// 	vec3 maxB = Support(B, -*direction);
// 	vec3 result = maxA - maxB;
// 	return result;
// }
// 
// inline vec3 Support(OrientedBoundingBox* A, Capsule* B, vec3* direction)
// {
// 	// find the point in A that has the largest value with dot(Ai, Direction)
// 	vec3 maxA = Support(A, *direction);
// 
// 	// find the point in B that has the largest value with dot(Bj, Direction)
// 	vec3 maxB = Support(B, -*direction);
// 	vec3 result = maxA - maxB;
// 	return result;
// }
// 
// inline vec3 Support(Sphere* A, Sphere* B, vec3* direction)
// {
// 	// find the point in A that has the largest value with dot(Ai, Direction)
// 	vec3 maxA = Support(A, *direction);
// 
// 	// find the point in B that has the largest value with dot(Bj, Direction)
// 	vec3 maxB = Support(B, -*direction);
// 	vec3 result = maxA - maxB;
// 	return result;
// }
// 
// inline vec3 Support(Sphere* A, OrientedBoundingBox* B, vec3* direction)
// {
// 	// find the point in A that has the largest value with dot(Ai, Direction)
// 	vec3 maxA = Support(A, *direction);
// 
// 	// find the point in B that has the largest value with dot(Bj, Direction)
// 	vec3 maxB = Support(B, -*direction);
// 	vec3 result = maxA - maxB;
// 	return result;
// }
// 
// inline vec3 Support(Sphere* A, Capsule* B, vec3* direction)
// {
// 	// find the point in A that has the largest value with dot(Ai, Direction)
// 	vec3 maxA = Support(A, *direction);
// 
// 	// find the point in B that has the largest value with dot(Bj, Direction)
// 	vec3 maxB = Support(B, -*direction);
// 	vec3 result = maxA - maxB;
// 	return result;
// }
// 
// inline vec3 Support(Capsule* A, Capsule* B, vec3* direction)
// {
// 	// find the point in A that has the largest value with dot(Ai, Direction)
// 	vec3 maxA = Support(A, *direction);
// 
// 	// find the point in B that has the largest value with dot(Bj, Direction)
// 	vec3 maxB = Support(B, -*direction);
// 	vec3 result = maxA - maxB;
// 	return result;
// }
// 
// inline vec3 Support(Capsule* A, OrientedBoundingBox* B, vec3* direction)
// {
// 	// find the point in A that has the largest value with dot(Ai, Direction)
// 	vec3 maxA = Support(A, *direction);
// 
// 	// find the point in B that has the largest value with dot(Bj, Direction)
// 	vec3 maxB = Support(B, -*direction);
// 	vec3 result = maxA - maxB;
// 	return result;
// }
// 
// inline vec3 Support(Capsule* A, Sphere* B, vec3* direction)
// {
// 	// find the point in A that has the largest value with dot(Ai, Direction)
// 	vec3 maxA = Support(A, *direction);
// 
// 	// find the point in B that has the largest value with dot(Bj, Direction)
// 	vec3 maxB = Support(B, -*direction);
// 	vec3 result = maxA - maxB;
// 	return result;
// }






enum SimplexType
{
	Simplex_Point = 0,
	Simplex_Line = 1,
	Simplex_Triangle = 2,
	Simplex_Tetrahedron = 3
};

struct Simplex
{
	SimplexType type;
	vec3 A;
	vec3 B;
	vec3 C;
	vec3 D;
};

struct DoSimplexResult
{
	bool containsGoal;
	Simplex simplex;
	vec3 d;
};

inline bool DoSimplexLine(vec2* simplex, SimplexType* simplexType, vec2* D)
{
	bool result = false;

	vec3 ab = vec3(simplex[0].x - simplex[1].x, simplex[0].y - simplex[1].y, 0);
	vec3 ag = vec3(0 - simplex[1].x, 0 - simplex[1].y, 0);

	vec3 DinR3 = cross(cross(ab, ag), ab);
	*D = vec2(DinR3.x, DinR3.y);

	return result;
}

inline bool DoSimplexLineCasey(vec2* simplex, SimplexType* simplexType, vec2* D)
{
	bool result = false;

	vec3 ab = vec3(simplex[0].x - simplex[1].x, simplex[0].y - simplex[1].y, 0);
	vec3 ag = vec3(0 - simplex[1].x, 0 - simplex[1].y, 0);
	if (dot(ab, ag) > 0)
	{
		vec3 DinR3 = cross(cross(ab, ag), ab);
		*D = vec2(DinR3.x, DinR3.y);
	}
	else
	{
		Assert(false);

		simplex[0] = simplex[1];
		simplex[1] = vec2();
		*simplexType = Simplex_Point;
	}

	return result;
}

inline DoSimplexResult DoSimplexLineAllVoronoi(Simplex simplex, vec3 D)
{
	DoSimplexResult result;
	result.containsGoal = false;

	vec3 ab = simplex.B - simplex.A;
	vec3 ag = vec3(0, 0, 0) - simplex.A;
	vec3 ba = simplex.A - simplex.B;
	vec3 bg = vec3(0, 0, 0) - simplex.B;

	if (dot(ab, ag) < 0)
	{
		// Voronoi Region A
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.A;
		result.d = ag;
	}
	else if (dot(ba, bg) < 0)
	{
		// Voronoi Region B
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.B;
		result.d = bg;
	}
	else
	{
		// Voronoi Region AB
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.B;
		result.d = cross(cross(ab, ag), ab);
	}

	return result;
}

inline DoSimplexResult DoSimplexLineAllVoronoiDouble(Simplex simplex, vec3 D)
{
	DoSimplexResult result;
	result.containsGoal = false;

	dvec3 ab = dvec3(simplex.B) - dvec3(simplex.A);
	dvec3 ag = dvec3(0, 0, 0) - dvec3(simplex.A);
	dvec3 ba = dvec3(simplex.A) - dvec3(simplex.B);
	dvec3 bg = dvec3(0, 0, 0) - dvec3(simplex.B);

	if (dot(ab, ag) < 0)
	{
		// Voronoi Region A
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.A;
		result.d = ag;
	}
	else if (dot(ba, bg) < 0)
	{
		// Voronoi Region B
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.B;
		result.d = bg;
	}
	else
	{
		// Voronoi Region AB
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.B;
		result.d = cross(cross(ab, ag), ab);
	}

	return result;
}



inline bool DoSimplexTriangle(vec2* simplex, SimplexType* simplexType, vec2* D)
{
	bool result = false;

	vec3 ab = vec3((simplex[1] - simplex[2]), 0);
	vec3 ac = vec3((simplex[0] - simplex[2]), 0);
	vec3 ag = vec3((vec2(0, 0) - simplex[2]), 0);
	vec3 abc = cross(ab, ac);

	if (dot(cross(abc, ac), ag) > 0)
	{
		// NOTE:	  1  0
		// simplex = [A, C]
		simplex[1] = simplex[2];
		simplex[2] = vec2();
		*simplexType = Simplex_Line;
		vec3 dInR3 = cross(cross(ac, ag), ac);
		*D = vec2(dInR3.x, dInR3.y);
	}
	else
	{
		if (dot(cross(ab, abc), ag) > 0)
		{
			// NOTE:	  1  0
			// simplex = [A, B]
			simplex[0] = simplex[1];
			simplex[1] = simplex[2];
			simplex[2] = vec2();
			*simplexType = Simplex_Line;
			vec3 dInR3 = cross(cross(ab, ag), ab);
			*D = vec2(dInR3.x, dInR3.y);
		}
		else
		{
			// NOTE: In the 2D case this means the origin is within the triangle.
			result = true;
		}
	}

	return result;
}

inline bool DoSimplexTriangleCasey(vec2* simplex, SimplexType* simplexType, vec2* D)
{
	bool result = false;

	vec3 ab = vec3((simplex[1] - simplex[2]), 0);
	vec3 ac = vec3((simplex[0] - simplex[2]), 0);
	vec3 ag = vec3((vec2(0, 0) - simplex[2]), 0);
	vec3 abc = cross(ab, ac);

	if (dot(cross(abc, ac), ag) > 0)
	{
		if (dot(ac, ag) > 0)
		{
			// CASE 1

			// NOTE		  1  0
			// simplex = [A, C]
			simplex[1] = simplex[2];
			simplex[2] = vec2();
			*simplexType = Simplex_Line;

			vec3 dInR3 = cross(cross(ac, ag), ac);
			*D = vec2(dInR3.x, dInR3.y);
		}
		else
		{
			// Magical special fun times case
			if (dot(ab, ag) > 0)
			{
				// CASE 4

				// NOTE		  1  0
				// simplex = [A, B]
				simplex[0] = simplex[1];
				simplex[1] = simplex[2];
				simplex[2] = vec2();
				*simplexType = Simplex_Line;

				vec3 dInR3 = cross(cross(ab, ag), ab);
				*D = vec2(dInR3.x, dInR3.y);
			}
			else
			{
				// CASE 5
				Assert(false);

				// NOTE		  0
				// simplex = [A]
				simplex[0] = simplex[2];
				simplex[1] = vec2();
				simplex[2] = vec2();
				*simplexType = Simplex_Point;

				*D = vec2(ag.x, ag.y);
			}
		}
	}
	else
	{
		if (dot(cross(ab, abc), ag) > 0)
		{
			// Magical special fun times case
			if (dot(ab, ag) > 0)
			{
				// CASE 4

				// NOTE		  1  0
				// simplex = [A, B]
				simplex[0] = simplex[1];
				simplex[1] = simplex[2];
				simplex[2] = vec2();
				*simplexType = Simplex_Line;

				vec3 dInR3 = cross(cross(ab, ag), ab);
				*D = vec2(dInR3.x, dInR3.y);
			}
			else
			{
				// CASE 5
				Assert(false);

				// NOTE		  0
				// simplex = [A]
				simplex[0] = simplex[2];
				simplex[1] = vec2();
				simplex[2] = vec2();
				*simplexType = Simplex_Point;

				*D = vec2(ag.x, ag.y);
			}
		}
		else
		{
			// CASE 2 AND CASE 3

			// NOTE: In the 2D case this means the origin is within the triangle.
			result = true;
			// NOTE: This is for the 3D case.
			/*if (dot(abc, ag) > 0)
			{
			*D = abc;
			}
			else
			{
			// NOTE: Swap points so plane normal will be in the direction of the new point.
			vec3 t = simplex[1];
			simplex[1] = simplex[0];
			simplex[0] = t;
			*D = -abc;
			}*/
		}
	}

	return result;
}

inline DoSimplexResult DoSimplexTriangleAllVoronoi(Simplex simplex, vec3 D)
{
	DoSimplexResult result;
	result.containsGoal = false;

	// Vectors
	vec3 ab = simplex.B - simplex.A;
	vec3 ac = simplex.C - simplex.A;
	vec3 ag = vec3(0, 0, 0) - simplex.A;

	vec3 bc = simplex.C - simplex.B;
	vec3 ba = simplex.A - simplex.B;
	vec3 bg = vec3(0, 0, 0) - simplex.B;

	vec3 ca = simplex.A - simplex.C;
	vec3 cb = simplex.B - simplex.C;
	vec3 cg = vec3(0, 0, 0) - simplex.C;

	vec3 sn = cross(ab, ac);


	// Tests
	F32 ab_DOT_ag = dot(ab, ag);
	F32 ac_DOT_ag = dot(ac, ag);

	F32 ba_DOT_bg = dot(ba, bg);
	F32 bc_DOT_bg = dot(bc, bg);

	F32 ca_DOT_cg = dot(ca, cg);
	F32 cb_DOT_cg = dot(cb, cg);

	F32 perpAB_DOT_ag = dot(cross(ab, sn), ag);
	F32 perpAC_DOT_ag = dot(cross(sn, ac), ag);
	F32 perpBC_DOT_bg = dot(cross(bc, sn), bg);
	F32 perpCB_DOT_cg = dot(cross(sn, cb), cg);
	F32 sn_DOT_ag = dot(sn, ag);

	if (ab_DOT_ag < 0 &&
		ac_DOT_ag < 0)
	{
		// Voronoi Region A
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.A;
		result.d = ag;
	}
	else if (ba_DOT_bg < 0 &&
		bc_DOT_bg < 0)
	{
		// Voronoi Region B
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.B;
		result.d = bg;
	}
	else if (ca_DOT_cg < 0 &&
		cb_DOT_cg < 0)
	{
		// Voronoi Region C
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.C;
		result.d = cg;
	}
	else if (ab_DOT_ag >= 0 &&
		ba_DOT_bg >= 0 &&
		perpAB_DOT_ag > 0)
	{
		// Voronoi Region AB
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.B;
		result.d = cross(cross(ab, ag), ab);
	}
	else if (ac_DOT_ag >= 0 &&
		ca_DOT_cg >= 0 &&
		perpAC_DOT_ag > 0)
	{
		// Voronoi Region AC
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.C;
		result.d = cross(cross(ac, ag), ac);
	}
	else if (bc_DOT_bg >= 0 &&
		cb_DOT_cg >= 0 &&
		perpBC_DOT_bg > 0)
	{
		// Voronoi Region BC
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.B;
		result.simplex.B = simplex.C;
		result.d = cross(cross(bc, bg), bc);
	}
	else if (perpAB_DOT_ag <= 0 &&
		perpAC_DOT_ag <= 0 &&
		perpBC_DOT_bg <= 0 &&
		sn_DOT_ag > 0)
	{
		// Voronoi Region ABC Above
		result.simplex.type = Simplex_Triangle;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.C;
		result.simplex.C = simplex.B;
		result.d = sn;
	}
	else if (perpAB_DOT_ag <= 0 &&
		perpAC_DOT_ag <= 0 &&
		perpBC_DOT_bg <= 0 &&
		sn_DOT_ag <= 0)
	{
		// Voronoi Region ABC Below
		result.simplex.type = Simplex_Triangle;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.B;
		result.simplex.C = simplex.C;
		result.d = -sn;
	}

	//result.d = normalize(result.d);
	return result;
}

inline DoSimplexResult DoSimplexTriangleAllVoronoiDouble(Simplex simplex, vec3 D)
{
	DoSimplexResult result;
	result.containsGoal = false;

	// Vectors
	dvec3 ab = dvec3(simplex.B) - dvec3(simplex.A);
	dvec3 ac = dvec3(simplex.C) - dvec3(simplex.A);
	dvec3 ag = dvec3(0, 0, 0) - dvec3(simplex.A);

	dvec3 bc = dvec3(simplex.C) - dvec3(simplex.B);
	dvec3 ba = dvec3(simplex.A) - dvec3(simplex.B);
	dvec3 bg = dvec3(0, 0, 0) - dvec3(simplex.B);

	dvec3 ca = dvec3(simplex.A) - dvec3(simplex.C);
	dvec3 cb = dvec3(simplex.B) - dvec3(simplex.C);
	dvec3 cg = dvec3(0, 0, 0) - dvec3(simplex.C);

	dvec3 sn = cross(ab, ac);


	// Tests
	F64 ab_DOT_ag = dot(ab, ag);
	F64 ac_DOT_ag = dot(ac, ag);

	F64 ba_DOT_bg = dot(ba, bg);
	F64 bc_DOT_bg = dot(bc, bg);

	F64 ca_DOT_cg = dot(ca, cg);
	F64 cb_DOT_cg = dot(cb, cg);

	F64 perpAB_DOT_ag = dot(cross(ab, sn), ag);
	F64 perpAC_DOT_ag = dot(cross(sn, ac), ag);
	F64 perpBC_DOT_bg = dot(cross(bc, sn), bg);
	F64 perpCB_DOT_cg = dot(cross(sn, cb), cg);
	F64 sn_DOT_ag = dot(sn, ag);

	if (ab_DOT_ag < 0 &&
		ac_DOT_ag < 0)
	{
		// Voronoi Region A
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.A;
		result.d = ag;
	}
	else if (ba_DOT_bg < 0 &&
		bc_DOT_bg < 0)
	{
		// Voronoi Region B
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.B;
		result.d = bg;
	}
	else if (ca_DOT_cg < 0 &&
		cb_DOT_cg < 0)
	{
		// Voronoi Region C
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.C;
		result.d = cg;
	}
	else if (ab_DOT_ag >= 0 &&
		ba_DOT_bg >= 0 &&
		perpAB_DOT_ag > 0)
	{
		// Voronoi Region AB
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.B;
		result.d = cross(cross(ab, ag), ab);
	}
	else if (ac_DOT_ag >= 0 &&
		ca_DOT_cg >= 0 &&
		perpAC_DOT_ag > 0)
	{
		// Voronoi Region AC
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.C;
		result.d = normalize(cross(cross(ac, ag), ac));
	}
	else if (bc_DOT_bg >= 0 &&
		cb_DOT_cg >= 0 &&
		perpBC_DOT_bg > 0)
	{
		// Voronoi Region BC
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.B;
		result.simplex.B = simplex.C;
		result.d = cross(cross(bc, bg), bc);
	}
	else if (perpAB_DOT_ag <= 0 &&
		perpAC_DOT_ag <= 0 &&
		perpBC_DOT_bg <= 0 &&
		sn_DOT_ag > 0)
	{
		// Voronoi Region ABC Above
		result.simplex.type = Simplex_Triangle;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.C;
		result.simplex.C = simplex.B;
		result.d = sn;
	}
	else if (perpAB_DOT_ag <= 0 &&
		perpAC_DOT_ag <= 0 &&
		perpBC_DOT_bg <= 0 &&
		sn_DOT_ag <= 0)
	{
		// Voronoi Region ABC Below
		result.simplex.type = Simplex_Triangle;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.B;
		result.simplex.C = simplex.C;
		result.d = -sn;
	}

	//result.d = normalize(result.d);
	return result;
}



inline bool DoSimplexTetrahedron(vec2* simplex, SimplexType* simplexType, vec2* D)
{
	bool result = false;

	return result;
}

inline DoSimplexResult DoSimplexTetrahedronAllVoronoi(Simplex simplex, vec3 D)
{
	DoSimplexResult result;
	result.containsGoal = false;

	vec3 G = vec3(0, 0, 0);
	vec3 AG = G - simplex.A;
	vec3 AB = simplex.B - simplex.A;
	vec3 AC = simplex.C - simplex.A;
	vec3 AD = simplex.D - simplex.A;

	vec3 BG = G - simplex.B;
	vec3 BA = simplex.A - simplex.B;
	vec3 BC = simplex.C - simplex.B;
	vec3 BD = simplex.D - simplex.B;

	vec3 CG = G - simplex.C;
	vec3 CA = simplex.A - simplex.C;
	vec3 CB = simplex.B - simplex.C;
	vec3 CD = simplex.D - simplex.C;

	vec3 DG = G - simplex.D;
	vec3 DA = simplex.A - simplex.D;
	vec3 DB = simplex.B - simplex.D;
	vec3 DC = simplex.C - simplex.D;

	vec3 normalABC = cross(AB, AC);
	vec3 normalADB = cross(AD, AB);
	vec3 normalACD = cross(AC, AD);

	vec3 normalBCA = cross(BC, BA);
	vec3 normalBDC = cross(BD, BC);
	vec3 normalBAD = cross(BA, BD);

	vec3 normalCDA = cross(CD, CA);
	vec3 normalCBD = cross(CB, CD);

	if (dot(AG, AB)<0 &&
		dot(AG, AC)<0 &&
		dot(AG, AD)<0)
	{
		// Voronoi A
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.A;
		result.d = AG;
	}
	else if (dot(BG, BA)<0 &&
		dot(BG, BC)<0 &&
		dot(BG, BD)<0)
	{
		// Voronoi B
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.B;
		result.d = BG;
	}
	else if (dot(CG, CA)<0 &&
		dot(CG, CB)<0 &&
		dot(CG, CD)<0)
	{
		// Voronoi C
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.C;
		result.d = CG;
	}
	else if (dot(DG, DA)<0 &&
		dot(DG, DB)<0 &&
		dot(DG, DC)<0)
	{
		// Voronoi D
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.D;
		result.d = DG;
	}
	// NOTE: Sign on the plane tests may need to be >= but i dont think so, investigate if errors crop up
	else if (dot(AG, AB) >= 0 &&
		dot(BG, BA) >= 0 &&
		dot(AG, cross(AB, normalABC))>0 &&
		dot(AG, cross(normalADB, AB))>0)
	{
		// Voronoi AB
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.B;
		result.d = cross(cross(AB, AG), AB);
	}
	else if (dot(AG, AC) >= 0 &&
		dot(CG, CA) >= 0 &&
		dot(AG, cross(AC, normalACD))>0 &&
		dot(AG, cross(normalABC, AC))>0)
	{
		// Voronoi AC
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.C;
		result.d = cross(cross(AC, AG), AC);
	}
	else if (dot(AG, AD) >= 0 &&
		dot(DG, DA) >= 0 &&
		dot(AG, cross(AD, normalADB))>0 &&
		dot(AG, cross(normalACD, AD))>0)
	{
		// Voronoi AD
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.D;
		result.d = cross(cross(AD, AG), AD);
	}
	else if (dot(BG, BC) >= 0 &&
		dot(CG, CB) >= 0 &&
		dot(BG, cross(BC, normalBCA))>0 &&
		dot(BG, cross(normalBDC, BC))>0)
	{
		// Voronoi BC
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.B;
		result.simplex.B = simplex.C;
		result.d = cross(cross(BC, BG), BC);
	}
	else if (dot(BG, BD) >= 0 &&
		dot(DG, DB) >= 0 &&
		dot(BG, cross(BD, normalBDC))>0 &&
		dot(BG, cross(normalBAD, BD))>0)
	{
		// Voronoi BD
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.B;
		result.simplex.B = simplex.D;
		result.d = cross(cross(BD, BG), BD);
	}
	else if (dot(CG, CD) >= 0 &&
		dot(DG, DC) >= 0 &&
		dot(BG, cross(BD, normalBDC))>0 &&
		dot(BG, cross(normalBAD, BD))>0)
	{
		// Voronoi CD
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.C;
		result.simplex.B = simplex.D;
		result.d = cross(cross(CD, CG), CD);
	}
	else if ((dot(AG, normalABC))*(dot(AD, normalABC))<0)
	{
		// Voronoi ABC
		result.simplex.type = Simplex_Triangle;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.B;
		result.simplex.C = simplex.C;
		if (dot(AG, normalABC)>0)
		{
			result.d = normalABC;
		}
		else
		{
			result.d = -normalABC;
		}
	}
	else if ((dot(AG, normalADB))*(dot(AC, normalADB))<0)
	{
		// Voronoi ABD
		result.simplex.type = Simplex_Triangle;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.B;
		result.simplex.C = simplex.D;
		if (dot(AG, normalADB)>0)
		{
			result.d = normalADB;
		}
		else
		{
			result.d = -normalADB;
		}
	}
	else if ((dot(AG, normalACD))*(dot(AB, normalACD))<0)
	{
		// Voronoi ACD
		result.simplex.type = Simplex_Triangle;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.C;
		result.simplex.C = simplex.D;
		if (dot(AG, normalACD)>0)
		{
			result.d = normalACD;
		}
		else
		{
			result.d = -normalACD;
		}

	}
	else if ((dot(BG, normalBDC))*(dot(BA, normalBDC))<0)
	{
		// Voronoi BCD
		result.simplex.type = Simplex_Triangle;
		result.simplex.A = simplex.B;
		result.simplex.B = simplex.C;
		result.simplex.C = simplex.D;
		if (dot(BG, normalBDC)>0)
		{
			result.d = normalBDC;
		}
		else
		{
			result.d = -normalBDC;
		}
	}
	else
	{
		// Voronoi ABCD
		result.containsGoal = true;
		result.simplex.type = Simplex_Tetrahedron;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.B;
		result.simplex.C = simplex.C;
		result.simplex.D = simplex.D;
		// NOTE: If d is needed from here remember it is not set.
	}

	return result;
}



inline bool DoSimplex__(vec2* simplex, SimplexType* simplexType, vec2* D)
{
	bool result = false;

	switch (*simplexType)
	{
	case Simplex_Line:
	{
						 vec2 tsimplex[4];
						 SimplexType tsimplexType;
						 vec2 tD;

						 tsimplex[0] = simplex[0];
						 tsimplex[1] = simplex[1];
						 tsimplex[2] = simplex[2];
						 tsimplex[3] = simplex[3];
						 tsimplexType = *simplexType;
						 tD = *D;
						 result = DoSimplexLine(tsimplex, &tsimplexType, &tD);

						 tsimplex[0] = simplex[0];
						 tsimplex[1] = simplex[1];
						 tsimplex[2] = simplex[2];
						 tsimplex[3] = simplex[3];
						 tsimplexType = *simplexType;
						 tD = *D;
						 bool resultCasey = DoSimplexLineCasey(tsimplex, &tsimplexType, &tD);

						 tsimplex[0] = simplex[0];
						 tsimplex[1] = simplex[1];
						 tsimplex[2] = simplex[2];
						 tsimplex[3] = simplex[3];
						 tsimplexType = *simplexType;
						 tD = *D;

						 Simplex s;
						 s.type = tsimplexType;
						 s.A = vec3(tsimplex[1].x, tsimplex[1].y, 0);
						 s.B = vec3(tsimplex[0].x, tsimplex[0].y, 0);

						 DoSimplexResult resultAllVoronoi = DoSimplexLineAllVoronoi(s, vec3(tD.x, tD.y, 0));

						 tsimplexType = resultAllVoronoi.simplex.type;
						 if (tsimplexType == Simplex_Point)
						 {
							 tsimplex[0] = vec2(resultAllVoronoi.simplex.A.x, resultAllVoronoi.simplex.A.y);
							 tsimplex[1] = vec2(0, 0);
							 tsimplex[2] = vec2(0, 0);
							 tsimplex[3] = vec2(0, 0);
						 }
						 else if (tsimplexType == Simplex_Line)
						 {
							 tsimplex[0] = vec2(resultAllVoronoi.simplex.B.x, resultAllVoronoi.simplex.B.y);
							 tsimplex[1] = vec2(resultAllVoronoi.simplex.A.x, resultAllVoronoi.simplex.A.y);
							 tsimplex[2] = vec2(0, 0);
							 tsimplex[3] = vec2(0, 0);
						 }
						 tD = vec2(resultAllVoronoi.d.x, resultAllVoronoi.d.y);



						 Assert(resultCasey == result);
						 Assert(resultCasey == resultAllVoronoi.containsGoal);
						 Assert(result == resultAllVoronoi.containsGoal);



						 simplex[0] = tsimplex[0];
						 simplex[1] = tsimplex[1];
						 simplex[2] = tsimplex[2];
						 simplex[3] = tsimplex[3];
						 *simplexType = tsimplexType;
						 *D = tD;

						 result = resultCasey;
						 break;
	}

	case Simplex_Triangle:
	{
							 vec2 tsimplex[4];
							 SimplexType tsimplexType;
							 vec2 tD;

							 tsimplex[0] = simplex[0];
							 tsimplex[1] = simplex[1];
							 tsimplex[2] = simplex[2];
							 tsimplex[3] = simplex[3];
							 tsimplexType = *simplexType;
							 tD = *D;
							 result = DoSimplexTriangle(tsimplex, &tsimplexType, &tD);


							 tsimplex[0] = simplex[0];
							 tsimplex[1] = simplex[1];
							 tsimplex[2] = simplex[2];
							 tsimplex[3] = simplex[3];
							 tsimplexType = *simplexType;
							 tD = *D;
							 bool resultCasey = DoSimplexTriangleCasey(tsimplex, &tsimplexType, &tD);


							 //
							 tsimplex[0] = simplex[0];
							 tsimplex[1] = simplex[1];
							 tsimplex[2] = simplex[2];
							 tsimplex[3] = simplex[3];
							 tsimplexType = *simplexType;
							 tD = *D;

							 Simplex s;
							 s.type = tsimplexType;
							 s.A = vec3(tsimplex[2].x, tsimplex[2].y, 0);
							 s.B = vec3(tsimplex[1].x, tsimplex[1].y, 0);
							 s.C = vec3(tsimplex[0].x, tsimplex[0].y, 0);

							 DoSimplexResult resultAllVoronoi = DoSimplexTriangleAllVoronoi(s, vec3(tD.x, tD.y, 0));

							 tsimplexType = resultAllVoronoi.simplex.type;
							 if (tsimplexType == Simplex_Point)
							 {
								 tsimplex[0] = vec2(resultAllVoronoi.simplex.A.x, resultAllVoronoi.simplex.A.y);
								 tsimplex[1] = vec2(0, 0);
								 tsimplex[2] = vec2(0, 0);
								 tsimplex[3] = vec2(0, 0);
							 }
							 else if (tsimplexType == Simplex_Line)
							 {
								 tsimplex[0] = vec2(resultAllVoronoi.simplex.B.x, resultAllVoronoi.simplex.B.y);
								 tsimplex[1] = vec2(resultAllVoronoi.simplex.A.x, resultAllVoronoi.simplex.A.y);
								 tsimplex[2] = vec2(0, 0);
								 tsimplex[3] = vec2(0, 0);
							 }
							 else if (tsimplexType == Simplex_Triangle)
							 {
								 tsimplex[0] = vec2(resultAllVoronoi.simplex.C.x, resultAllVoronoi.simplex.C.y);
								 tsimplex[1] = vec2(resultAllVoronoi.simplex.B.x, resultAllVoronoi.simplex.B.y);
								 tsimplex[2] = vec2(resultAllVoronoi.simplex.A.x, resultAllVoronoi.simplex.A.y);
								 tsimplex[3] = vec2(0, 0);
								 resultAllVoronoi.containsGoal = true;
							 }
							 tD = vec2(resultAllVoronoi.d.x, resultAllVoronoi.d.y);



							 Assert(resultCasey == result);
							 Assert(resultCasey == resultAllVoronoi.containsGoal);
							 Assert(result == resultAllVoronoi.containsGoal);
							 //




							 //Assert(resultCasey == result);

							 simplex[0] = tsimplex[0];
							 simplex[1] = tsimplex[1];
							 simplex[2] = tsimplex[2];
							 simplex[3] = tsimplex[3];
							 *simplexType = tsimplexType;
							 *D = tD;

							 result = resultCasey;
							 break;
	}

	case Simplex_Tetrahedron:
	{
								result = DoSimplexTetrahedron(simplex, simplexType, D);
								break;
	}

	default:
	{
			   break;
	}
	}

	return result;
}

inline bool DoSimplex(vec2* simplex, SimplexType* simplexType, vec2* D)
{
	bool result = false;

	switch (*simplexType)
	{
	case Simplex_Line:
	{

						 result = DoSimplexLine(simplex, simplexType, D);
						 break;
	}

	case Simplex_Triangle:
	{

							 result = DoSimplexTriangle(simplex, simplexType, D);
							 break;
	}

	case Simplex_Tetrahedron:
	{
								result = DoSimplexTetrahedron(simplex, simplexType, D);
								break;
	}

	default:
	{
			   break;
	}
	}

	return result;
}

inline bool DoSimplex_Casey(vec2* simplex, SimplexType* simplexType, vec2* D)
{
	bool result = false;

	switch (*simplexType)
	{
	case Simplex_Line:
	{

						 result = DoSimplexLineCasey(simplex, simplexType, D);
						 break;
	}

	case Simplex_Triangle:
	{

							 result = DoSimplexTriangleCasey(simplex, simplexType, D);
							 break;
	}

	case Simplex_Tetrahedron:
	{
								result = DoSimplexTetrahedron(simplex, simplexType, D);
								break;
	}

	default:
	{
			   break;
	}
	}

	return result;
}

//#define DOUBLES 1
inline bool DoSimplex_AllVoronoi(vec2* simplex, SimplexType* simplexType, vec2* D)
{
	bool result = false;

	switch (*simplexType)
	{
	case Simplex_Line:
	{
						 Simplex s;
						 s.type = *simplexType;
						 s.A = vec3(simplex[1].x, simplex[1].y, 0);
						 s.B = vec3(simplex[0].x, simplex[0].y, 0);

#ifndef DOUBLES
						 DoSimplexResult rav = DoSimplexLineAllVoronoi(s, vec3(D->x, D->y, 0));
#else
						 DoSimplexResult rav = DoSimplexLineAllVoronoiDouble(s, vec3(D->x, D->y, 0));
#endif

						 if (rav.simplex.type == Simplex_Point)
						 {
							 simplex[0] = vec2(rav.simplex.A.x, rav.simplex.A.y);
							 simplex[1] = vec2(0, 0);
							 simplex[2] = vec2(0, 0);
							 simplex[3] = vec2(0, 0);
						 }
						 else if (rav.simplex.type == Simplex_Line)
						 {
							 simplex[0] = vec2(rav.simplex.B.x, rav.simplex.B.y);
							 simplex[1] = vec2(rav.simplex.A.x, rav.simplex.A.y);
							 simplex[2] = vec2(0, 0);
							 simplex[3] = vec2(0, 0);
						 }

						 *simplexType = rav.simplex.type;
						 *D = vec2(rav.d.x, rav.d.y);
						 result = rav.containsGoal;
						 break;
	}

	case Simplex_Triangle:
	{
							 Simplex s;
							 s.type = *simplexType;
							 s.A = vec3(simplex[2].x, simplex[2].y, 0);
							 s.B = vec3(simplex[1].x, simplex[1].y, 0);
							 s.C = vec3(simplex[0].x, simplex[0].y, 0);

#ifndef DOUBLES
							 DoSimplexResult rav = DoSimplexTriangleAllVoronoi(s, vec3(D->x, D->y, 0));
#else
							 DoSimplexResult rav = DoSimplexTriangleAllVoronoiDouble(s, vec3(D->x, D->y, 0));
#endif

							 if (rav.simplex.type == Simplex_Point)
							 {
								 simplex[0] = vec2(rav.simplex.A.x, rav.simplex.A.y);
								 simplex[1] = vec2(0, 0);
								 simplex[2] = vec2(0, 0);
								 simplex[3] = vec2(0, 0);
							 }
							 else if (rav.simplex.type == Simplex_Line)
							 {
								 simplex[0] = vec2(rav.simplex.B.x, rav.simplex.B.y);
								 simplex[1] = vec2(rav.simplex.A.x, rav.simplex.A.y);
								 simplex[2] = vec2(0, 0);
								 simplex[3] = vec2(0, 0);
							 }
							 else if (rav.simplex.type == Simplex_Triangle)
							 {
								 simplex[0] = vec2(rav.simplex.C.x, rav.simplex.C.y);
								 simplex[1] = vec2(rav.simplex.B.x, rav.simplex.B.y);
								 simplex[2] = vec2(rav.simplex.A.x, rav.simplex.A.y);
								 simplex[3] = vec2(0, 0);
								 rav.containsGoal = true;
							 }

							 *simplexType = rav.simplex.type;
							 *D = vec2(rav.d.x, rav.d.y);
							 result = rav.containsGoal;
							 break;
	}

	case Simplex_Tetrahedron:
	{
								result = DoSimplexTetrahedron(simplex, simplexType, D);
								break;
	}

	default:
	{
			   break;
	}
	}

	return result;
}






template<typename S1, typename S2>
inline bool GJK(S1 shapeA, S2 shapeB, U32* numberOfLoopsCompleted = NULL)
{
	bool collisionDetected = false;

	vec2 S = Support(&shapeA, &shapeB, &vec2(1.0f, -1.0f));
	vec2 simplex[4];
	SimplexType simplexType = Simplex_Point;
	simplex[0] = S;
	vec2 D = -S;

	U32 loopCounter = 0;
	for (; loopCounter < 10;)
	{
		++loopCounter;
		vec2 A = Support(&shapeA, &shapeB, &D);
		if (A == vec2(0.0f, 0.0f))
		{
			collisionDetected = true;
			break;
		}
		if (dot(A, D) < 0)
		{
			collisionDetected = false;
			break;
		}
		simplexType = (SimplexType)(simplexType + 1);
		simplex[simplexType] = A;
		if (DoSimplex(simplex, &simplexType, &D))
		{
			collisionDetected = true;
			break;
		}
	}

	if (numberOfLoopsCompleted)
	{
		*numberOfLoopsCompleted = loopCounter;
	}

	return collisionDetected;
}

template<typename S1, typename S2>
inline bool GJK_Casey(S1 shapeA, S2 shapeB, U32* numberOfLoopsCompleted = NULL)
{
	bool collisionDetected = false;

	vec2 S = Support(&shapeA, &shapeB, &vec2(1.0f, -1.0f));
	vec2 simplex[4];
	SimplexType simplexType = Simplex_Point;
	simplex[0] = S;
	vec2 D = -S;

	U32 loopCounter = 0;
	for (; loopCounter < 10;)
	{
		vec2 A = Support(&shapeA, &shapeB, &D);
		if (A == vec2(0.0f, 0.0f))
		{
			collisionDetected = true;
			break;
		}
		if (dot(A, D) < 0)
		{
			collisionDetected = false;
			break;
		}
		simplexType = (SimplexType)(simplexType + 1);
		simplex[simplexType] = A;
		if (DoSimplex_Casey(simplex, &simplexType, &D))
		{
			collisionDetected = true;
			break;
		}
	}

	if (numberOfLoopsCompleted)
	{
		*numberOfLoopsCompleted = loopCounter;
	}

	return collisionDetected;
}

template<typename S1, typename S2>
inline bool GJK_AllVoronoi(S1 shapeA, S2 shapeB, U32* numberOfLoopsCompleted = NULL)
{
	bool collisionDetected = false;

	vec2 S = Support(&shapeA, &shapeB, &vec2(1.0f, -1.0f));
	vec2 simplex[4];
	SimplexType simplexType = Simplex_Point;
	simplex[0] = S;
	vec2 D = -S;

	U32 loopCounter = 0;
	for (; loopCounter < 10;)
	{
		vec2 A = Support(&shapeA, &shapeB, &D);
		if (A == vec2(0.0f, 0.0f))
		{
			collisionDetected = true;
			break;
		}
		if (dot(A, D) < 0)
		{
			collisionDetected = false;
			break;
		}
		simplexType = (SimplexType)(simplexType + 1);
		simplex[simplexType] = A;
		if (DoSimplex_AllVoronoi(simplex, &simplexType, &D))
		{
			collisionDetected = true;
			break;
		}
	}

	if (numberOfLoopsCompleted)
	{
		*numberOfLoopsCompleted = loopCounter;
	}

	return collisionDetected;
}






// Collision Detection Tests

// r rotated 0 degrees
void CollisionTestsRectRect1()
{
	Rectangle g;
	g.origin = vec2(0, 0);
	g.halfDim = vec2(.5, .5);
	g.transform = mat3(1, 0, 0, 0, 1, 0, .5, .5, 1);


	F32 xs[7] = { -1, -.5, 0, .5, 1, 1.5, 2 };
	F32 ys[7] = { 2, 1.5, 1, .5, 0, -.5, -1 };
	U32 hits[7][7];

	for (U32 y = 0; y < 7; ++y)
	{
		for (U32 x = 0; x < 7; ++x)
		{
			Rectangle r;
			r.origin = vec2(0, 0);
			r.halfDim = vec2(.5, .5);
			r.transform = mat3(1, 0, 0, 0, 1, 0, xs[x], ys[y], 1);

			bool collision = GJK(g, r);
			if (collision)
			{
				hits[y][x] = 1;
			}
			else
			{
				hits[y][x] = 0;
			}
		}
	}

	for (U32 i = 0; i < 7; ++i)
	{
		DebugPrintf(1024, "hits %u %u %u %u %u %u %u\n", hits[i][0], hits[i][1], hits[i][2], hits[i][3], hits[i][4], hits[i][5], hits[i][6]);
	}
}

// r rotated 45 degrees 
void CollisionTestsRectRect2()
{
	Rectangle g;
	g.origin = vec2(0, 0);
	g.halfDim = vec2(.5, .5);
	g.transform = mat3(1, 0, 0, 0, 1, 0, .5, .5, 1);

	F32 angle = DegreesToRadians(45.0f);

	F32 xs[7] = { -1, -.5, 0, .5, 1, 1.5, 2 };
	F32 ys[7] = { 2, 1.5, 1, .5, 0, -.5, -1 };
	U32 hits_Simple[7][7];
	U32 hits_Casey[7][7];
	U32 hits_AllVoronoi[7][7];

	for (U32 y = 0; y < 7; ++y)
	{
		for (U32 x = 0; x < 7; ++x)
		{
			Rectangle r;
			r.origin = vec2(0, 0);
			r.halfDim = vec2(.5, .5);
			r.transform = mat3(1, 0, 0, 0, 1, 0, xs[x], ys[y], 1) * mat3(cos(angle), sin(angle), 0, -sin(angle), cos(angle), 0, 0, 0, 1);

			bool collisionSimple = GJK(g, r);
			bool collisionCasey = GJK_Casey(g, r);
			bool collisionAllVoronoi = GJK_AllVoronoi(g, r);

			Assert(collisionSimple == collisionCasey);
			Assert(collisionSimple == collisionAllVoronoi);
			Assert(collisionCasey == collisionAllVoronoi);

			if (collisionSimple)
			{
				hits_Simple[y][x] = 1;
			}
			else
			{
				hits_Simple[y][x] = 0;
			}

			if (collisionCasey)
			{
				hits_Casey[y][x] = 1;
			}
			else
			{
				hits_Casey[y][x] = 0;
			}

			if (collisionAllVoronoi)
			{
				hits_AllVoronoi[y][x] = 1;
			}
			else
			{
				hits_AllVoronoi[y][x] = 0;
			}
		}
	}

	for (U32 i = 0; i < 7; ++i)
	{
		DebugPrintf(1024, "hits_Simple %u %u %u %u %u %u %u\n", hits_Simple[i][0], hits_Simple[i][1], hits_Simple[i][2], hits_Simple[i][3], hits_Simple[i][4], hits_Simple[i][5], hits_Simple[i][6]);
	}
	DebugPrint("\n\n");
	for (U32 i = 0; i < 7; ++i)
	{
		DebugPrintf(1024, "hits_Casey %u %u %u %u %u %u %u\n", hits_Casey[i][0], hits_Casey[i][1], hits_Casey[i][2], hits_Casey[i][3], hits_Casey[i][4], hits_Casey[i][5], hits_Casey[i][6]);
	}
	DebugPrint("\n\n");
	for (U32 i = 0; i < 7; ++i)
	{
		DebugPrintf(1024, "hits_AllVoronoi %u %u %u %u %u %u %u\n", hits_AllVoronoi[i][0], hits_AllVoronoi[i][1], hits_AllVoronoi[i][2], hits_AllVoronoi[i][3], hits_AllVoronoi[i][4], hits_AllVoronoi[i][5], hits_AllVoronoi[i][6]);
	}
}
