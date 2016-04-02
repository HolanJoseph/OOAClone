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
		Mode_Circle,
		Mode_Triangle,

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

Camera_CD2D     camera_CD2D;
Collidable_CD2D shape1_CD2D;
Collidable_CD2D shape2_CD2D;
vec4			color_CD2D;
bool			controllingShape1_CD2D;

#define NUMBER_OF_TRIANGLE_POINTS 3
#define NUMBER_OF_RECTANGLE_POINTS 4
#define NUMBER_OF_CIRCLE_POINTS 20

#define pointSize_px 5

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

	shape1_CD2D.transform.position = vec2(1, 0);
	shape2_CD2D.transform.position = vec2(-1, 0);

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

inline void DrawPoint(vec2 p, vec4 color, Camera_CD2D* camera)
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
	glPointSize(pointSize_px);
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

inline void DrawCircle(Circle_2D* circle, mat3* PCM, vec4 color)
{
	glUseProgram(solidColorCircleInPointShaderProgram);
	glBindVertexArray(circleVAO);
	glUniformMatrix3fv(solidColorCircleInPointPCMLocation, 1, GL_FALSE, &(*PCM)[0][0]);
	glUniform4fv(solidColorCircleInPointCircleColorLocation, 1, &color[0]);
	glPointSize(circle->radius * unitCircleSize_px);
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
	DrawCircle(&collidable->circle, &PCM, color);
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


struct PointCloud
{
	vec2* points;
	U64 count;

	PointCloud()
	{
		points = NULL;
		count = 0;
	}
};

void DestroyPointCloud(PointCloud* pointClound)
{
	free(pointClound->points);
	pointClound->count = 0;
}

PointCloud CreateCollidablePointCloud(Collidable_CD2D* shape)
{
	PointCloud result;

	mat3 lToW = shape->transform.LocalToWorldTransform();
	switch (shape->mode)
	{
	case Collidable_CD2D::Mode_Rectangle:
	{
											result.count = NUMBER_OF_RECTANGLE_POINTS;
											result.points = (vec2*)malloc(sizeof(vec2)*result.count);
											result.points[0] = vec2(lToW * vec3(-shape->rectangle.halfDim.x, -shape->rectangle.halfDim.y, 1.0f));
											result.points[1] = vec2(lToW * vec3(-shape->rectangle.halfDim.x,  shape->rectangle.halfDim.y, 1.0f));
											result.points[2] = vec2(lToW * vec3( shape->rectangle.halfDim.x, -shape->rectangle.halfDim.y, 1.0f));
											result.points[3] = vec2(lToW * vec3( shape->rectangle.halfDim.x,  shape->rectangle.halfDim.y, 1.0f));
	}
	break;

	case Collidable_CD2D::Mode_Circle:
	{
										 result.count = NUMBER_OF_CIRCLE_POINTS;
										 result.points = (vec2*)malloc(sizeof(vec2)*result.count);
										 vec2 direction = vec2(1, 0);
										 F32 angleIncrement = 360.0f / (F32)NUMBER_OF_CIRCLE_POINTS;
										 F32 angle = 0;
										 for (U32 i = 0; i < NUMBER_OF_CIRCLE_POINTS; ++i, angle += angleIncrement)
										 {
											 vec2 rotatedDirection = RotationMatrix2x2_2D(angle) * direction;
											 result.points[i] = Support(shape->circle, shape->transform, rotatedDirection);
										 }
	}
	break;

	case Collidable_CD2D::Mode_Triangle:
	{
										   result.count = NUMBER_OF_TRIANGLE_POINTS;
										   result.points = (vec2*)malloc(sizeof(vec2)*result.count);
										   result.points[0] = vec2(lToW * vec3(shape->triangle.points[0].x, shape->triangle.points[0].y, 1.0f));
										   result.points[1] = vec2(lToW * vec3(shape->triangle.points[1].x, shape->triangle.points[1].y, 1.0f));
										   result.points[2] = vec2(lToW * vec3(shape->triangle.points[2].x, shape->triangle.points[2].y, 1.0f));
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

	result.count = shape1PointCloud.count * shape2PointCloud.count;
	result.points = (vec2*)malloc(sizeof(vec2)*result.count);

	for (U64 i = 0; i < shape1PointCloud.count; ++i)
	{
		vec2 shape1Point = shape1PointCloud.points[i];

		for (U64 j = 0; j < shape2PointCloud.count; ++j)
		{
			vec2 shape2Point = shape2PointCloud.points[j];
			result.points[(i*shape2PointCloud.count) + j] = shape1Point - shape2Point;
		}
	}


	DestroyPointCloud(&shape1PointCloud);
	DestroyPointCloud(&shape2PointCloud);
	return result;
}

PointCloud CreateConvexHull(Collidable_CD2D* shape1, Collidable_CD2D* shape2)
{
	PointCloud result;
	return result;
}

inline void UpdateCollisionDetection2DApplet(F32 dt)
{
	glViewport(0, 0, 800, 800);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

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

	// Draw grid
	DrawGrid(gridColor, &camera_CD2D);
	DrawLine(vec2(0, -10), vec2(0, 10), yAxisColor, &camera_CD2D);
	DrawLine(vec2(-100, 0), vec2(100, 0), xAxisColor, &camera_CD2D);

	// Draw collidables
	DrawCollidable(&shape1_CD2D, color_CD2D, &camera_CD2D);
	DrawCollidable(&shape2_CD2D, color_CD2D, &camera_CD2D);
	
	// Draw convex hull
	PointCloud convexHull = CreateMinkowskiDifferencePointCloud(&shape1_CD2D, &shape2_CD2D);
	for (U64 i = 0; i < convexHull.count; ++i)
	{
		DrawPoint(convexHull.points[i], vec4(1, .5, 1, 1), &camera_CD2D);
	}
	DestroyPointCloud(&convexHull);
	// Draw GJK points 
	// Draw GJK line segments
	
}



inline void ShutdownCollisionDetection2DApplet()
{
}