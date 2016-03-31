#pragma once

// Scaling Circles??? 
// what spaces are the points in?

#include "Types.h"
#include "Math.h"

#include "DebugAPI.h"

// General Constants
#define UNREASONABLEFLOATNUMBER 100000.0f

// GJK Constants
#define MAXGJKITERATIONS_2D 128

// EPA Constants
#define MAXEPAITERATIONS_2D 128
#define EPATOLERANCE .0001






struct Transform
{
	mat3 transform;
};

struct Rectangle_2D
{
	vec2 halfDim;

	Rectangle_2D()
	{
		halfDim = vec2(1, 1);
	}
};

inline vec2 Support(Rectangle_2D A, Transform transform, vec2 direction)
{
	vec3 AiPoints[4] = {
		transform.transform * vec3(- A.halfDim.x, - A.halfDim.y, 1.0f),
		transform.transform * vec3(- A.halfDim.x,   A.halfDim.y, 1.0f),
		transform.transform * vec3(  A.halfDim.x, - A.halfDim.y, 1.0f),
		transform.transform * vec3(  A.halfDim.x,   A.halfDim.y, 1.0f)
	};
	F32 AiDots[4] = {
		dot(direction, vec2(AiPoints[0].x, AiPoints[0].y)),
		dot(direction, vec2(AiPoints[1].x, AiPoints[1].y)),
		dot(direction, vec2(AiPoints[2].x, AiPoints[2].y)),
		dot(direction, vec2(AiPoints[3].x, AiPoints[3].y))
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
	vec2 maxA = vec2(AiPoints[maxPositionAi].x, AiPoints[maxPositionAi].y);

	return maxA;
}



struct Circle_2D
{
	F32  radius;

	Circle_2D()
	{
		radius = 1;
	}
};

inline vec2 Support(Circle_2D A, Transform transform, vec2 direction)
{
	// NOTE: Should project direction vector onto plane of the sphere
	direction = normalize(direction);
	vec2 translationVector = vec2(transform.transform[2].x, transform.transform[2].y);
	vec2 maxA = (A.radius * direction) + translationVector;

	return maxA;
}



struct Triangle_2D
{
	vec2 points[3];

	Triangle_2D()
	{
		points[0] = vec2(-0.6f, -0.3f);
		points[1] = vec2(0.6f, -0.3f);
		points[2] = vec2(0.0f, 0.6f);
	}
};

inline vec2 Support(Triangle_2D A, Transform transform, vec2 direction)
{
	vec3 AiPoints[3] = {
		transform.transform * vec3(A.points[0].x, A.points[0].y, 1.0f),
		transform.transform * vec3(A.points[1].x, A.points[1].y, 1.0f),
		transform.transform * vec3(A.points[2].x, A.points[2].y, 1.0f)
	};
	F32 AiDots[3] = {
		dot(direction, vec2(AiPoints[0].x, AiPoints[0].y)),
		dot(direction, vec2(AiPoints[1].x, AiPoints[1].y)),
		dot(direction, vec2(AiPoints[2].x, AiPoints[2].y))
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
	vec2 maxA = vec2(AiPoints[maxPositionAi].x, AiPoints[maxPositionAi].y);

	return maxA;
}



template<typename S1, typename S2>
inline vec2 Support(S1 A, S2 B, vec2 direction)
{
	// find the point in A that has the largest value with dot(Ai, Direction)
	vec2 maxA = Support(A, direction);

	// find the point in B that has the largest value with dot(Bj, Direction)
	vec2 maxB = Support(B, -direction);

	vec2 result = maxA - maxB;
	return result;
}






struct Simplex_2D
{
	enum SimplexType_2D
	{
		Simplex_Empty_2D = 0,
		Simplex_Point_2D = 1,
		Simplex_Line_2D = 2,
		Simplex_Triangle_2D = 3
	};
	SimplexType_2D type;

	vec2 A;
	vec2 B;
	vec2 C;
};

Simplex_2D CreateSimplex(vec2 A)
{
	Simplex_2D result;
	result.type = Simplex_2D::Simplex_Point_2D;
	result.A = A;
	return result;
}

Simplex_2D CreateSimplex(vec2 B, vec2 A)
{
	Simplex_2D result;
	result.type = Simplex_2D::Simplex_Line_2D;
	result.B = B;
	result.A = A;
	return result;
}

Simplex_2D CreateSimplex(vec2 C, vec2 B, vec2 A)
{
	Simplex_2D result;
	result.type = Simplex_2D::Simplex_Triangle_2D;
	result.C = C;
	result.B = B;
	result.A = A;
	return result;
}

void AddSimplexPoint(Simplex_2D* simplex, vec2 point)
{
	Simplex_2D::SimplexType_2D simplexType = simplex->type;
	switch (simplexType)
	{
	case Simplex_2D::Simplex_Empty_2D:
	simplex->A = point;
	break;

	case Simplex_2D::Simplex_Point_2D:
	simplex->B = simplex->A;
	simplex->A = point;
	break;

	case Simplex_2D::Simplex_Line_2D:
	simplex->C = simplex->B;
	simplex->B = simplex->A;
	simplex->A = point;
	break;

	case Simplex_2D::Simplex_Triangle_2D:
	default:
	Assert(true);
	break;
	}
	simplex->type = (Simplex_2D::SimplexType_2D)(simplexType + 1);
}



struct DoSimplexResult_2D
{
	bool containsGoal;
	Simplex_2D simplex;
	vec2 direction;
};

inline DoSimplexResult_2D DoSimplexLine(Simplex_2D simplex)
{
	DoSimplexResult_2D result;
	result.containsGoal = false;

	vec2 ab = vec2(simplex.B.x - simplex.A.x, simplex.B.y - simplex.A.y);
	vec2 ag = vec2(0 - simplex.A.x, 0 - simplex.A.y);

	vec3 ab3 = vec3(ab, 0);
	vec3 ag3 = vec3(ag, 0);
	vec3 direction = cross(cross(ab3, ag3), ab3);

	result.direction = vec2(direction.x, direction.y);
	result.simplex = CreateSimplex(simplex.B, simplex.A);

	return result;
}

inline DoSimplexResult_2D DoSimplexTriangle(Simplex_2D simplex)
{
	DoSimplexResult_2D result;
	result.containsGoal = false;

	vec2 ab = simplex.B - simplex.A;
	vec2 ac = simplex.C - simplex.A;
	vec2 ag = vec2(0, 0) - simplex.A;

	vec3 ab3 = vec3(ab, 0);
	vec3 ac3 = vec3(ac, 0);
	vec3 ag3 = vec3(ag, 0);
	vec3 abc = cross(ab3, ac3);

	if (dot(cross(abc, ac3), ag3) > 0)
	{
		// Case AC
		result.simplex = CreateSimplex(simplex.C, simplex.A);

		vec3 direction = cross(cross(ac3, ag3), ac3);
		result.direction = vec2(direction.x, direction.y);
	}
	else
	{
		if (dot(cross(ab3, abc), ag3) > 0)
		{
			// Case AB
			result.simplex = CreateSimplex(simplex.B, simplex.A);

			vec3 direction = cross(cross(ab3, ag3), ab3);
			result.direction = vec2(direction.x, direction.y);
		}
		else
		{
			result.simplex = CreateSimplex(simplex.C, simplex.B, simplex.A);
			result.direction = vec2(0, 0);
			result.containsGoal = true;
		}
	}

	return result;
}

inline DoSimplexResult_2D DoSimplex(Simplex_2D simplex)
{
	DoSimplexResult_2D result;

	switch (simplex.type)
	{
	case Simplex_2D::Simplex_Line_2D:
	{
								  result = DoSimplexLine(simplex);
								  break;
	}

	case Simplex_2D::Simplex_Triangle_2D:
	{
									  result = DoSimplexTriangle(simplex);
									  break;
	}

	default:
	{
			   Assert(true);
			   break;
	}
	}

	return result;
}







struct GJKInfo_2D
{
	bool collided;
	U32 numberOfLoopsCompleted;
	Simplex_2D simplex;
};

template<typename S1, typename S2>
inline GJKInfo_2D GJK_2D(S1 shapeA, S2 shapeB)
{
	bool collisionDetected = false;

	vec2 s = Support(shapeA, shapeB, vec2(1.0f, -1.0f));
	if (s == vec2(0, 0))
	{
		GJKInfo_2D result;
		result.collided = true;
		result.numberOfLoopsCompleted = 0;
		result.simplex = CreateSimplex(s);
		return result;
	}
	Simplex_2D simplex = CreateSimplex(s);
	vec2 d = -s;

	U32 loopCounter = 0;
	for (; loopCounter < MAXGJKITERATIONS_2D;)
	{
		++loopCounter;
		vec2 A = Support(shapeA, shapeB, d);
		if (A == vec2(0, 0))
		{
			collisionDetected = true;
			break;
		}
		if (dot(A, d) < 0)
		{
			collisionDetected = false;
			break;
		}
		AddSimplexPoint(&simplex, A);
		DoSimplexResult_2D dsr = DoSimplex(simplex, d);
		simplex = dsr.simplex;
		d = dsr.d;
		if (dsr.containsGoal)
		{
			collisionDetected = true;
			break;
		}
	}

	if (loopCounter == maxNumberOfLoops)
	{
		collisionDetected = true;
		DebugPrint("GJK_2D: collision exceeded max number of iterations.\n");
	}

	if (collisionDetected)
	{
		switch (simplex.type)
		{
		case Simplex_2D::Simplex_Point_2D:
		{
									   Assert(true);
		}
		break;

		case Simplex_2D::Simplex_Line_2D:
		{
									  RandomNumberGenerator randomNumberGenerator;
									  SeedRandomNumberGenerator(&randomNumberGenerator, 1);

									  vec2 newPoint = simplex.A;
									  while (newPoint == simplex.A || newPoint == simplex.B)
									  {
										  F32 randomX = RandomF32Between(&randomNumberGenerator, -1, 1);
										  F32 randomY = RandomF32Between(&randomNumberGenerator, -1, 1);
										  vec2 randomDirection = vec2(randomX, randomY);
										  newPoint = Support(shapeA, shapeB, randomDirection);
									  }
									  AddSimplexPoint(&simplex, newPoint);
		}
		break;

		case Simplex_2D::Simplex_Triangle_2D:
		default:
		break;
		}
	}

	GJKInfo result;
	result.collided = collisionDetected;
	result.numberOfLoopsCompleted = loopCounter;
	result.simplex = simplex;

	return result;
}






// EPA
struct EPAInfo_2D
{
	vec2 normal;
	F32 distance;
};

struct EPAPoint_2D
{
	vec2 point;
	EPAPoint_2D* next;
};

struct EPAPointList_2D 
{
	EPAPoint_2D* first;
	U32 count;
};

EPAPointList_2D CreateEPAPointList()
{
	EPAPointList_2D result;

	result.first = NULL;
	result.count = 0;

	return result;
}

void AddPoint(EPAPointList_2D* list, vec2 point)
{
	EPAPoint_2D* currentPoint = list->first;
	while (currentPoint->next != NULL)
	{
		currentPoint = currentPoint->next;
	}

	currentPoint->next = (EPAPoint_2D*)malloc(sizeof(EPAPoint_2D));
	currentPoint->next->point = point;
	currentPoint->next->next = NULL;

	list->count += 1;
}

void InsertAfter(EPAPointList_2D* list, EPAPoint_2D* insertionPoint, vec2 point)
{
	EPAPoint_2D* next = insertionPoint->next;

	EPAPoint_2D* newPoint = (EPAPoint_2D*)malloc(sizeof(EPAPoint_2D));
	newPoint->point = point;
	newPoint->next = NULL;

	insertionPoint->next = newPoint;
	newPoint->next = next;

	list->count += 1;
}

void DestroyEPAPointList(EPAPointList_2D* list)
{
	EPAPoint_2D* prev = NULL;
	EPAPoint_2D* curr = list->first;

	while (curr != NULL)
	{
		prev = curr;
		curr = curr->next;

		free(prev);
		list->count -= 1;
	}

}

template<typename S1, typename S2>
inline EPAInfo EPA(S1 shapeA, S2 shapeB, Simplex simplex)
{
	EPAInfo result;

	EPAPointList_2D pointList = CreateEPAPointList();
	AddPoint(&pointList, simplex.A);
	AddPoint(&pointList, simplex.B);
	AddPoint(&pointList, simplex.C);
	
	vec3 normal_surface = cross(simplex.B - simplex.A, simplex.C - simplex.A);

	for (U32 i = 0; i < MAXEPAITERATIONS_2D; ++i)
	{
		EPAPoint_2D* a = pointList.first;
		EPAPoint_2D* b = a->next;

		EPAPoint_2D* closestEdge_point1 = NULL;
		F32 closestEdge_distance = UNREASONABLEFLOATNUMBER;
		vec2 closestEdge_normal;
		while (b != NULL)
		{
			vec2 ab = b->point - a->point;
			vec2 normal_ab = cross(ab, normal_surface);
			vec2 ab_distance = dot(normal_ab, a);

			if (ab_distance < closestEdge_distance)
			{
				closestEdge_point1 = a;
				closestEdge_distance = ab_distance;
				closestEdge_normal = normal_ab;
			}
		}

		vec2 newPolytopePoint = Support(shapeA, shapeB, closestEdge_normal);
		F32 newPolytopePoint_distance = dot(newPolytopePoint, closestEdge_normal);
		if (newPolytopePoint_distance - closestEdge_distance < EPATOLERANCE)
		{
			result.normal = closestEdge_normal;
			result.distance = newPolytopePoint_distance;

			break;
		}
		else
		{
			InsertAfter(&pointList, a, newPolytopePoint);
		}
	}

	DestroyEPAPointList(&pointList);

	return result;
}






struct CollisionInfo_2D
{
	bool collided;
	vec2 normal;
	F32 distance;
};

template<typename S1, typename S2>
inline CollisionInfo_2D DetectCollision_2D(S1 shapeA, S2 shapeB)
{
	CollisionInfo_2D result;
	result.collided = false;

	GJKInfo_2D gjkInfo = GJK(shapeA, shapeB);
	if (gjkInfo.collided)
	{
		// 		EPAInfo epaInfo = EPA(shapeA, shapeB, gjkInfo.simplex);
		// 
		result.collided = true;
		// 		result.normal = epaInfo.normal;
		// 		result.distance = epaInfo.distance;
	}

	return result;
}