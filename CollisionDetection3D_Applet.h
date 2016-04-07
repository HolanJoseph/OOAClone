#pragma once

#include "Types.h"
#include "Math.h"

#include "DebugAPI.h"
#include "InputAPI.h"

#include "AssetLoading.h"

#include "CollisionDetection.h"

#include "glew/GL/glew.h"
#include <gl/GL.h>

extern GLuint solidColorTriangleShaderProgram;
extern GLuint solidColorTrianglePCMLocation;
extern GLuint solidColorTriangleTriangleColorLocation;
extern GLuint texturedQuadShaderProgram;

struct Camera
{
	vec2 position;
	vec2 viewArea;
};

struct ViewFrustum
{
	F32 left;
	F32 right;
	F32 top;
	F32 bottom;
	F32 near;
	F32 far;
};

ViewFrustum CreateViewFrustum(F32 xFOV, F32 yFOV, F32 near, F32 far)
{
	ViewFrustum result;

	result.near = near;
	result.far = far;

	result.right = near * tan(DegreesToRadians(xFOV) / 2.0f);
	result.left = -result.right;

	result.top = near * tan(DegreesToRadians(yFOV) / 2.0f);
	result.bottom = -result.top;

	return result;
}

mat4 GetPerspectiveProjection(ViewFrustum frustum)
{
	mat4 result;

	result[0] = vec4((2.0f*frustum.near) / (frustum.right - frustum.left), 0.0f, 0.0f, 0.0f);
	result[1] = vec4(0.0f, (2.0f*frustum.near) / (frustum.top - frustum.bottom), 0.0f, 0.0f);
	result[2] = vec4((frustum.right + frustum.left) / (frustum.right - frustum.left), (frustum.top + frustum.bottom) / (frustum.top - frustum.bottom), -(frustum.far + frustum.near) / (frustum.far - frustum.near), -1.0f);
	result[3] = vec4(0.0f, 0.0f, -(2.0f*frustum.far*frustum.near) / (frustum.far - frustum.near), 0.0f);

	return result;
}


GLuint cubeVAO;
U64 numberOfCubeVertices;
U64 numberOfCubeIndices;

GLuint capsuleVAO;
U64 numberOfCapsuleWallVertices;
U64 numberOfCapsuleWallIndices;

GLuint sphereVAO;
U64 numberOfSphereVertices;
U64 numberOfSphereIndices;

vec3 camera3DPosition;
vec3 camera3DRotations;
ViewFrustum camera3DViewFrustum;

Camera gjkTestCamera;

OrientedBoundingBox collisionShape1_obb;
Sphere				collisionShape1_sphere;
Capsule				collisionShape1_capsule;
OrientedBoundingBox collisionShape2_obb;
Sphere				collisionShape2_sphere;
Capsule				collisionShape2_capsule;

GLuint frontTexture;
GLuint rightTexture;
GLuint topTexture;

GLuint front_1_Texture;
GLuint right_1_Texture;
GLuint top_1_Texture;

GLuint front_2_Texture;
GLuint right_2_Texture;
GLuint top_2_Texture;

vec4 darkRed = vec4(0.898f, 0.224f, 0.208f, 1.0f);
vec4 lightRed = vec4(0.957f, 0.263f, 0.212f, 1.0f);

vec4 darkGreen = vec4(0.263f, 0.627f, 0.278f, 1.0f);
vec4 lightGreen = vec4(0.298f, 0.686f, 0.314f, 1.0f);

inline void InitializeCollisionDetection3DApplet()
{
	F32* cubeVertices = NULL;
	U32* cubeIndices = NULL;
	Load3DModel("Assets/3D/Cube.obj", &cubeVertices, &numberOfCubeVertices, &cubeIndices, &numberOfCubeIndices);
	glGenVertexArrays(1, &cubeVAO);
	glBindVertexArray(cubeVAO);
	GLuint cubeVertexBuffer;
	glGenBuffers(1, &cubeVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices[0])*numberOfCubeVertices, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cubeVertices[0])*numberOfCubeVertices, cubeVertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	GLuint cubeIndexBuffer;
	glGenBuffers(1, &cubeIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices[0])*numberOfCubeIndices, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(cubeIndices[0])*numberOfCubeIndices, cubeIndices);
	glBindVertexArray(NULL);



	F32* capsuleWallVertices = NULL;
	U32* capsuleWallIndices = NULL;
	Load3DModel("Assets/3D/CapsuleWall.obj", &capsuleWallVertices, &numberOfCapsuleWallVertices, &capsuleWallIndices, &numberOfCapsuleWallIndices);
	glGenVertexArrays(1, &capsuleVAO);
	glBindVertexArray(capsuleVAO);
	GLuint capsuleVertexBuffer;
	glGenBuffers(1, &capsuleVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, capsuleVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(capsuleWallVertices[0])*numberOfCapsuleWallVertices, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(capsuleWallVertices[0])*numberOfCapsuleWallVertices, capsuleWallVertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	GLuint capsuleIndexBuffer;
	glGenBuffers(1, &capsuleIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, capsuleIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(capsuleWallIndices[0])*numberOfCapsuleWallIndices, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(capsuleWallIndices[0])*numberOfCapsuleWallIndices, capsuleWallIndices);
	glBindVertexArray(NULL);


	F32* sphereVertices = NULL;
	U32* sphereIndices = NULL;
	Load3DModel("Assets/3D/sphere.obj", &sphereVertices, &numberOfSphereVertices, &sphereIndices, &numberOfSphereIndices);
	glGenVertexArrays(1, &sphereVAO);
	glBindVertexArray(sphereVAO);
	GLuint sphereVertexBuffer;
	glGenBuffers(1, &sphereVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphereVertices[0])*numberOfSphereVertices, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(sphereVertices[0])*numberOfSphereVertices, sphereVertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	GLuint sphereIndexBuffer;
	glGenBuffers(1, &sphereIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphereIndices[0])*numberOfSphereIndices, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(sphereIndices[0])*numberOfSphereIndices, sphereIndices);
	glBindVertexArray(NULL);

	// ortho camera
	gjkTestCamera.viewArea = vec2(10, 10);

	// Create 3D Camera
	camera3DPosition = vec3(0, 0, -5);
	camera3DRotations = vec3(0, 0, 0);
	//camera3DViewFrustum = CreateViewFrustum(55.64f, 33.07f, 1.0f / 3.0f, 100.0f); // NOTE: These FOV values are for fullscreen
	camera3DViewFrustum = CreateViewFrustum(2 * 24.866f, 2 * 24.866f, 1.0f / 3.0f, 100.0f);



	I32 textureWidth = 0;
	I32 textureHeight = 0;
	I32 textureImageComponents = 0;
	I32 textureNumImageComponentsDesired = 4;
	U8 * textureData = LoadTexture("Assets/3D/Front.bmp", &textureWidth, &textureHeight, &textureImageComponents, textureNumImageComponentsDesired); // Channel order?

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &frontTexture);
	glBindTexture(GL_TEXTURE_2D, frontTexture);
	glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, textureWidth, textureHeight);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
	free(textureData);


	textureWidth = 0;
	textureHeight = 0;
	textureImageComponents = 0;
	textureNumImageComponentsDesired = 4;
	textureData = LoadTexture("Assets/3D/Right.bmp", &textureWidth, &textureHeight, &textureImageComponents, textureNumImageComponentsDesired); // Channel order?

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &rightTexture);
	glBindTexture(GL_TEXTURE_2D, rightTexture);
	glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, textureWidth, textureHeight);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
	free(textureData);


	textureWidth = 0;
	textureHeight = 0;
	textureImageComponents = 0;
	textureNumImageComponentsDesired = 4;
	textureData = LoadTexture("Assets/3D/Top.bmp", &textureWidth, &textureHeight, &textureImageComponents, textureNumImageComponentsDesired); // Channel order?

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &topTexture);
	glBindTexture(GL_TEXTURE_2D, topTexture);
	glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, textureWidth, textureHeight);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
	free(textureData);



	textureWidth = 0;
	textureHeight = 0;
	textureImageComponents = 0;
	textureNumImageComponentsDesired = 4;
	textureData = LoadTexture("Assets/3D/Front1.bmp", &textureWidth, &textureHeight, &textureImageComponents, textureNumImageComponentsDesired); // Channel order?

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &front_1_Texture);
	glBindTexture(GL_TEXTURE_2D, front_1_Texture);
	glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, textureWidth, textureHeight);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
	free(textureData);


	textureWidth = 0;
	textureHeight = 0;
	textureImageComponents = 0;
	textureNumImageComponentsDesired = 4;
	textureData = LoadTexture("Assets/3D/Right1.bmp", &textureWidth, &textureHeight, &textureImageComponents, textureNumImageComponentsDesired); // Channel order?

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &right_1_Texture);
	glBindTexture(GL_TEXTURE_2D, right_1_Texture);
	glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, textureWidth, textureHeight);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
	free(textureData);


	textureWidth = 0;
	textureHeight = 0;
	textureImageComponents = 0;
	textureNumImageComponentsDesired = 4;
	textureData = LoadTexture("Assets/3D/Top1.bmp", &textureWidth, &textureHeight, &textureImageComponents, textureNumImageComponentsDesired); // Channel order?

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &top_1_Texture);
	glBindTexture(GL_TEXTURE_2D, top_1_Texture);
	glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, textureWidth, textureHeight);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
	free(textureData);



	textureWidth = 0;
	textureHeight = 0;
	textureImageComponents = 0;
	textureNumImageComponentsDesired = 4;
	textureData = LoadTexture("Assets/3D/Front2.bmp", &textureWidth, &textureHeight, &textureImageComponents, textureNumImageComponentsDesired); // Channel order?

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &front_2_Texture);
	glBindTexture(GL_TEXTURE_2D, front_2_Texture);
	glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, textureWidth, textureHeight);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
	free(textureData);


	textureWidth = 0;
	textureHeight = 0;
	textureImageComponents = 0;
	textureNumImageComponentsDesired = 4;
	textureData = LoadTexture("Assets/3D/Right2.bmp", &textureWidth, &textureHeight, &textureImageComponents, textureNumImageComponentsDesired); // Channel order?

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &right_2_Texture);
	glBindTexture(GL_TEXTURE_2D, right_2_Texture);
	glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, textureWidth, textureHeight);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
	free(textureData);


	textureWidth = 0;
	textureHeight = 0;
	textureImageComponents = 0;
	textureNumImageComponentsDesired = 4;
	textureData = LoadTexture("Assets/3D/Top2.bmp", &textureWidth, &textureHeight, &textureImageComponents, textureNumImageComponentsDesired); // Channel order?

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &top_2_Texture);
	glBindTexture(GL_TEXTURE_2D, top_2_Texture);
	glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, textureWidth, textureHeight);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
	free(textureData);




	collisionShape1_obb.halfDim = vec3(1, 1, 1);
	collisionShape1_sphere.radius = 1;
	collisionShape1_capsule.points[0] = vec3(0, 1, 0);
	collisionShape1_capsule.points[1] = vec3(0, -1, 0);
	collisionShape1_capsule.radius = 1;

	collisionShape2_obb.halfDim = vec3(1, 1, 1);
	collisionShape2_sphere.radius = 1;
	collisionShape2_capsule.points[0] = vec3(0, 1, 0);
	collisionShape2_capsule.points[1] = vec3(0, -1, 0);
	collisionShape2_capsule.radius = 1;

}

enum GJKMode
{
	gjk_Simple,
	//gjk_Casey,
	gjk_AllVoronoi,

	gjk_COUNT
};
GJKMode gjkMode;

F32 movementSpeed = 2.0f;
F32 rotationSpeed = 45.0f; // In degrees

bool controllingShape1 = true;

enum csType3D
{
	cs3D_OrientedBoundingBox,
	cs3D_Sphere,
	cs3D_Capsule,

	cs3D_COUNT
};

struct CollisionShape_Data
{
	vec4 color;
	vec3 position;
	vec3 rotation;
	csType3D type;
};

CollisionShape_Data* controlledCollisionShape = NULL;
CollisionShape_Data collisionShape1_Data;
CollisionShape_Data collisionShape2_Data;

F32 cameraMovementSpeed = 2.0f;
F32 cameraRotationSpeed = 30.0f;

F32 cameraDistance = 10.0f;
F32 cameraAzimuthAngle = 0.0f;
F32 cameraPolarAngle = 0.0f;
F32 cameraHeight = 3.0f;

enum CameraMode
{
	Orthographic_Front,
	Orthographic_Right,
	Orthographic_Top,
	Orthographic_Front_1,
	Orthographic_Right_1,
	Orthographic_Top_1,
	Orthographic_Front_2,
	Orthographic_Right_2,
	Orthographic_Top_2,

	CameraMode_COUNT
};
CameraMode gjkTestCameraMode = Orthographic_Front;

bool moved;

inline void UpdateCollisionDetection3DApplet(F32 deltaTime)
{
	moved = false;

	if (GetKeyDown(KeyCode_1))
	{
		controllingShape1 = !controllingShape1;
	}
	if (controllingShape1)
	{
		controlledCollisionShape = &collisionShape1_Data;
	}
	else
	{
		controlledCollisionShape = &collisionShape2_Data;
	}

	if (GetKey(KeyCode_W))
	{
		controlledCollisionShape->position.z += deltaTime * movementSpeed;
		moved = true;
	}
	if (GetKey(KeyCode_S))
	{
		controlledCollisionShape->position.z -= deltaTime * movementSpeed;
		moved = true;
	}
	if (GetKey(KeyCode_A))
	{
		controlledCollisionShape->position.x -= deltaTime * movementSpeed;
		moved = true;
	}
	if (GetKey(KeyCode_D))
	{
		controlledCollisionShape->position.x += deltaTime * movementSpeed;
		moved = true;
	}
	if (GetKey(KeyCode_R))
	{
		controlledCollisionShape->position.y += deltaTime * movementSpeed;
		moved = true;
	}
	if (GetKey(KeyCode_F))
	{
		controlledCollisionShape->position.y -= deltaTime * movementSpeed;
		moved = true;
	}
	if (GetKey(KeyCode_I))
	{
		controlledCollisionShape->rotation.x += deltaTime * rotationSpeed;
		moved = true;
	}
	if (GetKey(KeyCode_K))
	{
		controlledCollisionShape->rotation.x -= deltaTime * rotationSpeed;
		moved = true;
	}
	if (GetKey(KeyCode_J))
	{
		controlledCollisionShape->rotation.y -= deltaTime * rotationSpeed;
		moved = true;
	}
	if (GetKey(KeyCode_L))
	{
		controlledCollisionShape->rotation.y += deltaTime * rotationSpeed;
		moved = true;
	}
	if (GetKey(KeyCode_U))
	{
		controlledCollisionShape->rotation.z += deltaTime * rotationSpeed;
		moved = true;
	}
	if (GetKey(KeyCode_O))
	{
		controlledCollisionShape->rotation.z -= deltaTime * rotationSpeed;
		moved = true;
	}
	if (GetKeyDown(KeyCode_3))
	{
		controlledCollisionShape->type = (csType3D)(controlledCollisionShape->type + 1);
		if (controlledCollisionShape->type == cs3D_COUNT)
		{
			controlledCollisionShape->type = cs3D_OrientedBoundingBox;
		}
		DebugPrintf(512, "cs3d %i\n", controlledCollisionShape->type);
		moved = true;
	}

	if (GetKey(KeyCode_Up))
	{
		cameraPolarAngle += deltaTime * cameraRotationSpeed;
		cameraHeight += deltaTime * cameraMovementSpeed;
	}
	if (GetKey(KeyCode_Down))
	{
		cameraPolarAngle -= deltaTime * cameraRotationSpeed;
		cameraHeight -= deltaTime * cameraMovementSpeed;
	}
	if (GetKey(KeyCode_Left))
	{
		cameraAzimuthAngle -= deltaTime * cameraRotationSpeed;
	}
	if (GetKey(KeyCode_Right))
	{
		cameraAzimuthAngle += deltaTime * cameraRotationSpeed;
	}
	if (GetKey(KeyCode_Minus))
	{
		cameraDistance -= deltaTime * cameraMovementSpeed;
	}
	if (GetKey(KeyCode_Equal))
	{
		cameraDistance += deltaTime * cameraMovementSpeed;
	}

	if (GetKeyDown(KeyCode_2))
	{
		gjkTestCameraMode = (CameraMode)(gjkTestCameraMode + 1);
		if (gjkTestCameraMode == CameraMode_COUNT)
		{
			gjkTestCameraMode = Orthographic_Front;
		}
		//DebugPrintf(512, "CameraMode %i\n", gjkTestCameraMode);
	}

	if (GetKeyDown(KeyCode_4))
	{
		gjkMode = (GJKMode)(gjkMode + 1);
		if (gjkMode == gjk_COUNT)
		{
			gjkMode = gjk_Simple;
		}
		DebugPrintf(512, "gjkMode = %i\n", gjkMode);
		moved = true;
	}

	// 	if (abs(cameraPolarAngle) > 80.0f)
	// 	{
	// 		cameraPolarAngle = 80.0f;
	// 	}

	glViewport(0, 0, 800, 800);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// 3D GJK
	CollisionInfo collision_AllVoronoi;
	CollisionInfo collision_Simple;

	mat4 Bmodel_cs1 = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, collisionShape1_Data.position.x, collisionShape1_Data.position.y, collisionShape1_Data.position.z, 1) * RotationMatrix_Z(collisionShape1_Data.rotation.z) * RotationMatrix_Y(collisionShape1_Data.rotation.y) * RotationMatrix_X(collisionShape1_Data.rotation.x); //collisionShape1.transform;
	mat4 Bmodel_cs2 = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, collisionShape2_Data.position.x, collisionShape2_Data.position.y, collisionShape2_Data.position.z, 1) * RotationMatrix_Z(collisionShape2_Data.rotation.z) * RotationMatrix_Y(collisionShape2_Data.rotation.y) * RotationMatrix_X(collisionShape2_Data.rotation.x); //collisionShape1.transform;

	collisionShape1_obb.transform = Bmodel_cs1;
	collisionShape1_sphere.origin = vec3(Bmodel_cs1[3].x, Bmodel_cs1[3].y, Bmodel_cs1[3].z);
	collisionShape1_capsule.transform = Bmodel_cs1;

	collisionShape2_obb.transform = Bmodel_cs2;
	collisionShape2_sphere.origin = vec3(Bmodel_cs2[3].x, Bmodel_cs2[3].y, Bmodel_cs2[3].z);
	collisionShape2_capsule.transform = Bmodel_cs2;

	switch (collisionShape1_Data.type)
	{
	case cs3D_OrientedBoundingBox:
	{
									 switch (collisionShape2_Data.type)
									 {
									 case cs3D_OrientedBoundingBox:{
																	   // 																	   ConvexHullPoints convexHullPoints = GetConvexHullPoints((OrientedBoundingBox)collisionShape1_obb, (OrientedBoundingBox)collisionShape2_obb);
																	   // 																	   DebugPrint("plot points\n ");
																	   // 																	   for (U32 i = 0; i < convexHullPoints.count - 1; ++i)
																	   // 																	   {
																	   // 																		   DebugPrintf(256, "(%.1f,%.1f,%.1f),\n", convexHullPoints.points[i].x, convexHullPoints.points[i].y, convexHullPoints.points[i].z);
																	   // 																	   }
																	   // 																	   DebugPrintf(256, "(%.1f,%.1f,%.1f)\n", convexHullPoints.points[convexHullPoints.count - 1].x, convexHullPoints.points[convexHullPoints.count - 1].y, convexHullPoints.points[convexHullPoints.count - 1].z);
																	   if (gjkMode == gjk_Simple)
																	   {
																		   collision_Simple = DetectCollision(collisionShape1_obb, collisionShape2_obb);
																	   }
																	   else
																	   {
																		   collision_AllVoronoi = DetectCollision_AllVoronoi(collisionShape1_obb, collisionShape2_obb);
																	   }}
										 break;

									 case cs3D_Sphere:
									 if (gjkMode == gjk_Simple)
									 {
										 collision_Simple = DetectCollision(collisionShape1_obb, collisionShape2_sphere);
									 }
									 else
									 {
										 collision_AllVoronoi = DetectCollision_AllVoronoi(collisionShape1_obb, collisionShape2_sphere);
									 }
									 break;

									 case cs3D_Capsule:
									 if (gjkMode == gjk_Simple)
									 {
										 collision_Simple = DetectCollision(collisionShape1_obb, collisionShape2_capsule);
									 }
									 else
									 {
										 collision_AllVoronoi = DetectCollision_AllVoronoi(collisionShape1_obb, collisionShape2_capsule);
									 }
									 break;
									 }
	}
	break;

	case cs3D_Sphere:
	{
						switch (collisionShape2_Data.type)
						{
						case cs3D_OrientedBoundingBox:
						if (gjkMode == gjk_Simple)
						{
							collision_Simple = DetectCollision(collisionShape1_sphere, collisionShape2_obb);
						}
						else
						{
							collision_AllVoronoi = DetectCollision_AllVoronoi(collisionShape1_sphere, collisionShape2_obb);
						}
						break;

						case cs3D_Sphere:
						if (gjkMode == gjk_Simple)
						{
							collision_Simple = DetectCollision(collisionShape1_sphere, collisionShape2_sphere);
						}
						else
						{
							collision_AllVoronoi = DetectCollision_AllVoronoi(collisionShape1_sphere, collisionShape2_sphere);
						}
						break;

						case cs3D_Capsule:
						if (gjkMode == gjk_Simple)
						{
							collision_Simple = DetectCollision(collisionShape1_sphere, collisionShape2_capsule);
						}
						else
						{
							collision_AllVoronoi = DetectCollision_AllVoronoi(collisionShape1_sphere, collisionShape2_capsule);
						}
						break;
						}
	}
	break;

	case cs3D_Capsule:
	{
						 switch (collisionShape2_Data.type)
						 {
						 case cs3D_OrientedBoundingBox:
						 if (gjkMode == gjk_Simple)
						 {
							 collision_Simple = DetectCollision(collisionShape1_capsule, collisionShape2_obb);
						 }
						 else
						 {
							 collision_AllVoronoi = DetectCollision_AllVoronoi(collisionShape1_capsule, collisionShape2_obb);
						 }
						 break;

						 case cs3D_Sphere:
						 if (gjkMode == gjk_Simple)
						 {
							 collision_Simple = DetectCollision(collisionShape1_capsule, collisionShape2_sphere);
						 }
						 else
						 {
							 collision_AllVoronoi = DetectCollision_AllVoronoi(collisionShape1_capsule, collisionShape2_sphere);
						 }
						 break;

						 case cs3D_Capsule:
						 if (gjkMode == gjk_Simple)
						 {
							 collision_Simple = DetectCollision(collisionShape1_capsule, collisionShape2_capsule);
						 }
						 else
						 {
							 collision_AllVoronoi = DetectCollision_AllVoronoi(collisionShape1_capsule, collisionShape2_capsule);
						 }
						 break;
						 }
	}
	break;
	}

	//Assert(collision_AllVoronoi == collision_Simple);
	bool collision = false;
	F32 penetrationDepth = 100000.0f;
	if (gjkMode == gjk_Simple)
	{
		collision = collision_Simple.collided;
		penetrationDepth = collision_Simple.distance;
	}
	if (gjkMode == gjk_AllVoronoi)
	{
		collision = collision_AllVoronoi.collided;
		penetrationDepth = collision_AllVoronoi.distance;
	}

	if (collision/*collision_AllVoronoi*/)
	{
		collisionShape1_Data.color = darkGreen;
		collisionShape2_Data.color = lightGreen;
	}
	else
	{
		collisionShape1_Data.color = darkRed;
		collisionShape2_Data.color = lightRed;
	}

	mat4 Pprojection = GetPerspectiveProjection(camera3DViewFrustum);
	mat4 Oprojection = mat4((2.0f / gjkTestCamera.viewArea.x), 0.0f, 0.0f, 0.0f, 0.0f, (2.0f / gjkTestCamera.viewArea.y), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	// Cylindrical Coordinates
	camera3DPosition.x = cameraDistance * sin(DegreesToRadians(cameraAzimuthAngle));
	camera3DPosition.y = cameraHeight;
	camera3DPosition.z = cameraDistance * -cos(DegreesToRadians(cameraAzimuthAngle));

	// Camera Basis
	vec3 cb_z = normalize(-camera3DPosition);
	vec3 cb_x = normalize(cross(cb_z, vec3(0, 1, 0)));
	vec3 cb_y = normalize(cross(cb_x, cb_z));

	//mat4 Ccamera = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1, 0, camera3DPosition.x, camera3DPosition.y, camera3DPosition.z, 1);
	//mat4 cBasis = mat4(cb_x.x, cb_x.y, cb_x.z, 0, cb_y.x, cb_y.y, cb_y.z, 0, cb_z.x, cb_z.y, cb_z.z, 0, 0, 0, 0, 1);

	vec3 cameraPosition_Front = vec3(0, 0, -10);
	vec3 cameraPosition_Right = vec3(10, 0, 0);
	vec3 cameraPosition_Top = vec3(0, 10, 0);

	// World
	mat4 cBasis_Front = mat4(vec4(1, 0, 0, 0), vec4(0, 1, 0, 0), vec4(0, 0, 1, 0), vec4(0, 0, 0, 1));  // x y  z w
	mat4 cBasis_Right = mat4(vec4(0, 0, 1, 0), vec4(0, 1, 0, 0), -vec4(1, 0, 0, 0), vec4(0, 0, 0, 1)); // z y -x w
	mat4 cBasis_Top = mat4(vec4(1, 0, 0, 0), vec4(0, 0, 1, 0), -vec4(0, 1, 0, 0), vec4(0, 0, 0, 1));   // x z -y w

	// CS1
	mat4 cBasis_Front_1 = mat4(normalize(Bmodel_cs1[0]), normalize(Bmodel_cs1[1]), normalize(Bmodel_cs1[2]), vec4(0, 0, 0, 1));
	mat4 cBasis_Right_1 = mat4(normalize(Bmodel_cs1[2]), normalize(Bmodel_cs1[1]), normalize(-Bmodel_cs1[0]), vec4(0, 0, 0, 1));
	mat4 cBasis_Top_1 = mat4(normalize(Bmodel_cs1[0]), normalize(Bmodel_cs1[2]), normalize(-Bmodel_cs1[1]), vec4(0, 0, 0, 1));

	// CS2
	mat4 cBasis_Front_2 = mat4(Bmodel_cs2[0], Bmodel_cs2[1], Bmodel_cs2[2], vec4(0, 0, 0, 1));
	mat4 cBasis_Right_2 = mat4(Bmodel_cs2[2], Bmodel_cs2[1], -Bmodel_cs2[0], vec4(0, 0, 0, 1));
	mat4 cBasis_Top_2 = mat4(Bmodel_cs2[0], Bmodel_cs2[2], -Bmodel_cs2[1], vec4(0, 0, 0, 1));

	//mat4 Ccamera = TranslationMatrix(camera3DPosition) * cBasis * ScaleMatrix(vec3(1, 1, -1));

	vec3 cPosition;
	mat4 cBasis;
	if (gjkTestCameraMode == Orthographic_Front)
	{
		cPosition = cameraPosition_Front;
		cBasis = cBasis_Front;
	}
	else if (gjkTestCameraMode == Orthographic_Right)
	{
		cPosition = cameraPosition_Right;
		cBasis = cBasis_Right;
	}
	else if (gjkTestCameraMode == Orthographic_Top)
	{
		cPosition = cameraPosition_Top;
		cBasis = cBasis_Top;
	}
	else if (gjkTestCameraMode == Orthographic_Front_1)
	{
		cPosition = vec3(0, 0, 0);// cameraPosition_Front;
		cBasis = cBasis_Front_1;
	}
	else if (gjkTestCameraMode == Orthographic_Right_1)
	{
		cPosition = vec3(0, 0, 0);
		cBasis = cBasis_Right_1;
	}
	else if (gjkTestCameraMode == Orthographic_Top_1)
	{
		cPosition = vec3(0, 0, 0); // cameraPosition_Top;
		cBasis = cBasis_Top_1;
	}
	else if (gjkTestCameraMode == Orthographic_Front_2)
	{
		cPosition = vec3(0, 0, 0);
		cBasis = cBasis_Front_2;
	}
	else if (gjkTestCameraMode == Orthographic_Right_2)
	{
		cPosition = vec3(0, 0, 0);
		cBasis = cBasis_Right_2;
	}
	else if (gjkTestCameraMode == Orthographic_Top_2)
	{
		cPosition = vec3(0, 0, 0);
		cBasis = cBasis_Top_2;
	}
	else
	{
		cPosition = cameraPosition_Front;
		cBasis_Front;
	}
	mat4 Ccamera = TranslationMatrix(cPosition) * cBasis * ScaleMatrix(vec3(1, 1, -1));


	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glUseProgram(solidColorTriangleShaderProgram);


	// COLLISION SHAPE 1
	//mat4 Bmodel_cs1 = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, collisionShape1_Data.position.x, collisionShape1_Data.position.y, collisionShape1_Data.position.z, 1) * RotationMatrix_Z(collisionShape1_Data.rotation.z) * RotationMatrix_Y(collisionShape1_Data.rotation.y) * RotationMatrix_X(collisionShape1_Data.rotation.x); //collisionShape1.transform;
	mat4 PCM = Oprojection/*Pprojection*/ * inverse(Ccamera) * Bmodel_cs1;
	glUniformMatrix4fv(solidColorTrianglePCMLocation, 1, GL_FALSE, &PCM[0][0]);
	glUniform4fv(solidColorTriangleTriangleColorLocation, 1, &collisionShape1_Data.color[0]);

	if (collisionShape1_Data.type == cs3D_OrientedBoundingBox)
	{
		glBindVertexArray(cubeVAO);
		glDrawElements(GL_TRIANGLES, numberOfCubeIndices, GL_UNSIGNED_INT, NULL);
	}
	else if (collisionShape1_Data.type == cs3D_Sphere)
	{
		glBindVertexArray(sphereVAO);
		glDrawElements(GL_TRIANGLES, numberOfSphereIndices, GL_UNSIGNED_INT, NULL);
	}
	else if (collisionShape1_Data.type == cs3D_Capsule)
	{
		// Capsule Wall
		glBindVertexArray(capsuleVAO);
		glDrawElements(GL_TRIANGLES, numberOfCapsuleWallIndices, GL_UNSIGNED_INT, NULL);

		// Capsule Sphere 1
		glBindVertexArray(sphereVAO);
		Bmodel_cs1 = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, collisionShape1_Data.position.x, collisionShape1_Data.position.y, collisionShape1_Data.position.z, 1) * RotationMatrix_Z(collisionShape1_Data.rotation.z) * RotationMatrix_Y(collisionShape1_Data.rotation.y) * RotationMatrix_X(collisionShape1_Data.rotation.x) * mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1);
		PCM = Oprojection/*Pprojection*/ * inverse(Ccamera) * Bmodel_cs1;
		glUniformMatrix4fv(solidColorTrianglePCMLocation, 1, GL_FALSE, &PCM[0][0]);
		glDrawElements(GL_TRIANGLES, numberOfSphereIndices, GL_UNSIGNED_INT, NULL);

		// Capsule Sphere 2
		glBindVertexArray(sphereVAO);
		Bmodel_cs1 = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, collisionShape1_Data.position.x, collisionShape1_Data.position.y, collisionShape1_Data.position.z, 1) * RotationMatrix_Z(collisionShape1_Data.rotation.z) * RotationMatrix_Y(collisionShape1_Data.rotation.y) * RotationMatrix_X(collisionShape1_Data.rotation.x) * mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, -1, 0, 1);
		PCM = Oprojection/*Pprojection*/ * inverse(Ccamera) * Bmodel_cs1;
		glUniformMatrix4fv(solidColorTrianglePCMLocation, 1, GL_FALSE, &PCM[0][0]);
		glDrawElements(GL_TRIANGLES, numberOfSphereIndices, GL_UNSIGNED_INT, NULL);
	}
	glBindVertexArray(NULL);



	// COLLISION SHAPE 2
	//mat4 Bmodel_cs2 = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, collisionShape2_Data.position.x, collisionShape2_Data.position.y, collisionShape2_Data.position.z, 1) * RotationMatrix_Z(collisionShape2_Data.rotation.z) * RotationMatrix_Y(collisionShape2_Data.rotation.y) * RotationMatrix_X(collisionShape2_Data.rotation.x); //collisionShape1.transform;
	PCM = Oprojection/*Pprojection*/ * inverse(Ccamera) * Bmodel_cs2;
	glUniformMatrix4fv(solidColorTrianglePCMLocation, 1, GL_FALSE, &PCM[0][0]);
	glUniform4fv(solidColorTriangleTriangleColorLocation, 1, &collisionShape2_Data.color[0]);

	if (collisionShape2_Data.type == cs3D_OrientedBoundingBox)
	{
		glBindVertexArray(cubeVAO);
		glDrawElements(GL_TRIANGLES, numberOfCubeIndices, GL_UNSIGNED_INT, NULL);
	}
	else if (collisionShape2_Data.type == cs3D_Sphere)
	{
		glBindVertexArray(sphereVAO);
		glDrawElements(GL_TRIANGLES, numberOfSphereIndices, GL_UNSIGNED_INT, NULL);
	}
	else if (collisionShape2_Data.type == cs3D_Capsule)
	{
		// Capsule Wall
		glBindVertexArray(capsuleVAO);
		glDrawElements(GL_TRIANGLES, numberOfCapsuleWallIndices, GL_UNSIGNED_INT, NULL);

		// Capsule Sphere 1
		glBindVertexArray(sphereVAO);
		Bmodel_cs1 = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, collisionShape2_Data.position.x, collisionShape2_Data.position.y, collisionShape2_Data.position.z, 1) * RotationMatrix_Z(collisionShape2_Data.rotation.z) * RotationMatrix_Y(collisionShape2_Data.rotation.y) * RotationMatrix_X(collisionShape2_Data.rotation.x) * mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1);
		PCM = Oprojection/*Pprojection*/ * inverse(Ccamera) * Bmodel_cs2;
		glUniformMatrix4fv(solidColorTrianglePCMLocation, 1, GL_FALSE, &PCM[0][0]);
		glDrawElements(GL_TRIANGLES, numberOfSphereIndices, GL_UNSIGNED_INT, NULL);

		// Capsule Sphere 2
		glBindVertexArray(sphereVAO);
		Bmodel_cs1 = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, collisionShape2_Data.position.x, collisionShape2_Data.position.y, collisionShape2_Data.position.z, 1) * RotationMatrix_Z(collisionShape2_Data.rotation.z) * RotationMatrix_Y(collisionShape2_Data.rotation.y) * RotationMatrix_X(collisionShape2_Data.rotation.x) * mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, -1, 0, 1);
		PCM = Oprojection/*Pprojection*/ * inverse(Ccamera) * Bmodel_cs2;
		glUniformMatrix4fv(solidColorTrianglePCMLocation, 1, GL_FALSE, &PCM[0][0]);
		glDrawElements(GL_TRIANGLES, numberOfSphereIndices, GL_UNSIGNED_INT, NULL);
	}
	glBindVertexArray(NULL);

	if (gjkTestCameraMode == Orthographic_Front)
	{
		mat3 PCM;
		vec4 orientationColor = vec4(0, 1, 0, 1);
		glUseProgram(texturedQuadShaderProgram);
		glBindTexture(GL_TEXTURE_2D, frontTexture);
		glBindVertexArray(texturedQuadVAO);
		PCM = mat3(vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1)) * mat3(vec3(.2, 0, 0), vec3(0, .2, 0), vec3(-.9, -.9, 1));
		glUniformMatrix3fv(solidColorQuadPCMLocation, 1, GL_FALSE, &PCM[0][0]);
		glUniform4fv(solidColorQuadQuadColorLocation, 1, &orientationColor[0]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices);
	}
	else if (gjkTestCameraMode == Orthographic_Right)
	{
		mat3 PCM;
		vec4 orientationColor = vec4(0, 1, 0, 1);
		glUseProgram(texturedQuadShaderProgram);
		glBindTexture(GL_TEXTURE_2D, rightTexture);
		glBindVertexArray(texturedQuadVAO);
		PCM = mat3(vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1)) * mat3(vec3(.2, 0, 0), vec3(0, .2, 0), vec3(-.9, -.9, 1));
		glUniformMatrix3fv(solidColorQuadPCMLocation, 1, GL_FALSE, &PCM[0][0]);
		glUniform4fv(solidColorQuadQuadColorLocation, 1, &orientationColor[0]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices);
	}
	else if (gjkTestCameraMode == Orthographic_Top)
	{
		mat3 PCM;
		vec4 orientationColor = vec4(0, 1, 0, 1);
		glUseProgram(texturedQuadShaderProgram);
		glBindTexture(GL_TEXTURE_2D, topTexture);
		glBindVertexArray(texturedQuadVAO);
		PCM = mat3(vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1)) * mat3(vec3(.2, 0, 0), vec3(0, .2, 0), vec3(-.9, -.9, 1));
		glUniformMatrix3fv(solidColorQuadPCMLocation, 1, GL_FALSE, &PCM[0][0]);
		glUniform4fv(solidColorQuadQuadColorLocation, 1, &orientationColor[0]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices);
	}
	else if (gjkTestCameraMode == Orthographic_Front_1)
	{
		mat3 PCM;
		vec4 orientationColor = vec4(0, 1, 0, 1);
		glUseProgram(texturedQuadShaderProgram);
		glBindTexture(GL_TEXTURE_2D, front_1_Texture);
		glBindVertexArray(texturedQuadVAO);
		PCM = mat3(vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1)) * mat3(vec3(.2, 0, 0), vec3(0, .2, 0), vec3(-.9, -.9, 1));
		glUniformMatrix3fv(solidColorQuadPCMLocation, 1, GL_FALSE, &PCM[0][0]);
		glUniform4fv(solidColorQuadQuadColorLocation, 1, &orientationColor[0]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices);
	}
	else if (gjkTestCameraMode == Orthographic_Right_1)
	{
		mat3 PCM;
		vec4 orientationColor = vec4(0, 1, 0, 1);
		glUseProgram(texturedQuadShaderProgram);
		glBindTexture(GL_TEXTURE_2D, right_1_Texture);
		glBindVertexArray(texturedQuadVAO);
		PCM = mat3(vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1)) * mat3(vec3(.2, 0, 0), vec3(0, .2, 0), vec3(-.9, -.9, 1));
		glUniformMatrix3fv(solidColorQuadPCMLocation, 1, GL_FALSE, &PCM[0][0]);
		glUniform4fv(solidColorQuadQuadColorLocation, 1, &orientationColor[0]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices);
	}
	else if (gjkTestCameraMode == Orthographic_Top_1)
	{
		mat3 PCM;
		vec4 orientationColor = vec4(0, 1, 0, 1);
		glUseProgram(texturedQuadShaderProgram);
		glBindTexture(GL_TEXTURE_2D, top_1_Texture);
		glBindVertexArray(texturedQuadVAO);
		PCM = mat3(vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1)) * mat3(vec3(.2, 0, 0), vec3(0, .2, 0), vec3(-.9, -.9, 1));
		glUniformMatrix3fv(solidColorQuadPCMLocation, 1, GL_FALSE, &PCM[0][0]);
		glUniform4fv(solidColorQuadQuadColorLocation, 1, &orientationColor[0]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices);
	}
	else if (gjkTestCameraMode == Orthographic_Front_2)
	{
		mat3 PCM;
		vec4 orientationColor = vec4(0, 1, 0, 1);
		glUseProgram(texturedQuadShaderProgram);
		glBindTexture(GL_TEXTURE_2D, front_2_Texture);
		glBindVertexArray(texturedQuadVAO);
		PCM = mat3(vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1)) * mat3(vec3(.2, 0, 0), vec3(0, .2, 0), vec3(-.9, -.9, 1));
		glUniformMatrix3fv(solidColorQuadPCMLocation, 1, GL_FALSE, &PCM[0][0]);
		glUniform4fv(solidColorQuadQuadColorLocation, 1, &orientationColor[0]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices);
	}
	else if (gjkTestCameraMode == Orthographic_Right_2)
	{
		mat3 PCM;
		vec4 orientationColor = vec4(0, 1, 0, 1);
		glUseProgram(texturedQuadShaderProgram);
		glBindTexture(GL_TEXTURE_2D, right_2_Texture);
		glBindVertexArray(texturedQuadVAO);
		PCM = mat3(vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1)) * mat3(vec3(.2, 0, 0), vec3(0, .2, 0), vec3(-.9, -.9, 1));
		glUniformMatrix3fv(solidColorQuadPCMLocation, 1, GL_FALSE, &PCM[0][0]);
		glUniform4fv(solidColorQuadQuadColorLocation, 1, &orientationColor[0]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices);
	}
	else if (gjkTestCameraMode == Orthographic_Top_2)
	{
		mat3 PCM;
		vec4 orientationColor = vec4(0, 1, 0, 1);
		glUseProgram(texturedQuadShaderProgram);
		glBindTexture(GL_TEXTURE_2D, top_2_Texture);
		glBindVertexArray(texturedQuadVAO);
		PCM = mat3(vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1)) * mat3(vec3(.2, 0, 0), vec3(0, .2, 0), vec3(-.9, -.9, 1));
		glUniformMatrix3fv(solidColorQuadPCMLocation, 1, GL_FALSE, &PCM[0][0]);
		glUniform4fv(solidColorQuadQuadColorLocation, 1, &orientationColor[0]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices);
	}
}

inline void ShutdownCollisionDetection3DApplet()
{
}