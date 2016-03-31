#pragma once

/*
 * Controls
 * 
 * Right Handed Coordinates
 * 
 * WASD - move the controlled shape in the xy plane
 * QE   - rotate the controlled shape around the z axis
 * 1	- switch controlled shape
 * 2	- switch the controlled shapes shape (Rectangle -> Circle -> Triangle)
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

extern GLuint texturedQuadVAO;
extern GLuint circleVAO;
extern GLuint equalateralTriangleVAO;

extern const GLuint numVertices; // Number of vertices in the textured quad
#define numVerticesTriangle 6
#define unitCircleSize_px 160.0f

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

	vec2 position;
	F32  rotationAngle;
	F32  scale;

	Collidable_CD2D()
	{
		mode = Collidable_CD2D::Mode_Rectangle;
		position = vec2(0, 0);
		rotationAngle = 0;
		scale = 1;
	}

	mat3 LocalToWorldTransform()
	{
		mat3 result = TranslationMatrix(position) * RotationMatrix_2D(rotationAngle) * ScaleMatrix(vec2(scale, scale));
		return result;
	}
};

Camera_CD2D     camera_CD2D;
Collidable_CD2D shape1_CD2D;
Collidable_CD2D shape2_CD2D;
vec4			color_CD2D;
bool			controllingShape1_CD2D;

inline void InitializeCollisionDetection2DApplet()
{
	shape1_CD2D.position = vec2(1, 0);
	shape2_CD2D.position = vec2(-1, 0);

	camera_CD2D.ResizeViewArea(vec2(5,5));

	color_CD2D = vec4(.933, .933, .933, 1);

	controllingShape1_CD2D = true;
}



inline void DrawGrid(vec2 axis1, vec2 axis2)
{

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
	mat3 M_model = collidable->LocalToWorldTransform();
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

	// Update collidable's position
	vec2 translationVector = vec2(0, 0);
	if (GetKey(KeyCode_W))
	{
		translationVector.y += MOVEMENTSPEED;
	}
	if (GetKey(KeyCode_S))
	{
		translationVector.y -= MOVEMENTSPEED;
	}
	if (GetKey(KeyCode_A))
	{
		translationVector.x -= MOVEMENTSPEED;
	}
	if (GetKey(KeyCode_D))
	{
		translationVector.x += MOVEMENTSPEED;
	}
	collidable->position += translationVector * dt;

	// Update collidables scale

	// Update collidables rotation

	// Update collidables mode
	// key 2
}

inline void UpdateCollisionDetection2DApplet(F32 dt)
{
	glViewport(0, 0, 800, 800);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Update applet state
	if (GetKey(KeyCode_1))
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

	// Draw collidables
	DrawCollidable(&shape1_CD2D, color_CD2D, &camera_CD2D);
	DrawCollidable(&shape2_CD2D, color_CD2D, &camera_CD2D);
}



inline void ShutdownCollisionDetection2DApplet()
{
}