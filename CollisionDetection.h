#pragma once

#include "Types.h"
#include "Math.h"

#include "DebugAPI.h"

#define UNREASONABLEFLOATNUMBER 1000000.0f

struct Rectangle
{
	mat4 transform;

	vec2 origin; // NOTE: this is in "model" space
	vec2 halfDim;
};

inline vec3 Support(Rectangle A, vec3 direction)
{
	vec4 AiPoints[4] = {
		A.transform * vec4(A.origin.x - A.halfDim.x, A.origin.y - A.halfDim.y, 0.0f, 1.0f),
		A.transform * vec4(A.origin.x - A.halfDim.x, A.origin.y + A.halfDim.y, 0.0f, 1.0f),
		A.transform * vec4(A.origin.x + A.halfDim.x, A.origin.y - A.halfDim.y, 0.0f, 1.0f),
		A.transform * vec4(A.origin.x + A.halfDim.x, A.origin.y + A.halfDim.y, 0.0f, 1.0f)
	};
	F32 AiDots[4] = {
		dot(direction, vec3(AiPoints[0].x, AiPoints[0].y, AiPoints[0].z)),
		dot(direction, vec3(AiPoints[1].x, AiPoints[1].y, AiPoints[1].z)),
		dot(direction, vec3(AiPoints[2].x, AiPoints[2].y, AiPoints[2].z)),
		dot(direction, vec3(AiPoints[3].x, AiPoints[3].y, AiPoints[3].z))
	};

	U32 maxPositionAi = 0;
	F32 maxDotAi = -UNREASONABLEFLOATNUMBER;
	for (U32 i = 0; i < 4; ++i)
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



struct Circle
{
	vec3 origin;
	F32  radius;
};

inline vec3 Support(Circle A, vec3 direction)
{
	// NOTE: Should project direction vector onto plane of the sphere
	direction = normalize(vec3(direction.x, direction.y, 0));
	vec3 maxA = A.origin + (A.radius * direction);

	return maxA;
}



struct Triangle
{
	mat4 transform;

	vec3 origin; // NOTE: this is in "model" space
	vec3 points[3];
};

inline vec3 Support(Triangle A, vec3 direction)
{
	vec4 AiPoints[3] = {
		A.transform * vec4(A.origin.x + A.points[0].x, A.origin.y + A.points[0].y, A.origin.z + A.points[0].z, 1.0f),
		A.transform * vec4(A.origin.x + A.points[1].x, A.origin.y + A.points[1].y, A.origin.z + A.points[1].z, 1.0f),
		A.transform * vec4(A.origin.x + A.points[2].x, A.origin.y + A.points[2].y, A.origin.z + A.points[2].z, 1.0f)
	};
	F32 AiDots[3] = {
		dot(direction, vec3(AiPoints[0].x, AiPoints[0].y, AiPoints[0].z)),
		dot(direction, vec3(AiPoints[1].x, AiPoints[1].y, AiPoints[1].z)),
		dot(direction, vec3(AiPoints[2].x, AiPoints[2].y, AiPoints[2].z))
	};

	U32 maxPositionAi = 0;
	F32 maxDotAi = -UNREASONABLEFLOATNUMBER;
	for (U32 i = 0; i < 3; ++i)
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



struct OrientedBoundingBox
{
	mat4 transform;

	vec3 origin; // NOTE: this is in "model" space
	vec3 halfDim;
};

inline vec3 Support(OrientedBoundingBox A, vec3 direction)
{
	vec4 AiPoints[8] = {
		A.transform * vec4(A.origin.x - A.halfDim.x, A.origin.y - A.halfDim.y, A.origin.z + A.halfDim.z, 1.0f),
		A.transform * vec4(A.origin.x - A.halfDim.x, A.origin.y + A.halfDim.y, A.origin.z + A.halfDim.z, 1.0f),
		A.transform * vec4(A.origin.x + A.halfDim.x, A.origin.y - A.halfDim.y, A.origin.z + A.halfDim.z, 1.0f),
		A.transform * vec4(A.origin.x + A.halfDim.x, A.origin.y + A.halfDim.y, A.origin.z + A.halfDim.z, 1.0f),
		A.transform * vec4(A.origin.x - A.halfDim.x, A.origin.y - A.halfDim.y, A.origin.z - A.halfDim.z, 1.0f),
		A.transform * vec4(A.origin.x - A.halfDim.x, A.origin.y + A.halfDim.y, A.origin.z - A.halfDim.z, 1.0f),
		A.transform * vec4(A.origin.x + A.halfDim.x, A.origin.y - A.halfDim.y, A.origin.z - A.halfDim.z, 1.0f),
		A.transform * vec4(A.origin.x + A.halfDim.x, A.origin.y + A.halfDim.y, A.origin.z - A.halfDim.z, 1.0f),
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
	F32 maxDotAi = -UNREASONABLEFLOATNUMBER;
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

inline vec3 Support(Sphere A, vec3 direction)
{
	if (direction == vec3(0,0,0))
	{
		RandomNumberGenerator randomDirectionGenerator;
		SeedRandomNumberGenerator(&randomDirectionGenerator, 1);
		direction = vec3(RandomF32Between(&randomDirectionGenerator, -1, 1), RandomF32Between(&randomDirectionGenerator, -1, 1), RandomF32Between(&randomDirectionGenerator,-1, 1));
	}
	direction = normalize(direction);
	vec3 maxA = A.origin + (A.radius * direction);

	return maxA;
}



struct Capsule
{
	mat4 transform;

	vec3 points[2]; // NOTE: these are in "model" space
	F32 radius;
};

inline vec3 Support(Capsule A, vec3 direction)
{
	vec4 AiPoints[2] = {
		A.transform * vec4(A.points[0].x, A.points[0].y, A.points[0].z, 1.0f),
		A.transform * vec4(A.points[1].x, A.points[1].y, A.points[1].z, 1.0f)
	};
	F32 AiDots[2] = {
		dot(direction, vec3(AiPoints[0])),
		dot(direction, vec3(AiPoints[1]))
	};

	U32 maxPositionAi = 0;
	F32 maxDotAi = -UNREASONABLEFLOATNUMBER;
	for (U32 i = 0; i < 3; ++i)
	{
		if (AiDots[i] > maxDotAi)
		{
			maxPositionAi = i;
			maxDotAi = AiDots[i];
		}
	}
	vec3 maxLinePointA = vec3(AiPoints[maxPositionAi].x, AiPoints[maxPositionAi].y, AiPoints[maxPositionAi].z);
	vec3 maxA = maxLinePointA + (normalize(direction) * A.radius);

	return maxA;
}



template<typename S1, typename S2>
inline vec3 Support(S1 A, S2 B, vec3 direction)
{
	// find the point in A that has the largest value with dot(Ai, Direction)
	vec3 maxA = Support(A, direction);

	// find the point in B that has the largest value with dot(Bj, Direction)
	vec3 maxB = Support(B, -direction);

	vec3 result = maxA - maxB;
	return result;
}






struct Simplex
{
	enum SimplexType
	{
		Simplex_Empty = 0,
		Simplex_Point = 1,
		Simplex_Line = 2,
		Simplex_Triangle = 3,
		Simplex_Tetrahedron = 4
	};
	SimplexType type;

	vec3 A;
	vec3 B;
	vec3 C;
	vec3 D;
};

Simplex CreateSimplex(vec3 A)
{
	Simplex result;
	result.type = Simplex::Simplex_Point;
	result.A = A;
	return result;
}

Simplex CreateSimplex(vec3 B, vec3 A)
{
	Simplex result;
	result.type = Simplex::Simplex_Line;
	result.B = B;
	result.A = A;
	return result;
}

Simplex CreateSimplex(vec3 C, vec3 B, vec3 A)
{
	Simplex result;
	result.type = Simplex::Simplex_Triangle;
	result.C = C;
	result.B = B;
	result.A = A;
	return result;
}

Simplex CreateSimplex(vec3 D, vec3 C, vec3 B, vec3 A)
{
	Simplex result;
	result.type = Simplex::Simplex_Tetrahedron;
	result.D = D;
	result.C = C;
	result.B = B;
	result.A = A;
	return result;
}

void AddSimplexPoint(Simplex* simplex, vec3 point)
{
	Simplex::SimplexType simplexType = simplex->type;
	switch (simplexType)
	{
	case Simplex::Simplex_Empty:
	simplex->A = point;
	break;

	case Simplex::Simplex_Point:
	simplex->B = simplex->A;
	simplex->A = point;
	break;

	case Simplex::Simplex_Line:
	simplex->C = simplex->B;
	simplex->B = simplex->A;
	simplex->A = point;
	break;

	case Simplex::Simplex_Triangle:
	simplex->D = simplex->C;
	simplex->C = simplex->B;
	simplex->B = simplex->A;
	simplex->A = point;
	break;

	case Simplex::Simplex_Tetrahedron:
	default:
	Assert(true);
	break;
	}
	simplex->type = (Simplex::SimplexType)(simplexType + 1);
}



struct DoSimplexResult
{
	bool containsGoal;
	Simplex simplex;
	vec3 d;
};

inline DoSimplexResult DoSimplexLine(Simplex simplex, vec3 D)
{
	DoSimplexResult result;
	result.containsGoal = false;

	vec3 ab = vec3(simplex.B.x - simplex.A.x, simplex.B.y - simplex.A.y, simplex.B.z - simplex.A.z);
	vec3 ag = vec3(0 - simplex.A.x, 0 - simplex.A.y, 0 - simplex.A.z);

	result.d = cross(cross(ab, ag), ab);
	result.simplex = CreateSimplex(simplex.B, simplex.A);

	return result;
}

inline DoSimplexResult DoSimplexLine_Casey(Simplex simplex, vec3 D)
{
	DoSimplexResult result;
	result.containsGoal = false;

	vec3 ab = vec3(simplex.B.x - simplex.A.x, simplex.B.y - simplex.A.y, simplex.B.z - simplex.A.z);
	vec3 ag = vec3(0 - simplex.A.x, 0 - simplex.A.y, 0 - simplex.A.z);
	if (dot(ab, ag) > 0)
	{
		result.d = cross(cross(ab, ag), ab);
		result.simplex = CreateSimplex(simplex.B, simplex.A);
	}
	else
	{
		Assert(false);
		
		result.simplex = CreateSimplex(simplex.B);
	}

	return result;
}

inline DoSimplexResult DoSimplexLine_AllVoronoi(Simplex simplex, vec3 D)
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
		result.simplex = CreateSimplex(simplex.A);
		result.d = ag;
	}
	else if (dot(ba, bg) < 0)
	{
		// Voronoi Region B
		result.simplex = CreateSimplex(simplex.B);
		result.d = bg;
	}
	else
	{
		// Voronoi Region AB
		result.simplex = CreateSimplex(simplex.B, simplex.A);
		result.d = cross(cross(ab, ag), ab);
	}

	return result;
}

inline DoSimplexResult DoSimplexLine_AllVoronoiDouble(Simplex simplex, vec3 D)
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
		result.simplex = CreateSimplex(simplex.A);
		result.d = ag;
	}
	else if (dot(ba, bg) < 0)
	{
		// Voronoi Region B
		result.simplex = CreateSimplex(simplex.B);
		result.d = bg;
	}
	else
	{
		// Voronoi Region AB
		result.simplex = CreateSimplex(simplex.B, simplex.A);
		result.d = cross(cross(ab, ag), ab);
	}

	return result;
}



inline DoSimplexResult DoSimplexTriangle(Simplex simplex, vec3 D)
{
	DoSimplexResult result;
	result.containsGoal = false;

	vec3 ab = simplex.B - simplex.A;
	vec3 ac = simplex.C - simplex.A;
	vec3 ag = vec3(0, 0, 0) - simplex.A;
	vec3 abc = cross(ab, ac);

	if (dot(cross(abc, ac), ag) > 0)
	{
		// Case AC
		result.simplex = CreateSimplex(simplex.C, simplex.A);
		result.d = cross(cross(ac, ag), ac);
	}
	else
	{
		if (dot(cross(ab, abc), ag) > 0)
		{
			// Case AB
			result.simplex = CreateSimplex(simplex.B, simplex.A);
			result.d = cross(cross(ab, ag), ab);
		}
		else
		{
			if (dot(abc, ag) > 0)
			{
				// Case ABC
				result.simplex = CreateSimplex(simplex.C, simplex.B, simplex.A);
				result.d = abc;
			}
			else
			{
				// NOTE: Swap points so plane normal will be in the direction of the new point.
				result.simplex = CreateSimplex(simplex.B, simplex.C, simplex.A);
				result.d = -abc;
			}
		}
	}

	return result;
}

inline DoSimplexResult DoSimplexTriangle_Casey(Simplex simplex, vec3 D)
{
	DoSimplexResult result;
	result.containsGoal = false;

	vec3 ab = simplex.B - simplex.A;
	vec3 ac = simplex.C - simplex.A;
	vec3 ag = vec3(0, 0, 0) - simplex.A;
	vec3 abc = cross(ab, ac);

	if (dot(cross(abc, ac), ag) > 0)
	{
		if (dot(ac, ag) > 0)
		{
			// CASE 1
			// Case AC
			result.simplex = CreateSimplex(simplex.C, simplex.A);
			result.d = cross(cross(ac, ag), ac);
		}
		else
		{
			// Magical special fun times case
			if (dot(ab, ag) > 0)
			{
				// CASE 4
				// Case AB
				result.simplex = CreateSimplex(simplex.B, simplex.A);
				result.d = cross(cross(ab, ag), ab);
			}
			else
			{
				// CASE 5
				// Case A
				Assert(false);

				result.simplex = CreateSimplex(simplex.A);
				result.d = ag;
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
				// Case AB
				result.simplex = CreateSimplex(simplex.B, simplex.A);
				result.d = cross(cross(ab, ag), ab);
			}
			else
			{
				// CASE 5
				// Case A
				Assert(false);

				result.simplex = CreateSimplex(simplex.A);
				result.d = ag;
			}
		}
		else
		{
			// CASE 2 AND CASE 3

			// NOTE: In the 2D case this means the origin is within the triangle.
			result.containsGoal = true;
			result.simplex = CreateSimplex(simplex.C, simplex.B, simplex.A);

			// NOTE: This is for the 3D case.
			/*if (dot(abc, ag) > 0)
			{
				result.d = abc;
			}
			else
			{
				// NOTE: Swap points so plane normal will be in the direction of the new point.
				result.simplex.B = simplex.C;
				result.simplex.C = simplex.B;
				result.d = -abc;
			}*/
		}
	}

	return result;
}

inline DoSimplexResult DoSimplexTriangle_AllVoronoi(Simplex simplex, vec3 D)
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
	F32 sn_DOT_ag = dot(sn, ag);

	if (ab_DOT_ag < 0 &&
		ac_DOT_ag < 0)
	{
		// Voronoi Region A
		result.simplex = CreateSimplex(simplex.A);
		result.d = ag;
	}
	else if (ba_DOT_bg < 0 &&
		bc_DOT_bg < 0)
	{
		// Voronoi Region B
		result.simplex = CreateSimplex(simplex.B);
		result.d = bg;
	}
	else if (ca_DOT_cg < 0 &&
		cb_DOT_cg < 0)
	{
		// Voronoi Region C
		result.simplex = CreateSimplex(simplex.C);
		result.d = cg;
	}
	else if (ab_DOT_ag >= 0 &&
		ba_DOT_bg >= 0 &&
		perpAB_DOT_ag > 0)
	{
		// Voronoi Region AB
		result.simplex = CreateSimplex(simplex.B, simplex.A);
		result.d = cross(cross(ab, ag), ab);
	}
	else if (ac_DOT_ag >= 0 &&
		ca_DOT_cg >= 0 &&
		perpAC_DOT_ag > 0)
	{
		// Voronoi Region AC
		result.simplex = CreateSimplex(simplex.C, simplex.A);
		result.d = cross(cross(ac, ag), ac);
	}
	else if (bc_DOT_bg >= 0 &&
		cb_DOT_cg >= 0 &&
		perpBC_DOT_bg > 0)
	{
		// Voronoi Region BC
		result.simplex = CreateSimplex(simplex.C, simplex.B);
		result.d = cross(cross(bc, bg), bc);
	}
	else if (perpAB_DOT_ag <= 0 &&
		perpAC_DOT_ag <= 0 &&
		perpBC_DOT_bg <= 0 &&
		sn_DOT_ag > 0)
	{
		// Voronoi Region ABC Above
		result.simplex = CreateSimplex(simplex.B, simplex.C, simplex.A);
		result.d = sn;
	}
	else if (perpAB_DOT_ag <= 0 &&
		perpAC_DOT_ag <= 0 &&
		perpBC_DOT_bg <= 0 &&
		sn_DOT_ag <= 0)
	{
		// Voronoi Region ABC Below
		result.simplex = CreateSimplex(simplex.C, simplex.B, simplex.A);
		result.d = -sn;
	}

	return result;
}

inline DoSimplexResult DoSimplexTriangle_AllVoronoiDouble(Simplex simplex, vec3 D)
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
	F64 sn_DOT_ag = dot(sn, ag);

	if (ab_DOT_ag < 0 &&
		ac_DOT_ag < 0)
	{
		// Voronoi Region A
		result.simplex = CreateSimplex(simplex.A);
		result.d = ag;
	}
	else if (ba_DOT_bg < 0 &&
		bc_DOT_bg < 0)
	{
		// Voronoi Region B
		result.simplex = CreateSimplex(simplex.B);
		result.d = bg;
	}
	else if (ca_DOT_cg < 0 &&
		cb_DOT_cg < 0)
	{
		// Voronoi Region C
		result.simplex = CreateSimplex(simplex.C);
		result.d = cg;
	}
	else if (ab_DOT_ag >= 0 &&
		ba_DOT_bg >= 0 &&
		perpAB_DOT_ag > 0)
	{
		// Voronoi Region AB
		result.simplex = CreateSimplex(simplex.B, simplex.A);
		result.d = cross(cross(ab, ag), ab);
		if (result.d == vec3(0, 0, 0))
		{
			result.d = cross(sn, ac);
		}
	}
	else if (ac_DOT_ag >= 0 &&
		ca_DOT_cg >= 0 &&
		perpAC_DOT_ag > 0)
	{
		// Voronoi Region AC
		result.simplex = CreateSimplex(simplex.C, simplex.A);
		result.d = normalize(cross(cross(ac, ag), ac));
		if (result.d == vec3(0, 0, 0))
		{
			result.d = cross(ab, sn);
		}
	}
	else if (bc_DOT_bg >= 0 &&
		cb_DOT_cg >= 0 &&
		perpBC_DOT_bg > 0)
	{
		// Voronoi Region BC
		result.simplex = CreateSimplex(simplex.C, simplex.B);
		result.d = cross(cross(bc, bg), bc);
		if (result.d == vec3(0,0,0))
		{
			result.d = cross(bc, sn);
		}
	}
	else if (perpAB_DOT_ag <= 0 &&
		perpAC_DOT_ag <= 0 &&
		perpBC_DOT_bg <= 0 &&
		sn_DOT_ag > 0)
	{
		// Voronoi Region ABC Above
		result.simplex = CreateSimplex(simplex.B, simplex.C, simplex.A);
		result.d = sn;
	}
	else if (perpAB_DOT_ag <= 0 &&
		perpAC_DOT_ag <= 0 &&
		perpBC_DOT_bg <= 0 &&
		sn_DOT_ag <= 0)
	{
		// Voronoi Region ABC Below
		result.simplex = CreateSimplex(simplex.C, simplex.B, simplex.A);
		result.d = -sn;
	}

	result.d = normalize(result.d);
	return result;
}



inline DoSimplexResult DoSimplexTetrahedron(Simplex simplex, vec3 D)
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

	
	// NOTE: Sign on the plane tests may need to be >= but i dont think so, investigate if errors crop up
	if (dot(AG, AB) >= 0 &&
		dot(BG, BA) >= 0 &&
		dot(AG, cross(AB, normalABC))>0 &&
		dot(AG, cross(normalADB, AB)) > 0)
	{
		// Voronoi AB
		result.simplex = CreateSimplex(simplex.B, simplex.A);
		result.d = cross(cross(AB, AG), AB);
	}
	else if (dot(AG, AC) >= 0 &&
		dot(CG, CA) >= 0 &&
		dot(AG, cross(AC, normalACD)) > 0 &&
		dot(AG, cross(normalABC, AC)) > 0)
	{
		// Voronoi AC
		result.simplex = CreateSimplex(simplex.C, simplex.A);
		result.d = cross(cross(AC, AG), AC);
	}
	else if (dot(AG, AD) >= 0 &&
		dot(DG, DA) >= 0 &&
		dot(AG, cross(AD, normalADB)) > 0 &&
		dot(AG, cross(normalACD, AD)) > 0)
	{
		// Voronoi AD
		result.simplex = CreateSimplex(simplex.D, simplex.A);
		result.d = cross(cross(AD, AG), AD);
	}
	else if (dot(BG, BC) >= 0 &&
		dot(CG, CB) >= 0 &&
		dot(BG, cross(BC, normalBCA)) > 0 &&
		dot(BG, cross(normalBDC, BC)) > 0)
	{
		// Voronoi BC
		result.simplex = CreateSimplex(simplex.C, simplex.B);
		result.d = cross(cross(BC, BG), BC);
	}
	else if (dot(BG, BD) >= 0 &&
		dot(DG, DB) >= 0 &&
		dot(BG, cross(BD, normalBDC)) > 0 &&
		dot(BG, cross(normalBAD, BD)) > 0)
	{
		// Voronoi BD
		result.simplex = CreateSimplex(simplex.D, simplex.B);
		result.d = cross(cross(BD, BG), BD);
	}
	else if ((dot(AG, normalABC))*(dot(AD, normalABC))<0)
	{
		// Voronoi ABC
		result.simplex = CreateSimplex(simplex.C, simplex.B, simplex.A);
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
		result.simplex = CreateSimplex(simplex.D, simplex.B, simplex.A);
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
		result.simplex = CreateSimplex(simplex.D, simplex.C, simplex.A);
		if (dot(AG, normalACD)>0)
		{
			result.d = normalACD;
		}
		else
		{
			result.d = -normalACD;
		}

	}
	else
	{
		// Voronoi ABCD
		result.containsGoal = true;
		result.simplex = CreateSimplex(simplex.D, simplex.C, simplex.B, simplex.A);
		// NOTE: If d is needed from here remember it is not set.
	}

	return result;
}

inline DoSimplexResult DoSimplexTetrahedron_AllVoronoi(Simplex simplex, vec3 D)
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
		result.simplex = CreateSimplex(simplex.A);
		result.d = AG;
	}
	else if (dot(BG, BA)<0 &&
		dot(BG, BC)<0 &&
		dot(BG, BD)<0)
	{
		// Voronoi B
		result.simplex = CreateSimplex(simplex.B);
		result.d = BG;
	}
	else if (dot(CG, CA)<0 &&
		dot(CG, CB)<0 &&
		dot(CG, CD)<0)
	{
		// Voronoi C
		result.simplex = CreateSimplex(simplex.C);
		result.d = CG;
	}
	else if (dot(DG, DA)<0 &&
		dot(DG, DB)<0 &&
		dot(DG, DC)<0)
	{
		// Voronoi D
		result.simplex = CreateSimplex(simplex.D);
		result.d = DG;
	}
	// NOTE: Sign on the plane tests may need to be >= but i dont think so, investigate if errors crop up
	else if (dot(AG, AB) >= 0 &&
		dot(BG, BA) >= 0 &&
		dot(AG, cross(AB, normalABC))>0 &&
		dot(AG, cross(normalADB, AB))>0)
	{
		// Voronoi AB
		result.simplex = CreateSimplex(simplex.B, simplex.A);
		result.d = cross(cross(AB, AG), AB);
	}
	else if (dot(AG, AC) >= 0 &&
		dot(CG, CA) >= 0 &&
		dot(AG, cross(AC, normalACD))>0 &&
		dot(AG, cross(normalABC, AC))>0)
	{
		// Voronoi AC
		result.simplex = CreateSimplex(simplex.C, simplex.A);
		result.d = cross(cross(AC, AG), AC);
	}
	else if (dot(AG, AD) >= 0 &&
		dot(DG, DA) >= 0 &&
		dot(AG, cross(AD, normalADB))>0 &&
		dot(AG, cross(normalACD, AD))>0)
	{
		// Voronoi AD
		result.simplex = CreateSimplex(simplex.D, simplex.A);
		result.d = cross(cross(AD, AG), AD);
	}
	else if (dot(BG, BC) >= 0 &&
		dot(CG, CB) >= 0 &&
		dot(BG, cross(BC, normalBCA))>0 &&
		dot(BG, cross(normalBDC, BC))>0)
	{
		// Voronoi BC
		result.simplex = CreateSimplex(simplex.C, simplex.B);
		result.d = cross(cross(BC, BG), BC);
	}
	else if (dot(BG, BD) >= 0 &&
		dot(DG, DB) >= 0 &&
		dot(BG, cross(BD, normalBDC))>0 &&
		dot(BG, cross(normalBAD, BD))>0)
	{
		// Voronoi BD
		result.simplex = CreateSimplex(simplex.D, simplex.B);
		result.d = cross(cross(BD, BG), BD);
	}
	else if (dot(CG, CD) >= 0 &&
		dot(DG, DC) >= 0 &&
		dot(BG, cross(BD, normalBDC))>0 &&
		dot(BG, cross(normalBAD, BD))>0)
	{
		// Voronoi CD
		result.simplex = CreateSimplex(simplex.D, simplex.C); 
		result.d = cross(cross(CD, CG), CD);
	}
	else if ((dot(AG, normalABC))*(dot(AD, normalABC))<0)
	{
		// Voronoi ABC
		result.simplex = CreateSimplex(simplex.C, simplex.B, simplex.A);
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
		result.simplex = CreateSimplex(simplex.D, simplex.B, simplex.A);
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
		result.simplex = CreateSimplex(simplex.D, simplex.C, simplex.A);
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
		result.simplex = CreateSimplex(simplex.D, simplex.C, simplex.B);
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
		result.simplex = CreateSimplex(simplex.D, simplex.C, simplex.B, simplex.A);
		// NOTE: If d is needed from here remember it is not set.
	}

	return result;
}



inline DoSimplexResult DoSimplex(Simplex simplex, vec3 D)
{
	DoSimplexResult result;
	result.containsGoal = false;

	switch (simplex.type)
	{
	case Simplex::Simplex_Line:
	{
								  result = DoSimplexLine(simplex, D);
								  break;
	}

	case Simplex::Simplex_Triangle:
	{
									  result = DoSimplexTriangle(simplex, D);
									  break;
	}

	case Simplex::Simplex_Tetrahedron:
	{
								result = DoSimplexTetrahedron(simplex, D);
								break;
	}

	default:
	{
			   break;
	}
	}

	return result;
}

inline DoSimplexResult DoSimplex_Casey(Simplex simplex, vec3 D)
{
	DoSimplexResult result;
	result.containsGoal = false;

	switch (simplex.type)
	{
	case Simplex::Simplex_Line:
	{
								  result = DoSimplexLine_Casey(simplex, D);
							      break;
	}

	case Simplex::Simplex_Triangle:
	{
									  result = DoSimplexTriangle_Casey(simplex, D);
									  break;
	}

	case Simplex::Simplex_Tetrahedron:
	{
										 result.containsGoal = true; // = DoSimplexTetrahedron(simplex, D);
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
inline DoSimplexResult DoSimplex_AllVoronoi(Simplex simplex, vec3 D)
{
	DoSimplexResult result;
	result.containsGoal = false;

	switch (simplex.type)
	{
	case Simplex::Simplex_Line:
	{
#ifndef DOUBLES
						 result = DoSimplexLine_AllVoronoi(simplex, D);
#else
						 result = DoSimplexLine_AllVoronoiDouble(simplex, D);
#endif
						 break;
	}

	case Simplex::Simplex_Triangle:
	{
#ifndef DOUBLES
							 result = DoSimplexTriangle_AllVoronoi(simplex, D);
#else
							 result = DoSimplexTriangle_AllVoronoiDouble(simplex, D);
#endif
							 break;
	}

	case Simplex::Simplex_Tetrahedron:
	{
								result = DoSimplexTetrahedron_AllVoronoi(simplex, D);
								break;
	}

	default:
	{
			   break;
	}
	}

	return result;
}





struct GJKInfo
{
	bool collided;
	U32 numberOfLoopsCompleted;
	Simplex simplex;
};

template<typename S1, typename S2>
inline GJKInfo GJK(S1 shapeA, S2 shapeB)
{
	bool collisionDetected = false;

	vec3 S = Support(shapeA, shapeB, vec3(1.0f, -1.0f, -1.0f));
	if (S == vec3(0, 0, 0))
	{
		GJKInfo result;
		result.collided = true;
		result.numberOfLoopsCompleted = 0;
		result.simplex.type = Simplex::Simplex_Point;
		result.simplex.A = S;
		return result;
	}
	Simplex simplex = CreateSimplex(S);
	vec3 D = -S;

	U32 maxNumberOfLoops = 20;
	U32 loopCounter = 0;
	for (; loopCounter < maxNumberOfLoops;)
	{
		++loopCounter;
		vec3 A = Support(shapeA, shapeB, D);
		if (A == vec3(0.0f, 0.0f, 0.0f))
		{
			collisionDetected = true;
			break;
		}
		if (dot(A, D) < 0)
		{
			collisionDetected = false;
			break;
		}
		AddSimplexPoint(&simplex, A);
		DoSimplexResult dsr = DoSimplex(simplex, D);
		simplex = dsr.simplex;
		D = dsr.d;
		if (dsr.containsGoal)
		{
			collisionDetected = true;
			break;
		}
	}

	if (loopCounter == maxNumberOfLoops)
	{
		collisionDetected = true;
		DebugPrint("GJK: collision exceeded max number of iterations.\n");
	}

// 	if (collisionDetected)
// 	{
// 		switch (simplexType)
// 		{
// 		case Simplex::Simplex_Point:
// 		{
// 							  Assert(true);
// 		}
// 		break;
// 
// 		case Simplex::Simplex_Line:
// 		{
// 							 RandomNumberGenerator randomNumberGenerator;
// 							 SeedRandomNumberGenerator(&randomNumberGenerator, 1);
// 							 simplex[2] = simplex[0];
// 							 while (simplex[2] == simplex[0] || simplex[2] == simplex[1])
// 							 {
// 								vec3 randomDirection = vec3(RandomF32Between(&randomNumberGenerator, -1, 1), RandomF32Between(&randomNumberGenerator, -1, 1), RandomF32Between(&randomNumberGenerator, -1, 1));
// 								simplex[2] = Support(&shapeA, &shapeB, &randomDirection);
// 							 }
// 
// 							 simplexType = Simplex::Simplex_Tetrahedron;
// 							 vec3 AB = simplex[1] - simplex[2];
// 							 vec3 AC = simplex[0] - simplex[2];
// 							 vec3 normal_ABC = cross(AB, AC);
// 							 simplex[3] = Support(&shapeA, &shapeB, &normal_ABC); 
// 		}
// 		break;
// 
// 		case Simplex::Simplex_Triangle:
// 		{
// 								 simplexType = Simplex::Simplex_Tetrahedron;
// 								 vec3 AB = simplex[1] - simplex[2];
// 								 vec3 AC = simplex[0] - simplex[2];
// 								 vec3 normal_ABC = cross(AB, AC);
// 								 simplex[3] = Support(&shapeA, &shapeB, &normal_ABC);
// 		}
// 		break;
// 
// 		default:
// 		break;
// 		}
// 	}

	GJKInfo result;
	result.collided = collisionDetected;
	result.numberOfLoopsCompleted = loopCounter;
	result.simplex = simplex;

	return result;
}

template<typename S1, typename S2>
inline GJKInfo GJK_Casey(S1 shapeA, S2 shapeB)
{
	bool collisionDetected = false;

	vec3 S = Support(shapeA, shapeB, vec3(1.0f, -1.0f, -1.0f));
	if (S == vec3(0, 0, 0))
	{
		GJKInfo result;
		result.collided = true;
		result.numberOfLoopsCompleted = 0;
		result.simplex.type = Simplex::Simplex_Point;
		result.simplex.A = S;
		return result;
	}
	Simplex simplex = CreateSimplex(S);
	vec3 D = -S;

	U32 loopCounter = 0;
	for (; loopCounter < 10;)
	{
		++loopCounter;
		vec3 A = Support(shapeA, shapeB, D);
		if (A == vec3(0.0f, 0.0f, 0.0f))
		{
			collisionDetected = true;
			break;
		}
		if (dot(A, D) < 0)
		{
			collisionDetected = false;
			break;
		}
		AddSimplexPoint(&simplex, A);
		DoSimplexResult dsr = DoSimplex_Casey(simplex, D);
		simplex = dsr.simplex;
		D = dsr.d;
		if (dsr.containsGoal)
		{
			collisionDetected = true;
			break;
		}
	}

	GJKInfo result;
	result.collided = collisionDetected;
	result.numberOfLoopsCompleted = loopCounter;
	result.simplex = simplex;

	return result;
}

template<typename S1, typename S2>
inline GJKInfo GJK_AllVoronoi(S1 shapeA, S2 shapeB)
{
	bool collisionDetected = false;

	vec3 S = Support(shapeA, shapeB, vec3(1.0f, -1.0f, -1.0f));
	if (S == vec3(0, 0, 0))
	{
		GJKInfo result;
		result.collided = true;
		result.numberOfLoopsCompleted = 0;
		result.simplex.type = Simplex::Simplex_Point;
		result.simplex.A = S;
		return result;
	}
	Simplex simplex = CreateSimplex(S);
	vec3 D = -S;

	U32 maxNumberOfLoops = 20;
	U32 loopCounter = 0;
	for (; loopCounter < maxNumberOfLoops;)
	{
		++loopCounter;
		vec3 A = Support(shapeA, shapeB, D);
		if (A == vec3(0.0f, 0.0f, 0.0f))
		{
			collisionDetected = true;
			break;
		}
		if (dot(A, D) < 0)
		{
			collisionDetected = false;
			break;
		}
		AddSimplexPoint(&simplex, A);
		DoSimplexResult dsr = DoSimplex_AllVoronoi(simplex, D);
		simplex = dsr.simplex;
		D = dsr.d;
		if (dsr.containsGoal)
		{
			collisionDetected = true;
			break;
		}
	}

	if (loopCounter == maxNumberOfLoops)
	{
		collisionDetected = true;
		DebugPrint("GJK_AllVoronoi: collision exceeded max number of iterations.\n");
	}

	GJKInfo result;
	result.collided = collisionDetected;
	result.numberOfLoopsCompleted = loopCounter;
	result.simplex = simplex;

	return result;
}





struct EPAInfo
{
	vec3 normal;
	F32 distance;
};

struct Face
{
	vec3 A;
	vec3 B;
	vec3 C;

	vec3 normal;
	F32 distance;
};

template<typename S1, typename S2>
inline EPAInfo EPA(S1 shapeA, S2 shapeB, Simplex simplex, F32 tolerance)
{
	EPAInfo result;

	U32 polytopeFaces_Count = 4;
	const U32 polytopeFaces_MaxCount = 100;
	Face polytopeFaces[polytopeFaces_MaxCount];

	// for each face in the simplex passed in
	// NOTE: I believe the simplex passed from GJK is wound CCW so these front/back face checks can likely be removed in optimization phase.
	// if the normal dot direction to the origin is negative
	// we have the front face and should record it. 
	// record with CCW winding 
	// ADC
	vec3 AD = simplex.D - simplex.A;
	vec3 AC = simplex.C - simplex.A;
	vec3 normal_ACD = normalize(cross(AC, AD));
	vec3 AG = vec3(0, 0, 0) - simplex.A;
	vec3 AB = simplex.B - simplex.A;
	F32 normal_ACD_DOT_AB = dot(normal_ACD, AB);
	F32 normal_ACD_DOT_AG = dot(normal_ACD, AG);
	if (normal_ACD_DOT_AB < 0)
	{
		polytopeFaces[0].A = simplex.A;
		polytopeFaces[0].B = simplex.C;
		polytopeFaces[0].C = simplex.D;
		polytopeFaces[0].normal = normal_ACD;
	}
	else
	{
		polytopeFaces[0].A = simplex.A;
		polytopeFaces[0].B = simplex.D;
		polytopeFaces[0].C = simplex.C;
		polytopeFaces[0].normal = -normal_ACD;
	}
	// ACB
	AB = simplex.B - simplex.A;
	vec3 normal_ABC = normalize(cross(AB, AC));
	F32 normal_ABC_DOT_AD = dot(normal_ABC, AD);
	F32 normal_ABC_DOT_AG = dot(normal_ABC, AG);
	if (normal_ABC_DOT_AD < 0)
	{
		polytopeFaces[1].A = simplex.A;
		polytopeFaces[1].B = simplex.B;
		polytopeFaces[1].C = simplex.C;
		polytopeFaces[1].normal = normal_ABC;
	}
	else
	{
		polytopeFaces[1].A = simplex.A;
		polytopeFaces[1].B = simplex.C;
		polytopeFaces[1].C = simplex.B;
		polytopeFaces[1].normal = -normal_ABC;
	}
	// ABD
	vec3 normal_ABD = normalize(cross(AB, AD));
	F32 normal_ABD_DOT_AC = dot(normal_ABD, AC);
	F32 normal_ABD_DOT_AG = dot(normal_ABD, AG);
	if (normal_ABD_DOT_AC < 0)
	{
		polytopeFaces[2].A = simplex.A;
		polytopeFaces[2].B = simplex.B;
		polytopeFaces[2].C = simplex.D;
		polytopeFaces[2].normal = normal_ABD;
	}
	else
	{
		polytopeFaces[2].A = simplex.A;
		polytopeFaces[2].B = simplex.D;
		polytopeFaces[2].C = simplex.B;
		polytopeFaces[2].normal = -normal_ABD;
	}
	// DCB
	vec3 BC = simplex.C - simplex.B;
	vec3 BD = simplex.D - simplex.B;
	vec3 BG = vec3(0, 0, 0) - simplex.B;
	vec3 normal_BCD = normalize(cross(BC, BD));
	vec3 BA = simplex.A - simplex.B;
	F32 normal_BCD_DOT_BA = dot(normal_BCD, BA);
	F32 normal_BCD_DOT_AG = dot(normal_BCD, BG);
	if (normal_BCD_DOT_AG < 0)
	{
		polytopeFaces[3].A = simplex.B;
		polytopeFaces[3].B = simplex.C;
		polytopeFaces[3].C = simplex.D;
		polytopeFaces[3].normal = normal_BCD;
	}
	else
	{
		polytopeFaces[3].A = simplex.B;
		polytopeFaces[3].B = simplex.D;
		polytopeFaces[3].C = simplex.C;
		polytopeFaces[3].normal = -normal_BCD;
	}

	polytopeFaces[0].distance = dot(polytopeFaces[0].A, polytopeFaces[0].normal);
	polytopeFaces[1].distance = dot(polytopeFaces[1].A, polytopeFaces[1].normal);
	polytopeFaces[2].distance = dot(polytopeFaces[2].A, polytopeFaces[2].normal);
	polytopeFaces[3].distance = dot(polytopeFaces[3].A, polytopeFaces[3].normal);

	// while
	// find the face with the lowest distance to the origin
	// if the distance is less than the tolerance 
	// we have found what we are looking for, record it and exit
	// else 
	// find a new support point in the direction of the surface normal 
	// remove the face we had tested from our list of faces.
	// add the 3 new faces to the list of faces.


	// NOTE: Bounds??? 
	while (polytopeFaces_Count + 2 < polytopeFaces_MaxCount)
	{
		Face closestFace = polytopeFaces[0];
		U32 closestFace_Index = 0;
		for (U32 i = 0; i < polytopeFaces_Count; ++i)
		{
			if (polytopeFaces[i].distance < closestFace.distance)
			{
				closestFace = polytopeFaces[i];
				closestFace_Index = i;
			}
		}

		vec3 newPolytopePoint = Support(&shapeA, &shapeB, &closestFace.normal);
		F32 newPolytopePoint_Distance = dot(newPolytopePoint, closestFace.normal);
		if (newPolytopePoint_Distance - closestFace.distance < tolerance)
		{
			result.normal = closestFace.normal;
			result.distance = newPolytopePoint_Distance;

			break;
		}
		else
		{
			U32 indices[3] = {closestFace_Index, polytopeFaces_Count, polytopeFaces_Count+1};
			Face faces[3];


			// New A B
			faces[0].A = newPolytopePoint;
			faces[0].B = closestFace.A;
			faces[0].C = closestFace.B;

// 			vec3 AB = polytopeFaces[closestFace_Index].B - polytopeFaces[closestFace_Index].A;
// 			vec3 AC = polytopeFaces[closestFace_Index].C - polytopeFaces[closestFace_Index].A;
// 			vec3 normal_ABC = cross(AB, AC);
// 			polytopeFaces[closestFace_Index].normal = normalize(normal_ABC);
// 			polytopeFaces[closestFace_Index].distance = dot(polytopeFaces[closestFace_Index].A, polytopeFaces[closestFace_Index].normal);



			// New B C
			faces[1].A = newPolytopePoint;
			faces[1].B = closestFace.B;
			faces[1].C = closestFace.C;

// 			AB = polytopeFaces[polytopeFaces_Count].B - polytopeFaces[polytopeFaces_Count].A;
// 			AC = polytopeFaces[polytopeFaces_Count].C - polytopeFaces[polytopeFaces_Count].A;
// 			normal_ABC = cross(AB, AC);
// 			polytopeFaces[polytopeFaces_Count].normal = normalize(normal_ABC);
// 			polytopeFaces[polytopeFaces_Count].distance = dot(polytopeFaces[polytopeFaces_Count].A, polytopeFaces[polytopeFaces_Count].normal);



			// New C A
			faces[2].A = newPolytopePoint;
			faces[2].B = closestFace.C;
			faces[2].C = closestFace.A;

// 			AB = polytopeFaces[polytopeFaces_Count + 1].B - polytopeFaces[polytopeFaces_Count + 1].A;
// 			AC = polytopeFaces[polytopeFaces_Count + 1].C - polytopeFaces[polytopeFaces_Count + 1].A;
// 			normal_ABC = cross(AB, AC);
// 			polytopeFaces[polytopeFaces_Count + 1].normal = normalize(normal_ABC);
// 			polytopeFaces[polytopeFaces_Count + 1].distance = dot(polytopeFaces[polytopeFaces_Count + 1].A, polytopeFaces[polytopeFaces_Count + 1].normal);

//			polytopeFaces_Count += 2;

			U32 uniqueCount = 0;
			for (U32 i = 0; i < 3; ++i)
			{
				Face* face = &faces[i];
				vec3 AB = face->B - face->A;
				vec3 AC = face->C - face->A;
				face->normal = normalize(cross(AB, AC));

				bool unique = true;
				for (U32 j = 0; j < polytopeFaces_Count; ++j)
				{
					if (polytopeFaces[j].normal == face->normal)
					{
						unique = false;
						break;
					}
				}

				if (unique)
				{
					face->distance = dot(face->A, face->normal);
					polytopeFaces[indices[uniqueCount++]] = *face;
				}

			}

			polytopeFaces_Count += uniqueCount - 1;
		}
		// if npp projected onto the face normal is within the tolerance range then we have our feature
		// ie if the distane to the new point - the distance to the face is less than the tolerance STOP
		// otherwise add the 3 new faces and delete the old face

		// np a b
		// np b c
		// np c a
	}

	return result;
}






struct CollisionInfo
{
	bool collided;
	vec3 normal;
	F32 distance;
};

template<typename S1, typename S2>
inline CollisionInfo DetectCollision(S1 shapeA, S2 shapeB)
{
	CollisionInfo result;
	result.collided = false;

	GJKInfo gjkInfo = GJK(shapeA, shapeB);
	if (gjkInfo.collided)
	{
// 		EPAInfo epaInfo = EPA(shapeA, shapeB, gjkInfo.simplex, .0001);
// 

		result.collided = true;
// 		result.normal = epaInfo.normal;
// 		result.distance = epaInfo.distance;
	}

	return result;
}

template<typename S1, typename S2>
inline CollisionInfo DetectCollision_Casey(S1 shapeA, S2 shapeB)
{
	CollisionInfo result;
	result.collided = false;

	GJKInfo gjkInfo = GJK_Casey(shapeA, shapeB);
	if (gjkInfo.collided)
	{
// 		EPAInfo epaInfo = EPA(shapeA, shapeB, gjkInfo.simplex, .0001);
//
		result.collided = true;
// 		result.normal = epaInfo.normal;
// 		result.distance = epaInfo.distance;
	}

	return result;
}

template<typename S1, typename S2>
inline CollisionInfo DetectCollision_AllVoronoi(S1 shapeA, S2 shapeB)
{
	CollisionInfo result;
	result.collided = false;

	GJKInfo gjkInfo = GJK_AllVoronoi(shapeA, shapeB);
	if (gjkInfo.collided)
	{
		// 		EPAInfo epaInfo = EPA(shapeA, shapeB, gjkInfo.simplex, .0001);
		//
		result.collided = true;
		// 		result.normal = epaInfo.normal;
		// 		result.distance = epaInfo.distance;
	}

	return result;
}