#pragma once

#include "Types.h"
#include "Math.h"

#include "_SystemAPI.h"

#include "Transform.h"

#include "RandomNumberGenerator.h"
#include <vector> 


// General Constants
#define UNREASONABLEFLOATNUMBER 100000.0f

// GJK Constants
#define MAXGJKITERATIONS_2D 128

// EPA Constants
#define MAXEPAITERATIONS_2D 128
#define EPATOLERANCE .0001

typedef std::vector<vec2> PointCloud;

struct Shape_2D
{
	virtual vec2 Support(Transform transform, vec2 direction) = 0;
	virtual vec2 GetOffset() = 0;
	virtual void SetOffset(vec2 offset) = 0;
	virtual bool IsPhantom() = 0;
	virtual void SetPhantomState(bool phantomState) = 0;
	virtual void TogglePhantomState() = 0;
	//virtual PointCloud Points(Transform transform) = 0;
};


struct Rectangle_2D : public Shape_2D
{
	vec2 halfDim;
	vec2 offset;
	bool isPhantom;

	Rectangle_2D()
	{
		this->halfDim = vec2(1.0f, 1.0f);
		this->offset = vec2(0.0f, 0.0f);
		this->isPhantom = false;
	}

	Rectangle_2D(vec2 halfDim, vec2 offset = vec2(0.0f, 0.0f), bool isPhantom = false)
	{
		this->halfDim = halfDim;
		this->offset = offset;
		this->isPhantom = isPhantom;
	}

	virtual vec2 Support(Transform transform, vec2 direction)
	{
		transform.position += this->offset;
		vec2 direction_ALocal = vec2(transform.WorldToLocalTransform() * vec3(direction.x, direction.y, 0.0f));

		vec2 AiPoints[4] = {
			vec2(-this->halfDim.x, -this->halfDim.y),
			vec2(-this->halfDim.x,  this->halfDim.y),
			vec2( this->halfDim.x, -this->halfDim.y),
			vec2( this->halfDim.x,  this->halfDim.y)
		};
		F32 AiDots[4] = {
			dot(direction_ALocal, vec2(AiPoints[0].x, AiPoints[0].y)),
			dot(direction_ALocal, vec2(AiPoints[1].x, AiPoints[1].y)),
			dot(direction_ALocal, vec2(AiPoints[2].x, AiPoints[2].y)),
			dot(direction_ALocal, vec2(AiPoints[3].x, AiPoints[3].y))
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
		vec3 maxA3_World = transform.LocalToWorldTransform() * vec3(maxA.x, maxA.y, 1.0f);
		vec2 maxA_World = vec2(maxA3_World.x, maxA3_World.y);

		return  maxA_World;
	}

	virtual vec2 GetOffset()
	{
		vec2 result;
		result = this->offset;
		return result;
	}

	virtual void SetOffset(vec2 offset)
	{
		this->offset = offset;
	}

	virtual bool IsPhantom()
	{
		bool result;
		
		result = this->isPhantom;

		return result;
	}

	virtual void SetPhantomState(bool phantomState)
	{
		this->isPhantom = phantomState;
	}

	virtual void TogglePhantomState()
	{
		this->isPhantom = !this->isPhantom;
	}

	/*virtual PointCloud Points(Transform transform)
	{
		PointCloud result;

		mat3 A = transform.LocalToWorldTransform();
		result.push_back(vec2(A * vec3(-this->halfDim.x,  this->halfDim.y, 1.0f)));
		result.push_back(vec2(A * vec3(-this->halfDim.x, -this->halfDim.y, 1.0f)));
		result.push_back(vec2(A * vec3( this->halfDim.x, -this->halfDim.y, 1.0f)));
		result.push_back(vec2(A * vec3( this->halfDim.x,  this->halfDim.y, 1.0f)));

		return result;
	}*/
};



struct Circle_2D : public Shape_2D
{
	F32  radius;
	vec2 offset;
	bool isPhantom;

	Circle_2D()
	{
		this->radius = 1.0f;
		this->offset = vec2(0.0f, 0.0f);
		this->isPhantom = false;
	}

	Circle_2D(F32 radius, vec2 offset = vec2(0.0f, 0.0f), bool isPhantom = false)
	{
		this->radius = radius;
		this->offset = offset;
		this->isPhantom = isPhantom;
	}

	virtual vec2 Support(Transform transform, vec2 direction)
	{
		transform.position += this->offset;
		vec2 direction_ALocal = normalize(vec2(transform.WorldToLocalTransform() * vec3(direction.x, direction.y, 0)));

		vec2 maxA = this->radius * direction_ALocal;
		vec2 maxA_World = vec2(transform.LocalToWorldTransform() * vec3(maxA.x, maxA.y, 1));

		return maxA_World;
	}

	virtual vec2 GetOffset()
	{
		vec2 result;
		result = this->offset;
		return result;
	}

	virtual void SetOffset(vec2 offset)
	{
		this->offset = offset;
	}

	virtual bool IsPhantom()
	{
		bool result;

		result = this->isPhantom;

		return result;
	}

	virtual void SetPhantomState(bool phantomState)
	{
		this->isPhantom = phantomState;
	}

	virtual void TogglePhantomState()
	{
		this->isPhantom = !this->isPhantom;
	}
};



struct Triangle_2D : public Shape_2D
{
	vec2 points[3];
	vec2 offset;
	bool isPhantom;

	Triangle_2D()
	{
		this->points[0] = vec2(-0.6f, -0.3f);
		this->points[1] = vec2(0.6f, -0.3f);
		this->points[2] = vec2(0.0f, 0.6f);
		this->offset = vec2(0.0f, 0.0f);
		this->isPhantom = false;
	}

	/* NOTE: Must be wound counter clockwise. */
	Triangle_2D(vec2 A, vec2 B, vec2 C, vec2 offset = vec2(0.0f, 0.0f), bool isPhantom = false)
	{
		this->points[0] = A;
		this->points[1] = B;
		this->points[2] = C;
		this->offset = offset;
		this->isPhantom = isPhantom;
	}

	virtual vec2 Support(Transform transform, vec2 direction)
	{
		transform.position += this->offset;
		vec2 direction_ALocal = vec2(transform.WorldToLocalTransform() * vec3(direction.x, direction.y, 0));

		// NOTE: AiPoints is unnecessary here, just for uniformity.
		vec2 AiPoints[3] = {
			vec2(this->points[0].x, this->points[0].y),
			vec2(this->points[1].x, this->points[1].y),
			vec2(this->points[2].x, this->points[2].y)
		};
		F32 AiDots[3] = {
			dot(direction_ALocal, AiPoints[0]),
			dot(direction_ALocal, AiPoints[1]),
			dot(direction_ALocal, AiPoints[2])
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
		vec2 maxA = AiPoints[maxPositionAi];
		vec2 maxA_World = vec2(transform.LocalToWorldTransform() * vec3(maxA.x, maxA.y, 1));

		return maxA_World;
	}

	virtual vec2 GetOffset()
	{
		vec2 result;
		result = this->offset;
		return result;
	}

	virtual void SetOffset(vec2 offset)
	{
		this->offset = offset;
	}

	virtual bool IsPhantom()
	{
		bool result;

		result = this->isPhantom;

		return result;
	}

	virtual void SetPhantomState(bool phantomState)
	{
		this->isPhantom = phantomState;
	}

	virtual void TogglePhantomState()
	{
		this->isPhantom = !this->isPhantom;
	}

	/*virtual PointCloud Points(Transform transform)
	{
		PointCloud result;

		mat3 A = transform.LocalToWorldTransform();
		result.push_back(vec2(A * vec3(this->points[0].x, this->points[0].y, 1.0f)));
		result.push_back(vec2(A * vec3(this->points[1].x, this->points[1].y, 1.0f)));
		result.push_back(vec2(A * vec3(this->points[2].x, this->points[2].y, 1.0f)));

		return result;
	}*/
};



inline vec2 Support(Shape_2D* A, Transform A_Transform, Shape_2D* B, Transform B_Transform, vec2 direction)
{
	// find the point in A that has the largest value with dot(Ai, Direction)
	vec2 maxA = A->Support(A_Transform, direction);

	// find the point in B that has the largest value with dot(Bj, Direction)
	vec2 maxB = B->Support(B_Transform, -direction);

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

	Simplex_2D()
	{
		this->type = Simplex_Empty_2D;
	}

	Simplex_2D(vec2 A)
	{
		this->type = Simplex_Point_2D;
		this->A = A;
	}

	Simplex_2D(vec2 B, vec2 A)
	{
		this->type = Simplex_Line_2D;
		this->B = B;
		this->A = A;
	}

	Simplex_2D(vec2 C, vec2 B, vec2 A)
	{
		this->type = Simplex_Triangle_2D;
		this->C = C;
		this->B = B;
		this->A = A;
	}

	void AddSimplexPoint(vec2 point)
	{
		Simplex_2D::SimplexType_2D simplexType = this->type;
		switch (simplexType)
		{
		case Simplex_2D::Simplex_Empty_2D:
		this->A = point;
		break;

		case Simplex_2D::Simplex_Point_2D:
		this->B = this->A;
		this->A = point;
		break;

		case Simplex_2D::Simplex_Line_2D:
		this->C = this->B;
		this->B = this->A;
		this->A = point;
		break;

		case Simplex_2D::Simplex_Triangle_2D:
		default:
		Assert(true);
		break;
		}
		this->type = (Simplex_2D::SimplexType_2D)(simplexType + 1);
	}
};



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
	result.simplex = Simplex_2D(simplex.B, simplex.A);

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
		result.simplex = Simplex_2D(simplex.C, simplex.A);

		vec3 direction = cross(cross(ac3, ag3), ac3);
		result.direction = vec2(direction.x, direction.y);
	}
	else
	{
		if (dot(cross(ab3, abc), ag3) > 0)
		{
			// Case AB
			result.simplex = Simplex_2D(simplex.B, simplex.A);

			vec3 direction = cross(cross(ab3, ag3), ab3);
			result.direction = vec2(direction.x, direction.y);
		}
		else
		{
			result.simplex = Simplex_2D(simplex.C, simplex.B, simplex.A);
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

inline GJKInfo_2D GJK_2D(Shape_2D* shapeA, Transform shapeATransform, Shape_2D* shapeB, Transform shapeBTransform, const U64 MAXITERATIONS = MAXGJKITERATIONS_2D)
{
	bool collisionDetected = false;

	vec2 s = Support(shapeA, shapeATransform, shapeB, shapeBTransform, vec2(1.0f, -1.0f));
	if (s == vec2(0, 0))
	{
		collisionDetected = true;

		//GJKInfo_2D result;
		//result.collided = true;
		//result.numberOfLoopsCompleted = 0;
		//result.simplex = Simplex_2D(s);
		//return result;
	}
	Simplex_2D simplex = Simplex_2D(s);
	vec2 d = -s;

	
	U32 loopCounter = 0;
	if (!collisionDetected)
	{
		for (; loopCounter < MAXITERATIONS;)
		{
			++loopCounter;
			vec2 A = Support(shapeA, shapeATransform, shapeB, shapeBTransform, d);
			if (A == vec2(0, 0))
			{
				collisionDetected = true;
				simplex.AddSimplexPoint(A);
				break;
			}
			if (dot(A, d) < 0)
			{
				collisionDetected = false;
				break;
			}
			simplex.AddSimplexPoint(A);
			DoSimplexResult_2D dsr = DoSimplex(simplex);
			simplex = dsr.simplex;
			d = dsr.direction;
			if (dsr.containsGoal)
			{
				collisionDetected = true;
				break;
			}
		}

		if (loopCounter == MAXITERATIONS)
		{
			collisionDetected = true;
			DebugPrint("GJK_2D: collision exceeded max number of iterations.\n");
		}
	}

	if (collisionDetected)
	{
		switch (simplex.type)
		{
		case Simplex_2D::Simplex_Point_2D:
		{
											 //Assert(false);
											 RandomNumberGenerator randomNumberGenerator;
											 Seed(&randomNumberGenerator, 1);

											 vec2 newPoint = simplex.A;
											 while (newPoint == simplex.A)
											 {
												 F32 randomX = RandomF32Between(&randomNumberGenerator, -1, 1);
												 F32 randomY = RandomF32Between(&randomNumberGenerator, -1, 1);
												 vec2 randomDirection = vec2(randomX, randomY);
												 newPoint = Support(shapeA, shapeATransform, shapeB, shapeBTransform, randomDirection);
											 }
											 simplex.AddSimplexPoint(newPoint);

											 newPoint = simplex.A;
											 while (newPoint == simplex.A || newPoint == simplex.B)
											 {
												 F32 randomX = RandomF32Between(&randomNumberGenerator, -1, 1);
												 F32 randomY = RandomF32Between(&randomNumberGenerator, -1, 1);
												 vec2 randomDirection = vec2(randomX, randomY);
												 newPoint = Support(shapeA, shapeATransform, shapeB, shapeBTransform, randomDirection);
											 }
											 simplex.AddSimplexPoint(newPoint);
		}
		break;

		case Simplex_2D::Simplex_Line_2D:
		{
											RandomNumberGenerator randomNumberGenerator;
											Seed(&randomNumberGenerator, 1);

											vec2 newPoint = simplex.A;
											while (newPoint == simplex.A || newPoint == simplex.B)
											{
												F32 randomX = RandomF32Between(&randomNumberGenerator, -1, 1);
												F32 randomY = RandomF32Between(&randomNumberGenerator, -1, 1);
												vec2 randomDirection = vec2(randomX, randomY);
												newPoint = Support(shapeA, shapeATransform, shapeB, shapeBTransform, randomDirection);
											}
											simplex.AddSimplexPoint(newPoint);
		}
		break;

		case Simplex_2D::Simplex_Triangle_2D:
		default:
		break;
		}
	}

	GJKInfo_2D result;
	result.collided = collisionDetected;
	result.numberOfLoopsCompleted = loopCounter;
	result.simplex = simplex;

	return result;
}



struct PointList_2D
{
	struct Point
	{
		vec2 point;
		Point* next;
	};
	Point* first;
	U32 count;

	PointList_2D()
	{
		this->first = NULL;
		this->count = 0;
	}

	void AddPoint(vec2 point)
	{
		Point* currentPoint = this->first;
		if (currentPoint == NULL)
		{
			currentPoint = (Point*)malloc(sizeof(Point));
			currentPoint->point = point;
			currentPoint->next = NULL;
			this->first = currentPoint;
		}
		else
		{
			while (currentPoint->next != NULL)
			{
				currentPoint = currentPoint->next;
			}

			currentPoint->next = (Point*)malloc(sizeof(Point));
			currentPoint->next->point = point;
			currentPoint->next->next = NULL;
		}

		this->count += 1;
	}

	void InsertAfter(Point* insertionPoint, vec2 point)
	{
		Point* next = insertionPoint->next;

		Point* newPoint = (Point*)malloc(sizeof(Point));
		newPoint->point = point;
		newPoint->next = NULL;

		insertionPoint->next = newPoint;
		newPoint->next = next;

		this->count += 1;
	}

	void Destroy()
	{
		Point* prev = NULL;
		Point* curr = this->first;

		while (curr != NULL)
		{
			prev = curr;
			curr = curr->next;

			free(prev);
			this->count -= 1;
		}

	}
};





// EPA
struct EPAInfo_2D
{
	vec2 normal;
	F32 distance;
	PointCloud points;
};

inline EPAInfo_2D EPA_2D(Shape_2D* shapeA, Transform shapeATransform, Shape_2D* shapeB, Transform shapeBTransform, Simplex_2D simplex, const U64 MAXITERATIONS = MAXGJKITERATIONS_2D)
{
	Assert(simplex.type == Simplex_2D::Simplex_Triangle_2D);

	EPAInfo_2D result;

	// Make sure the winding is correct
	vec2 simplexAB = simplex.B - simplex.A;
	vec2 simplexAC = simplex.C - simplex.A;
	vec3 normal_surface = cross(vec3(simplexAB.x, simplexAB.y, 0), vec3(simplexAC.x, simplexAC.y, 0));
	if (dot(normal_surface, vec3(1, 0, 0)) > 0)
	{
		vec2 t = simplex.A;
		simplex.A = simplex.C;
		simplex.C = t;
		simplexAB = simplex.B - simplex.A;
		simplexAC = simplex.C - simplex.A;
		normal_surface = cross(vec3(simplexAB.x, simplexAB.y, 0), vec3(simplexAC.x, simplexAC.y, 0));
	}

	PointList_2D pointList = PointList_2D();
	pointList.AddPoint(simplex.A);
	pointList.AddPoint(simplex.B);
	pointList.AddPoint(simplex.C);

	for (U32 i = 0; i < MAXEPAITERATIONS_2D; ++i)
	{
		PointList_2D::Point* a = pointList.first;
		PointList_2D::Point* b = a->next;

		PointList_2D::Point* closestEdge_point1 = NULL;
		F32 closestEdge_distance = UNREASONABLEFLOATNUMBER;
		vec2 closestEdge_normal;
		while (b != NULL)
		{
			vec2 ab = b->point - a->point;
			vec3 ab3 = vec3(ab.x, ab.y, 0);
			vec3 normal3_ab = cross(ab3, normal_surface);
			vec2 normal_ab = normalize(vec2(normal3_ab.x, normal3_ab.y));
			F32 ab_distance = dot(normal_ab, a->point);

			if (ab_distance < closestEdge_distance)
			{
				closestEdge_point1 = a;
				closestEdge_distance = ab_distance;
				closestEdge_normal = normal_ab;
			}

			a = a->next;
			b = b->next;
		}
		{	// NOTE: Do the wrap around edge
			// a = a;
			b = pointList.first;

			vec2 ab = b->point - a->point;
			vec3 ab3 = vec3(ab.x, ab.y, 0);
			vec3 normal3_ab = cross(ab3, normal_surface);
			vec2 normal_ab = normalize(vec2(normal3_ab.x, normal3_ab.y));
			F32 ab_distance = dot(normal_ab, a->point);

			if (ab_distance < closestEdge_distance)
			{
				closestEdge_point1 = a;
				closestEdge_distance = ab_distance;
				closestEdge_normal = normal_ab;
			}
		}

		vec2 newPolytopePoint = Support(shapeA, shapeATransform, shapeB, shapeBTransform, closestEdge_normal);
		F32 newPolytopePoint_distance = dot(newPolytopePoint, closestEdge_normal);
		if (newPolytopePoint_distance - closestEdge_distance < EPATOLERANCE)
		{
			result.normal = closestEdge_normal;
			result.distance = newPolytopePoint_distance;

			break;
		}
		else
		{
			pointList.InsertAfter(closestEdge_point1, newPolytopePoint);
		}
	}

	PointList_2D::Point* p = pointList.first;
	while (p != NULL)
	{
		result.points.push_back(p->point);
		p = p->next;
	}
	pointList.Destroy();

	return result;
}






struct CollisionInfo_2D
{
	bool collided;
	vec2 normal;
	F32 distance;
};

inline CollisionInfo_2D DetectCollision_2D(Shape_2D* shapeA, Transform shapeATransform, Shape_2D* shapeB, Transform shapeBTransform)
{
	CollisionInfo_2D result;
	result.collided = false;

	GJKInfo_2D gjkInfo = GJK_2D(shapeA, shapeATransform, shapeB, shapeBTransform);
	if (gjkInfo.collided)
	{
		EPAInfo_2D epaInfo = EPA_2D(shapeA, shapeATransform, shapeB, shapeBTransform, gjkInfo.simplex);

		result.collided = true;
		result.normal = epaInfo.normal;
		result.distance = epaInfo.distance;
	}

	return result;
}