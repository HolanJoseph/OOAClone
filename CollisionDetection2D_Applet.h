#pragma once

/*
 * Controls
 * 
 * Right Handed Coordinates
 * 
 * WASD - move the controlled shape in the xy plane
 * QE   - rotate the controlled shape around the z axis
 * RF   - scale the controlled shape
 * 1	- switch controlled shape
 * 2	- switch the controlled shapes shape (Rectangle -> Circle -> Triangle)
 * 
 */

#include "Types.h"
#include "Math.h"

#include "DebugAPI.h"
#include "InputAPI.h"

#include "CollisionDetection2D.h"

#include "glew/GL/glew.h"
#include <gl/GL.h>

#include <vector>

// NOTE: NEXT IS GRAPHICS, HOLY CRAP
extern GLuint solidColorQuadShaderProgram;
extern GLuint solidColorQuadPCMLocation;
extern GLuint solidColorQuadQuadColorLocation;

extern GLuint solidColorCircleInPointShaderProgram;
extern GLuint solidColorCircleInPointPCMLocation;
extern GLuint solidColorCircleInPointCircleColorLocation;

extern GLuint gridVAO;
	   GLuint lineVAO;
	   GLuint lineVertexBuffer;
	   GLuint pointVAO;
	   GLuint pointVertexBuffer;
extern GLuint texturedQuadVAO;
extern GLuint circleVAO;
extern GLuint equalateralTriangleVAO;

extern const GLuint numVertices; // Number of vertices in the textured quad
#define numVerticesTriangle 6
#define unitCircleSize_px 160.0f
#define numGridLines 10
#define pointsPerLine 2
#define gridLinePointDimensionality 2

struct Camera_CD2D
{
	Rectangle_2D rectangle;

	vec2 position;
	F32  rotationAngle;
	F32  scale;

	Camera_CD2D()
	{
		position = vec2(0, 0);
		rotationAngle = 0;
		scale = 1;
	}

	void ResizeViewArea(vec2 halfDim)
	{
		rectangle.halfDim = halfDim;
	}

	mat3 GetProjectionMatrix()
	{
		mat3 result = ScaleMatrix(1.0f / rectangle.halfDim);
		return result;
	}
};

struct Collidable_CD2D
{
	enum Mode
	{
		Mode_Uninitialized,

		Mode_Rectangle,
		Mode_Triangle,
		Mode_Circle,

		Mode_Count
	};
	Mode mode;
	Rectangle_2D rectangle;
	Circle_2D    circle;
	Triangle_2D  triangle;

	Transform transform;

	Collidable_CD2D()
	{
		mode = Collidable_CD2D::Mode_Rectangle;
	}
};

vec4 backgroundColor;
vec4 gridColor;
vec4 xAxisColor;
vec4 yAxisColor;
vec4 minkowskiPointColor;
vec4 gjkPointColor;
vec4 seperatorColor;

Camera_CD2D     camera_CD2D;
Collidable_CD2D shape1_CD2D;
Collidable_CD2D shape2_CD2D;
vec4			color_CD2D;
bool			controllingShape1_CD2D;

#define NUMBER_OF_TRIANGLE_POINTS 3
#define NUMBER_OF_RECTANGLE_POINTS 4
#define NUMBER_OF_CIRCLE_POINTS 20

#define minkowskiPointSize_px 9
#define gjkPointSize_px 5

inline void InitializeLineVAO()
{
	glGenVertexArrays(1, &lineVAO);
	glBindVertexArray(lineVAO);
	//GLuint lineVertexBuffer;
	glGenBuffers(1, &lineVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, lineVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(F32)* 4, NULL, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}

inline void InitializePointVAO()
{
	glGenVertexArrays(1, &pointVAO);
	glBindVertexArray(pointVAO);
	//GLuint pointVertexBuffer;
	glGenBuffers(1, &pointVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, pointVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(F32) * 2, NULL, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}

inline void InitializeCollisionDetection2DApplet()
{
	gridColor = vec4(0.29f, 0.29f, 0.29f, 1.0f);
	backgroundColor = vec4(0.22f, 0.22f, 0.22f, 1.0f);
	xAxisColor = vec4(1, 0, 0, 1);
	yAxisColor = vec4(0, 1, 0, 1);
	minkowskiPointColor = vec4(1, 1, 0, 1);
	gjkPointColor = vec4(0, 0, 1, 1);
	seperatorColor = vec4(.663, .733, .384, 1);

	shape1_CD2D.transform.position = vec2(-0.650137424, 0.198641479);
	shape1_CD2D.transform.rotationAngle = 28.6177559f;
	shape1_CD2D.transform.scale = 1.77565324f;
	shape2_CD2D.transform.position = vec2(-0.866670012, 0.266644627);
	shape2_CD2D.transform.rotationAngle = 0;
	shape2_CD2D.transform.scale = 1.67522526f;

	camera_CD2D.ResizeViewArea(vec2(5,5));

	color_CD2D = vec4(.933, .933, .933, 1);

	controllingShape1_CD2D = true;


	InitializeLineVAO();
	InitializePointVAO();
	glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
}



inline void DrawGrid(vec4 color, Camera_CD2D* camera)
{
	mat3 P_projection = camera->GetProjectionMatrix();
	mat3 C_camera = TranslationMatrix(camera->position);
	mat3 M_model = mat3(1,0,0,0,1,0,0,0,1);
	mat3 PCM = P_projection * inverse(C_camera) * M_model;

	glUseProgram(solidColorQuadShaderProgram);
	glBindVertexArray(gridVAO);
	glUniformMatrix3fv(solidColorQuadPCMLocation, 1, GL_FALSE, &PCM[0][0]);
	glUniform4fv(solidColorQuadQuadColorLocation, 1, &color[0]);
	glDrawArrays(GL_LINES, 0, (numGridLines + numGridLines + 2) * pointsPerLine * gridLinePointDimensionality);
}

inline void DrawPoint(vec2 p, U32 pointSize, vec4 color, Camera_CD2D* camera)
{
	mat3 P_projection = camera->GetProjectionMatrix();
	mat3 C_camera = TranslationMatrix(camera->position);
	mat3 M_model = mat3(1, 0, 0, 0, 1, 0, 0, 0, 1);
	mat3 PCM = P_projection * inverse(C_camera) * M_model;

	glUseProgram(solidColorQuadShaderProgram);
	glBindVertexArray(pointVAO);

	glBindBuffer(GL_ARRAY_BUFFER, pointVertexBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(F32) * 2, &p[0]);

	glUniformMatrix3fv(solidColorQuadPCMLocation, 1, GL_FALSE, &PCM[0][0]);
	glUniform4fv(solidColorQuadQuadColorLocation, 1, &color[0]);
	glPointSize(pointSize);
	glDrawArrays(GL_POINTS, 0, 1);
}

inline void DrawLine(vec2 a, vec2 b, vec4 color, Camera_CD2D* camera)
{
	mat3 P_projection = camera->GetProjectionMatrix();
	mat3 C_camera = TranslationMatrix(camera->position);
	mat3 M_model = mat3(1, 0, 0, 0, 1, 0, 0, 0, 1);
	mat3 PCM = P_projection * inverse(C_camera) * M_model;

	glUseProgram(solidColorQuadShaderProgram);
	glBindVertexArray(lineVAO);

	vec4 points = vec4(a.x, a.y, b.x, b.y);
	glBindBuffer(GL_ARRAY_BUFFER, lineVertexBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(F32)* 4, &points[0]);

	glUniformMatrix3fv(solidColorQuadPCMLocation, 1, GL_FALSE, &PCM[0][0]);
	glUniform4fv(solidColorQuadQuadColorLocation, 1, &color[0]);
	glDrawArrays(GL_LINES, 0, 4);
}

inline void DrawRectangle(Rectangle_2D* rectangle, mat3* PCM, vec4 color)
{
	glUseProgram(solidColorQuadShaderProgram);
	glBindVertexArray(texturedQuadVAO);
	glUniformMatrix3fv(solidColorQuadPCMLocation, 1, GL_FALSE, &(*PCM)[0][0]);
	glUniform4fv(solidColorQuadQuadColorLocation, 1, &color[0]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices);
}

inline void DrawCircle(Circle_2D* circle, F32 scale, mat3* PCM, vec4 color)
{
	glUseProgram(solidColorCircleInPointShaderProgram);
	glBindVertexArray(circleVAO);
	glUniformMatrix3fv(solidColorCircleInPointPCMLocation, 1, GL_FALSE, &(*PCM)[0][0]);
	glUniform4fv(solidColorCircleInPointCircleColorLocation, 1, &color[0]);
	glPointSize(scale * circle->radius * unitCircleSize_px);
	glDrawArrays(GL_POINTS, 0, 1);
}

inline void DrawTriangle(Triangle_2D* triangle, mat3* PCM, vec4 color)
{
	glUseProgram(solidColorQuadShaderProgram);
	glBindVertexArray(equalateralTriangleVAO);
	glUniformMatrix3fv(solidColorQuadPCMLocation, 1, GL_FALSE, &(*PCM)[0][0]);
	glUniform4fv(solidColorQuadQuadColorLocation, 1, &color[0]);
	glDrawArrays(GL_TRIANGLES, 0, numVerticesTriangle);
}

inline void DrawCollidable(Collidable_CD2D* collidable, vec4 color, Camera_CD2D* camera)
{
	mat3 P_projection = camera->GetProjectionMatrix();
	mat3 C_camera = TranslationMatrix(camera->position);
	mat3 M_model = collidable->transform.LocalToWorldTransform();
	mat3 PCM = P_projection * inverse(C_camera) * M_model;

	switch (collidable->mode)
	{
	case Collidable_CD2D::Mode_Rectangle:
	DrawRectangle(&collidable->rectangle, &PCM, color);
	break;

	case Collidable_CD2D::Mode_Circle:
	DrawCircle(&collidable->circle, collidable->transform.scale, &PCM, color);
	break;

	case Collidable_CD2D::Mode_Triangle:
	DrawTriangle(&collidable->triangle, &PCM, color);
	break;

	default:
	break;
	}
}

void UpdateCollidable(Collidable_CD2D* collidable, F32 dt)
{
#define MOVEMENTSPEED 1.0f
#define SCALESPEED 0.5f
#define ROTATIONSPEED 45.0f

	// Update collidable's position
	if (GetKey(KeyCode_W))
	{
		collidable->transform.position.y += MOVEMENTSPEED * dt;
	}
	if (GetKey(KeyCode_S))
	{
		collidable->transform.position.y -= MOVEMENTSPEED * dt;
	}
	if (GetKey(KeyCode_A))
	{
		collidable->transform.position.x -= MOVEMENTSPEED * dt;
	}
	if (GetKey(KeyCode_D))
	{
		collidable->transform.position.x += MOVEMENTSPEED * dt;
	}

	// Update collidables scale
	if (GetKey(KeyCode_R))
	{
		collidable->transform.scale += SCALESPEED * dt;
	}
	if (GetKey(KeyCode_F))
	{
		collidable->transform.scale -= SCALESPEED * dt;
	}

	// Update collidables rotation
	if (GetKey(KeyCode_Q))
	{
		collidable->transform.rotationAngle += ROTATIONSPEED * dt;
	}
	if (GetKey(KeyCode_E))
	{
		collidable->transform.rotationAngle -= ROTATIONSPEED * dt;
	}

	// Update collidables mode
	if (GetKeyDown(KeyCode_2))
	{
		collidable->mode = (Collidable_CD2D::Mode)((I32)collidable->mode + 1);
		if (collidable->mode == Collidable_CD2D::Mode_Count)
		{
			collidable->mode = Collidable_CD2D::Mode_Rectangle;
		}
		DebugPrintf(512, "collidable mode: %i\n", collidable->mode);
	}
}


typedef std::vector<vec2> PointCloud;

PointCloud CreateCollidablePointCloud(Collidable_CD2D* shape)
{
	PointCloud result;

	mat3 lToW = shape->transform.LocalToWorldTransform();
	switch (shape->mode)
	{
	case Collidable_CD2D::Mode_Rectangle:
	{
											result.push_back(vec2(lToW * vec3(-shape->rectangle.halfDim.x, -shape->rectangle.halfDim.y, 1.0f)));
											result.push_back(vec2(lToW * vec3(-shape->rectangle.halfDim.x,  shape->rectangle.halfDim.y, 1.0f)));
											result.push_back(vec2(lToW * vec3( shape->rectangle.halfDim.x, -shape->rectangle.halfDim.y, 1.0f)));
											result.push_back(vec2(lToW * vec3( shape->rectangle.halfDim.x,  shape->rectangle.halfDim.y, 1.0f)));
	}
	break;

	case Collidable_CD2D::Mode_Circle:
	{
										 vec2 direction = vec2(1, 0);
										 F32 angleIncrement = 360.0f / (F32)NUMBER_OF_CIRCLE_POINTS;
										 F32 angle = 0;
										 for (U32 i = 0; i < NUMBER_OF_CIRCLE_POINTS; ++i, angle += angleIncrement)
										 {
											 vec2 rotatedDirection = RotationMatrix2x2_2D(angle) * direction;
											 result.push_back(Support(shape->circle, shape->transform, rotatedDirection));
										 }
	}
	break;

	case Collidable_CD2D::Mode_Triangle:
	{
										   result.push_back(vec2(lToW * vec3(shape->triangle.points[0].x, shape->triangle.points[0].y, 1.0f)));
										   result.push_back(vec2(lToW * vec3(shape->triangle.points[1].x, shape->triangle.points[1].y, 1.0f)));
										   result.push_back(vec2(lToW * vec3(shape->triangle.points[2].x, shape->triangle.points[2].y, 1.0f)));
	}
	break;
	}

	return result;
}

PointCloud CreateMinkowskiDifferencePointCloud(Collidable_CD2D* shape1, Collidable_CD2D* shape2)
{

	PointCloud result;

	PointCloud shape1PointCloud = CreateCollidablePointCloud(shape1);
	PointCloud shape2PointCloud = CreateCollidablePointCloud(shape2);

	for (U64 i = 0; i < shape1PointCloud.size(); ++i)
	{
		vec2 shape1Point = shape1PointCloud[i];

		for (U64 j = 0; j < shape2PointCloud.size(); ++j)
		{
			vec2 shape2Point = shape2PointCloud[j];
			result.push_back(shape1Point - shape2Point);
		}
	}

	// Dealloc shape pointclouds?
	return result;
}

PointCloud RemoveDuplicatePoints(PointCloud* points)
{
	PointCloud uniquePoints;

	for (size_t i = 0; i < points->size(); ++i)
	{
		bool unique = true;

		for (size_t j = 0; j < uniquePoints.size(); ++j)
		{
			if ((*points)[i] == uniquePoints[j])
			{
				unique = false;
				break;
			}
		}

		if (unique)
		{
			uniquePoints.push_back((*points)[i]);
		}
	}

	return uniquePoints;
}

GJKInfo_2D GJKDispatch(Collidable_CD2D* collidable1, Collidable_CD2D* collidable2, U64 iterationCount = MAXGJKITERATIONS_2D)
{
	GJKInfo_2D result;

	switch (collidable1->mode)
	{
	case Collidable_CD2D::Mode_Rectangle:
	{
											switch (collidable2->mode)
											{
											case Collidable_CD2D::Mode_Rectangle:
											{
																					result = GJK_2D(collidable1->rectangle, collidable1->transform, collidable2->rectangle, collidable2->transform, iterationCount);
											}
											break;

											case Collidable_CD2D::Mode_Circle:
											{
																				 result = GJK_2D(collidable1->rectangle, collidable1->transform, collidable2->circle, collidable2->transform, iterationCount);
											}
											break;

											case Collidable_CD2D::Mode_Triangle:
											{
																				   result = GJK_2D(collidable1->rectangle, collidable1->transform, collidable2->triangle, collidable2->transform, iterationCount);
											}
											break;

											default:
											break;
											}
	}
	break;

	case Collidable_CD2D::Mode_Circle:
	{
										 switch (collidable2->mode)
										 {
										 case Collidable_CD2D::Mode_Rectangle:
										 {
																				 result = GJK_2D(collidable1->circle, collidable1->transform, collidable2->rectangle, collidable2->transform, iterationCount);
										 }
										 break;

										 case Collidable_CD2D::Mode_Circle:
										 {
																			  result = GJK_2D(collidable1->circle, collidable1->transform, collidable2->circle, collidable2->transform, iterationCount);
										 }
										 break;

										 case Collidable_CD2D::Mode_Triangle:
										 {
																				result = GJK_2D(collidable1->circle, collidable1->transform, collidable2->triangle, collidable2->transform, iterationCount);
										 }
										 break;

										 default:
										 break;
										 }
	}
	break;

	case Collidable_CD2D::Mode_Triangle:
	{
										   switch (collidable2->mode)
										   {
										   case Collidable_CD2D::Mode_Rectangle:
										   {
																				   result = GJK_2D(collidable1->triangle, collidable1->transform, collidable2->rectangle, collidable2->transform, iterationCount);
										   }
										   break;

										   case Collidable_CD2D::Mode_Circle:
										   {
																				result = GJK_2D(collidable1->triangle, collidable1->transform, collidable2->circle, collidable2->transform, iterationCount);
										   }
										   break;

										   case Collidable_CD2D::Mode_Triangle:
										   {
																				  result = GJK_2D(collidable1->triangle, collidable1->transform, collidable2->triangle, collidable2->transform, iterationCount);
										   }
										   break;

										   default:
										   break;
										   }
	}
	break;

	default:
	break;
	}

	return result;
}

EPAInfo_2D EPADispatch(Collidable_CD2D* collidable1, Collidable_CD2D* collidable2, Simplex_2D simplex, U64 iterationCount = MAXEPAITERATIONS_2D)
{
	EPAInfo_2D result;

	switch (collidable1->mode)
	{
	case Collidable_CD2D::Mode_Rectangle:
	{
											switch (collidable2->mode)
											{
											case Collidable_CD2D::Mode_Rectangle:
											{
																					result = EPA_2D(collidable1->rectangle, collidable1->transform, collidable2->rectangle, collidable2->transform, simplex, iterationCount);
											}
											break;

											case Collidable_CD2D::Mode_Circle:
											{
																				 result = EPA_2D(collidable1->rectangle, collidable1->transform, collidable2->circle, collidable2->transform, simplex, iterationCount);
											}
											break;

											case Collidable_CD2D::Mode_Triangle:
											{
																				   result = EPA_2D(collidable1->rectangle, collidable1->transform, collidable2->triangle, collidable2->transform, simplex, iterationCount);
											}
											break;

											default:
											break;
											}
	}
	break;

	case Collidable_CD2D::Mode_Circle:
	{
										 switch (collidable2->mode)
										 {
										 case Collidable_CD2D::Mode_Rectangle:
										 {
																				 result = EPA_2D(collidable1->circle, collidable1->transform, collidable2->rectangle, collidable2->transform, simplex, iterationCount);
										 }
										 break;

										 case Collidable_CD2D::Mode_Circle:
										 {
																			  result = EPA_2D(collidable1->circle, collidable1->transform, collidable2->circle, collidable2->transform, simplex, iterationCount);
										 }
										 break;

										 case Collidable_CD2D::Mode_Triangle:
										 {
																				result = EPA_2D(collidable1->circle, collidable1->transform, collidable2->triangle, collidable2->transform, simplex, iterationCount);
										 }
										 break;

										 default:
										 break;
										 }
	}
	break;

	case Collidable_CD2D::Mode_Triangle:
	{
										   switch (collidable2->mode)
										   {
										   case Collidable_CD2D::Mode_Rectangle:
										   {
																				   result = EPA_2D(collidable1->triangle, collidable1->transform, collidable2->rectangle, collidable2->transform, simplex, iterationCount);
										   }
										   break;

										   case Collidable_CD2D::Mode_Circle:
										   {
																				result = EPA_2D(collidable1->triangle, collidable1->transform, collidable2->circle, collidable2->transform, simplex, iterationCount);
										   }
										   break;

										   case Collidable_CD2D::Mode_Triangle:
										   {
																				  result = EPA_2D(collidable1->triangle, collidable1->transform, collidable2->triangle, collidable2->transform, simplex, iterationCount);
										   }
										   break;

										   default:
										   break;
										   }
	}
	break;

	default:
	break;
	}

	return result;
}

GJKInfo_2D gjkResults;
EPAInfo_2D epaResults;
inline void UpdateAppletState(F32 dt)
{
	// Update applet state
	if (GetKeyDown(KeyCode_1))
	{
		controllingShape1_CD2D = !controllingShape1_CD2D;
	}

	// Update collidables
	if (controllingShape1_CD2D)
	{
		UpdateCollidable(&shape1_CD2D, dt);
	}
	else
	{
		UpdateCollidable(&shape2_CD2D, dt);
	}

	gjkResults = GJKDispatch(&shape1_CD2D, &shape2_CD2D);
	if (gjkResults.collided)
	{
		color_CD2D = vec4(0.298f, 0.686f, 0.314f, 1.0f);
	}
	else
	{
		color_CD2D = vec4(0.957f, 0.263f, 0.212f, 1.0f);
	}

	if (gjkResults.collided)
	{
		epaResults = EPADispatch(&shape1_CD2D, &shape2_CD2D, gjkResults.simplex);
	}
}

inline void DrawCollidablesVisualization()
{
	// Draw grid
	DrawGrid(gridColor, &camera_CD2D);
	DrawLine(vec2(0, -100), vec2(0, 100), yAxisColor, &camera_CD2D);
	DrawLine(vec2(-100, 0), vec2(100, 0), xAxisColor, &camera_CD2D);

	// Draw collidables
	DrawCollidable(&shape1_CD2D, color_CD2D, &camera_CD2D);
	DrawCollidable(&shape2_CD2D, color_CD2D, &camera_CD2D);
}

inline void DrawGJKVisualization()
{
	// Draw grid
	DrawGrid(gridColor, &camera_CD2D);
	DrawLine(vec2(0, -100), vec2(0, 100), yAxisColor, &camera_CD2D);
	DrawLine(vec2(-100, 0), vec2(100, 0), xAxisColor, &camera_CD2D);

	// Draw Minkowski Difference
	PointCloud minkowskiDifference = CreateMinkowskiDifferencePointCloud(&shape1_CD2D, &shape2_CD2D);
	for (U64 i = 0; i < minkowskiDifference.size(); ++i)
	{
		DrawPoint(minkowskiDifference[i], minkowskiPointSize_px, minkowskiPointColor, &camera_CD2D);
	}

	// Draw GJK points
	switch (gjkResults.simplex.type)
	{
	case Simplex_2D::Simplex_Point_2D:
	DrawPoint(gjkResults.simplex.A, gjkPointSize_px, gjkPointColor, &camera_CD2D);
	break;

	case Simplex_2D::Simplex_Line_2D:
	DrawPoint(gjkResults.simplex.A, gjkPointSize_px, gjkPointColor, &camera_CD2D);
	DrawPoint(gjkResults.simplex.B, gjkPointSize_px, gjkPointColor, &camera_CD2D);
	DrawLine(gjkResults.simplex.A, gjkResults.simplex.B, gjkPointColor, &camera_CD2D);
	break;

	case Simplex_2D::Simplex_Triangle_2D:
	DrawPoint(gjkResults.simplex.A, gjkPointSize_px, gjkPointColor, &camera_CD2D);
	DrawPoint(gjkResults.simplex.B, gjkPointSize_px, gjkPointColor, &camera_CD2D);
	DrawPoint(gjkResults.simplex.C, gjkPointSize_px, gjkPointColor, &camera_CD2D);
	DrawLine(gjkResults.simplex.A, gjkResults.simplex.B, gjkPointColor, &camera_CD2D);
	DrawLine(gjkResults.simplex.B, gjkResults.simplex.C, gjkPointColor, &camera_CD2D);
	DrawLine(gjkResults.simplex.C, gjkResults.simplex.A, gjkPointColor, &camera_CD2D);
	break;

	default:
	break;
	}

	
	// Draw EPA
	if (gjkResults.collided)
	{
		DrawLine(vec2(0, 0), epaResults.normal * epaResults.distance, vec4(1, 1, 1, 1), &camera_CD2D);
	}
}

inline void UpdateCollisionDetection2DApplet(F32 dt)
{
	UpdateAppletState(dt);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	//         x  y  x    y
	glViewport(0, 0, 800, 800);
	DrawCollidablesVisualization();

	glViewport(800, 0, 800, 800);
	DrawGJKVisualization();

	glViewport(0, 0, 1600, 1600);
	Rectangle_2D sep;
	mat3 sepPCM = mat3(.005,0,0, 0,1,0, 0,0,1);
	DrawRectangle(&sep, &sepPCM, seperatorColor);
	DrawLine(vec2(0, -10), vec2(0, 10), seperatorColor, &camera_CD2D);
}

inline void ShutdownCollisionDetection2DApplet()
{
}