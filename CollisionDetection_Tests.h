#pragma once

#include "CollisionDetection.h"



// r rotated 0 degrees
void CollisionTestsRectRect1()
{
	Rectangle g;
	g.origin = vec2(0, 0);
	g.halfDim = vec2(.5, .5);
	g.transform = mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, .5f, .5f, 0.0f, 1.0f);


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
			r.transform = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, xs[x], ys[y], 0, 1);

			bool collision = DetectCollision(g, r).collided;
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
	g.transform = mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, .5f, .5f, 0.0f, 1.0f);

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
			r.transform = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, xs[x], ys[y], 0, 1) * mat4(cos(angle), sin(angle), 0, 0, -sin(angle), cos(angle), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

			bool collisionSimple = DetectCollision(g, r).collided;
			bool collisionCasey = DetectCollision_Casey(g, r).collided;
			bool collisionAllVoronoi = DetectCollision_AllVoronoi(g, r).collided;

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