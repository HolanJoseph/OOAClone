#pragma once

#include "Types.h"
#include "Math.h"
#include "CollisionDetection.h"

struct ConvexHullPoints
{
	vec3 points[64];
	U32 count;
};

struct OrientedBoundingBoxPoints
{
	vec4 points[8];
};

OrientedBoundingBoxPoints operator*(const mat4 &A, const OrientedBoundingBoxPoints &x)
{
	OrientedBoundingBoxPoints result;
	for (U32 i = 0; i < 8; ++i)
	{
		result.points[i] = A * x.points[i];
	}
	return result;
}

OrientedBoundingBoxPoints Homogenize(OrientedBoundingBoxPoints points)
{
	OrientedBoundingBoxPoints result;

	for (U32 i = 0; i < 8; ++i)
	{
		result.points[i] = points.points[i] / points.points[i].w;
	}

	return result;
}

OrientedBoundingBoxPoints GetOBBPoints(OrientedBoundingBox box)
{
	OrientedBoundingBoxPoints result;

	result.points[0] = box.transform * vec4(box.origin.x - box.halfDim.x, box.origin.y - box.halfDim.y, box.origin.z + box.halfDim.z, 1.0f);
	result.points[1] = box.transform * vec4(box.origin.x - box.halfDim.x, box.origin.y + box.halfDim.y, box.origin.z + box.halfDim.z, 1.0f);
	result.points[2] = box.transform * vec4(box.origin.x + box.halfDim.x, box.origin.y - box.halfDim.y, box.origin.z + box.halfDim.z, 1.0f);
	result.points[3] = box.transform * vec4(box.origin.x + box.halfDim.x, box.origin.y + box.halfDim.y, box.origin.z + box.halfDim.z, 1.0f);
	result.points[4] = box.transform * vec4(box.origin.x - box.halfDim.x, box.origin.y - box.halfDim.y, box.origin.z - box.halfDim.z, 1.0f);
	result.points[5] = box.transform * vec4(box.origin.x - box.halfDim.x, box.origin.y + box.halfDim.y, box.origin.z - box.halfDim.z, 1.0f);
	result.points[6] = box.transform * vec4(box.origin.x + box.halfDim.x, box.origin.y - box.halfDim.y, box.origin.z - box.halfDim.z, 1.0f);
	result.points[7] = box.transform * vec4(box.origin.x + box.halfDim.x, box.origin.y + box.halfDim.y, box.origin.z - box.halfDim.z, 1.0f);

	return result;
}

inline ConvexHullPoints GetConvexHullPoints(OrientedBoundingBox A, OrientedBoundingBox B)
{
	ConvexHullPoints result;
	result.count = 0;

	OrientedBoundingBoxPoints Apoints = Homogenize(GetOBBPoints(A));
	OrientedBoundingBoxPoints Bpoints = Homogenize(GetOBBPoints(B));

	ConvexHullPoints cvhPoints;
	U32 count = 0;
	for (U32 i = 0; i < 8; ++i)
	{
		vec3 Ai = vec3(Apoints.points[i].x, Apoints.points[i].y, Apoints.points[i].z);

		for (U32 j = 0; j < 8; ++j)
		{
			vec3 Bj = vec3(Bpoints.points[j].x, Bpoints.points[j].y, Bpoints.points[j].z);

			cvhPoints.points[count++] = Ai - Bj;
		}
	}

	for (U32 i = 0; i < 64; ++i)
	{
		vec3 cvhPoint = cvhPoints.points[i];
		bool unique = true;

		for (U32 j = 0; j < result.count; ++j)
		{
			if (cvhPoint == result.points[j])
			{
				unique = false;
			}
		}

		if (unique)
		{
			result.points[result.count++] = cvhPoint;
		}
	}

	return result;
}