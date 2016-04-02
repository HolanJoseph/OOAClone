#include "glew/GL/glew.h"
#include <gl/GL.h>

#include "Types.h"
#include "Math.h"
#include "BitManip.h"
#include "Util.h"
#include "RandomNumberGenerator.h"

#include "GameAPI.h"
#include "DebugAPI.h"
#include "InputAPI.h"
#include "FileAPI.h"
#include "StringAPI.h"

#include "AssetLoading.h"
#include "CollisionDetection.h"
#include "CollisionDetection2D.h"

#include "CollisionDetection_Tests.h"

#include "CollisionDetection_Visualization.h"

#include "CollisionDetection2D_Applet.h"

//#define COLLISION2DOLD 1
//#define COLLISION3D 1
#define COLLISION2DAPPLET 1

struct verifyShaderReturnResult
{
	bool compiled;
	char* infoLog;
	I64 infoLogLength;

};
verifyShaderReturnResult verifyShader(GLuint shader);

struct verifyProgramReturnResult
{
	bool compiled;
	char* infoLog;
	I64 infoLogLength;

};
verifyProgramReturnResult verifyProgram(GLuint program);






const GLuint numVertices = 4;
const GLuint vertexDimensionality = 2;
const GLuint textureSpaceDimensionality = 2;
GLuint texturedQuadVAO;


// Textured Quad Shader Things
GLuint texturedQuadShaderProgram;

GLuint spriteSamplerLocation;
GLuint PCMLocation;

GLuint texture;
GLuint textureSampler;

// Solid Color Quad Shader Things
GLuint solidColorQuadShaderProgram;
GLuint solidColorQuadPCMLocation;
GLuint solidColorQuadQuadColorLocation;

GLuint solidColorCircleInPointShaderProgram;
GLuint solidColorCircleInPointPCMLocation;
GLuint solidColorCircleInPointCircleColorLocation;

GLuint solidColorTriangleShaderProgram;
GLuint solidColorTrianglePCMLocation;
GLuint solidColorTriangleTriangleColorLocation;



struct Entity
{
	vec2 position;
	vec2 scale;
	F32 rotationAngle;
	GLuint texture;
};

struct Camera
{
	vec2 position;
	vec2 viewArea;
};

U32 numCollisionEntities = 2;
Rectangle cs2Rectangle;
Rectangle cs1Rectangle;
Triangle  cs1Triangle;
Circle    cs1Circle;
Camera collisionCamera;

GLuint gridVAO;
#define numGridLines 10
#define gridLinePointDimensionality 2
#define pointsPerLine 2

GLuint equalateralTriangleVAO;
#define numPointsInTriangle 3
#define equalateralTrianglePointDimensionality 2

GLuint circleVAO;
#define numPointsInCircle 1
#define circlePointDimensionality 2

void Init2DCollisionTestScene()
{
	collisionCamera.position = vec2(0,0);
	collisionCamera.viewArea = vec2(10,10);

	cs2Rectangle.origin = vec2(0,0);
	cs2Rectangle.halfDim = vec2(.5f, .5f);
	cs2Rectangle.transform = mat4(1,0,0,0,   0,1,0,0,   0,0,1,0,   .5f,.5f,0,1);

	cs1Rectangle.origin = vec2(0, 0);
	cs1Rectangle.halfDim = vec2(.5f, .5f);

	// Grid
	glGenVertexArrays(1, &gridVAO);
	glBindVertexArray(gridVAO);
	GLfloat gridPositions[(numGridLines + numGridLines + 2) * pointsPerLine * gridLinePointDimensionality] = {
		// varying y values
		-numGridLines / 2.0f, (-numGridLines / 2.0f) + 0.0f, numGridLines / 2.0f, (-numGridLines / 2.0f) + 0.0f,
		-numGridLines / 2.0f, (-numGridLines / 2.0f) + 1.0f, numGridLines / 2.0f, (-numGridLines / 2.0f) + 1.0f,
		-numGridLines / 2.0f, (-numGridLines / 2.0f) + 2.0f, numGridLines / 2.0f, (-numGridLines / 2.0f) + 2.0f,
		-numGridLines / 2.0f, (-numGridLines / 2.0f) + 3.0f, numGridLines / 2.0f, (-numGridLines / 2.0f) + 3.0f,
		-numGridLines / 2.0f, (-numGridLines / 2.0f) + 4.0f, numGridLines / 2.0f, (-numGridLines / 2.0f) + 4.0f,
		-numGridLines / 2.0f, (-numGridLines / 2.0f) + 5.0f, numGridLines / 2.0f, (-numGridLines / 2.0f) + 5.0f,
		-numGridLines / 2.0f, (-numGridLines / 2.0f) + 6.0f, numGridLines / 2.0f, (-numGridLines / 2.0f) + 6.0f,
		-numGridLines / 2.0f, (-numGridLines / 2.0f) + 7.0f, numGridLines / 2.0f, (-numGridLines / 2.0f) + 7.0f,
		-numGridLines / 2.0f, (-numGridLines / 2.0f) + 8.0f, numGridLines / 2.0f, (-numGridLines / 2.0f) + 8.0f,
		-numGridLines / 2.0f, (-numGridLines / 2.0f) + 9.0f, numGridLines / 2.0f, (-numGridLines / 2.0f) + 9.0f,
		-numGridLines / 2.0f, (-numGridLines / 2.0f) + 10.0f, numGridLines / 2.0f, (-numGridLines / 2.0f) + 10.0f,

		// varying x values
		(-numGridLines / 2.0f) + 0.0f, -numGridLines / 2.0f, (-numGridLines / 2.0f) + 0.0f, numGridLines / 2.0f,
		(-numGridLines / 2.0f) + 1.0f, -numGridLines / 2.0f, (-numGridLines / 2.0f) + 1.0f, numGridLines / 2.0f,
		(-numGridLines / 2.0f) + 2.0f, -numGridLines / 2.0f, (-numGridLines / 2.0f) + 2.0f, numGridLines / 2.0f,
		(-numGridLines / 2.0f) + 3.0f, -numGridLines / 2.0f, (-numGridLines / 2.0f) + 3.0f, numGridLines / 2.0f,
		(-numGridLines / 2.0f) + 4.0f, -numGridLines / 2.0f, (-numGridLines / 2.0f) + 4.0f, numGridLines / 2.0f,
		(-numGridLines / 2.0f) + 5.0f, -numGridLines / 2.0f, (-numGridLines / 2.0f) + 5.0f, numGridLines / 2.0f,
		(-numGridLines / 2.0f) + 6.0f, -numGridLines / 2.0f, (-numGridLines / 2.0f) + 6.0f, numGridLines / 2.0f,
		(-numGridLines / 2.0f) + 7.0f, -numGridLines / 2.0f, (-numGridLines / 2.0f) + 7.0f, numGridLines / 2.0f,
		(-numGridLines / 2.0f) + 8.0f, -numGridLines / 2.0f, (-numGridLines / 2.0f) + 8.0f, numGridLines / 2.0f,
		(-numGridLines / 2.0f) + 9.0f, -numGridLines / 2.0f, (-numGridLines / 2.0f) + 9.0f, numGridLines / 2.0f,
		(-numGridLines / 2.0f) + 10.0f, -numGridLines / 2.0f, (-numGridLines / 2.0f) + 10.0f, numGridLines / 2.0f,
	};
	GLuint gridVertexBuffer;
	glGenBuffers(1, &gridVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, gridVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gridPositions), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(gridPositions), gridPositions);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);


	// Equilateral triangle
	glGenVertexArrays(1, &equalateralTriangleVAO);
	glBindVertexArray(equalateralTriangleVAO);

	GLfloat equalateralTrianglePoints[numPointsInTriangle * equalateralTrianglePointDimensionality] = {
		-0.6f, -0.3f,
		 0.6f, -0.3f,
		 0.0f,  0.6f
	};
	GLuint equalateralTriangleVertexBuffer;
	glGenBuffers(1, &equalateralTriangleVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, equalateralTriangleVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(equalateralTrianglePoints), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(equalateralTrianglePoints), equalateralTrianglePoints);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	cs1Triangle.origin = vec3(0, 0, 0);
	cs1Triangle.points[0] = vec3(equalateralTrianglePoints[0], equalateralTrianglePoints[1], 0);
	cs1Triangle.points[1] = vec3(equalateralTrianglePoints[2], equalateralTrianglePoints[3], 0);
	cs1Triangle.points[2] = vec3(equalateralTrianglePoints[4], equalateralTrianglePoints[5], 0);

	// Circle
	glGenVertexArrays(1, &circleVAO);
	glBindVertexArray(circleVAO);

	GLfloat circleOrigin[2] = {0.0f, 0.0f};
	GLuint circleVertexBuffer;
	glGenBuffers(1, &circleVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, circleVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(circleOrigin), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(circleOrigin), circleOrigin);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	cs1Circle.origin = vec3(0, 0, 0);
	cs1Circle.radius = 0.5f;
}






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

	result.right = near * tan(DegreesToRadians(xFOV)/2.0f);
	result.left = -result.right;

	result.top = near * tan(DegreesToRadians(yFOV)/2.0f);
	result.bottom = -result.top;

	return result;
}

mat4 GetPerspectiveProjection(ViewFrustum frustum)
{
	mat4 result;

	result[0] = vec4((2.0f*frustum.near)/(frustum.right-frustum.left), 0.0f, 0.0f, 0.0f);
	result[1] = vec4(0.0f, (2.0f*frustum.near)/(frustum.top-frustum.bottom), 0.0f, 0.0f);
	result[2] = vec4((frustum.right+frustum.left)/(frustum.right-frustum.left), (frustum.top+frustum.bottom)/(frustum.top-frustum.bottom), -(frustum.far+frustum.near)/(frustum.far-frustum.near), -1.0f);
	result[3] = vec4(0.0f, 0.0f, -(2.0f*frustum.far*frustum.near)/(frustum.far-frustum.near), 0.0f);

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

void Init3DCollisionTestScene()
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
	camera3DViewFrustum = CreateViewFrustum(2*24.866f, 2*24.866f, 1.0f / 3.0f, 100.0f);



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
	collisionShape1_capsule.points[0] = vec3(0,1,0);
	collisionShape1_capsule.points[1] = vec3(0, -1, 0);
	collisionShape1_capsule.radius = 1;

	collisionShape2_obb.halfDim = vec3(1, 1, 1);
	collisionShape2_sphere.radius = 1;
	collisionShape2_capsule.points[0] = vec3(0, 1, 0);
	collisionShape2_capsule.points[1] = vec3(0, -1, 0);
	collisionShape2_capsule.radius = 1;

	/*mat4 Pp = GetPerspectiveProjection(camera3DViewFrustum);

	vec4 csNearZero = Pp * vec4(0.0f, 0.0f, -camera3DViewFrustum.near, 1.0f); csNearZero /= csNearZero.w;
	vec4 csNearMin = Pp * vec4(camera3DViewFrustum.left, camera3DViewFrustum.bottom, -camera3DViewFrustum.near, 1.0f); csNearMin /= csNearMin.w;
	vec4 csNearMax = Pp * vec4(camera3DViewFrustum.right, camera3DViewFrustum.top, -camera3DViewFrustum.near, 1.0f); csNearMax /= csNearMax.w;

	vec4 csFarZero = Pp * vec4(0.0f, 0.0f, -camera3DViewFrustum.far, 1.0f); csFarZero /= csFarZero.w;
	vec4 csFarMin = Pp * vec4(-camera3DViewFrustum.far*tan(DegreesToRadians(55.64) / 2.0f), -camera3DViewFrustum.far*tan(DegreesToRadians(33.07f) / 2.0f), -camera3DViewFrustum.far, 1.0f); csFarMin /= csFarMin.w;
	vec4 csFarMax = Pp * vec4(camera3DViewFrustum.far*tan(DegreesToRadians(55.64) / 2.0f), camera3DViewFrustum.far*tan(DegreesToRadians(33.07f) / 2.0f), -camera3DViewFrustum.far, 1.0f); csFarMax /= csFarMax.w;

	collisionShape1.origin = vec3(0,0,0);
	collisionShape1.halfDim = vec3(1,1,1);
	collisionShape1.transform = mat4(1,0,0,0,   0,1,0,0,   0,0,1,0, 0,0,1,1);

	collisionShape2.origin = vec3(0,0,0);
	collisionShape2.halfDim = vec3(1,1,1);
	collisionShape2.transform = mat4(1,0,0,0,   0,1,0,0,   0,0,1,0,   -1,-1,1,1);

	OrientedBoundingBoxPoints collisionShape1WorldSpacePoints = GetOBBPoints(collisionShape1);

	mat4 Ccamera = inverse(mat4(1,0,0,0,   0,1,0,0,   0,0,-1,0,   0,0,-5,1));
	OrientedBoundingBoxPoints collisionShape1CameraSpacePoints = Ccamera * collisionShape1WorldSpacePoints;

	ViewFrustum cvf = CreateViewFrustum(55.64f, 33.07f, 1.0f, 100.0f);
	mat4 Pproj = GetPerspectiveProjection(cvf);
	OrientedBoundingBoxPoints collisionShape1ClipSpacePoints = Homogenize(Pproj * collisionShape1CameraSpacePoints);

	int x = 5;*/

	/*vec3 collisionShape1_Rotation = vec3(0, 0, DegreesToRadians(45.0f));
	mat4 Pprojection = GetPerspectiveProjection(camera3DViewFrustum);
	mat4 Ccamera = mat4(cos(collisionShape1_Rotation.z), sin(collisionShape1_Rotation.z), 0, 0, -sin(collisionShape1_Rotation.z), cos(collisionShape1_Rotation.z), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1) * mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1, 0, camera3DPosition.x, camera3DPosition.y, camera3DPosition.z, 1);
	mat4 Bmodel = collisionShape1.transform;
	mat4 PCM = Pprojection * inverse(Ccamera) * Bmodel;

	OrientedBoundingBoxPoints wsp;
	for (U32 i = 0; i < 8; ++i)
	{
		wsp.points[i] = PCM * vec4(cubeVertices[(i * 3) + 0], cubeVertices[(i * 3) + 1], cubeVertices[(i * 3) + 2], 1.0f);
	}
	OrientedBoundingBoxPoints csp = Homogenize(wsp);*/
}






U32 numEntities = (10 * 9) + 1;
Entity* entities;
Camera  camera;
U32 linkEntityLocation = numEntities - 1;

void InitScene()
{
	camera.position = vec2(35, -12.5f);
	camera.viewArea.x = 10;
	camera.viewArea.y = 9;

	entities = (Entity*)malloc(sizeof(Entity) * numEntities);

	char* tileFilenames[] = {
		// System Bar
		"Assets/x60/sysBar.bmp",
		"Assets/x60/sysBar.bmp",
		"Assets/x60/sysBar.bmp",
		"Assets/x60/sysBar.bmp",
		"Assets/x60/sysBar.bmp",
		"Assets/x60/sysBar.bmp",
		"Assets/x60/sysBar.bmp",
		"Assets/x60/sysBar.bmp",
		"Assets/x60/sysBar.bmp",
		"Assets/x60/sysBar.bmp",

		// row 1
		"Assets/x60/treeLowRight.bmp",
		"Assets/x60/treeLowLeft.bmp",
		"Assets/x60/treeLowRight.bmp",
		"Assets/x60/yellow.bmp",
		"Assets/x60/yellow.bmp",
		"Assets/x60/greenFringeBL.bmp",
		"Assets/x60/green.bmp",
		"Assets/x60/greenFringeR.bmp",
		"Assets/x60/treeLowLeft.bmp",
		"Assets/x60/treeLowRight.bmp",

		// row 2
		"Assets/x60/treeHighRight.bmp",
		"Assets/x60/greenFringeUL.bmp",
		"Assets/x60/greenFringeU.bmp",
		"Assets/x60/greenFringeUR.bmp",
		"Assets/x60/yellow.bmp",
		"Assets/x60/yellow.bmp",
		"Assets/x60/greenFringeL.bmp",
		"Assets/x60/greenFringeR.bmp",
		"Assets/x60/treeHighLeft.bmp",
		"Assets/x60/treeHighRight.bmp",

		// row 3
		"Assets/x60/treeLowRight.bmp",
		"Assets/x60/greenFringeL.bmp",
		"Assets/x60/greenFlowers.bmp",
		"Assets/x60/greenFringeR.bmp",
		"Assets/x60/yellow.bmp",
		"Assets/x60/greenFringeUL.bmp",
		"Assets/x60/greenWeeds.bmp",
		"Assets/x60/greenFringeBR.bmp",
		"Assets/x60/treeLowLeft.bmp",
		"Assets/x60/treeLowRight.bmp",
		
		// row 4
		"Assets/x60/treeHighRight.bmp",
		"Assets/x60/greenFringeBL.bmp",
		"Assets/x60/greenFringeB.bmp",
		"Assets/x60/greenWeeds.bmp",
		"Assets/x60/greenFringeU.bmp",
		"Assets/x60/green.bmp",
		"Assets/x60/greenFringeBR.bmp",
		"Assets/x60/yellow.bmp",
		"Assets/x60/treeHighLeft.bmp",
		"Assets/x60/treeHighRight.bmp",
		
		// row 5
		"Assets/x60/treeLowRight.bmp",
		"Assets/x60/weed.bmp",
		"Assets/x60/weed.bmp",
		"Assets/x60/greenFringeL.bmp",
		"Assets/x60/green.bmp",
		"Assets/x60/greenFringeR.bmp",
		"Assets/x60/yellow.bmp",
		"Assets/x60/weed.bmp",
		"Assets/x60/treeLowLeft.bmp",
		"Assets/x60/treeLowRight.bmp",

		// row 6
		"Assets/x60/treeHighRight.bmp",
		"Assets/x60/weed.bmp",
		"Assets/x60/greenFringeUL.bmp",
		"Assets/x60/green.bmp",
		"Assets/x60/greenFlowers.bmp",
		"Assets/x60/greenFringeR.bmp",
		"Assets/x60/weed.bmp",
		"Assets/x60/weed.bmp",
		"Assets/x60/treeHighLeft.bmp",
		"Assets/x60/treeHighRight.bmp",

		// row 7
		"Assets/x60/treeLowRight.bmp",
		"Assets/x60/yellow.bmp",
		"Assets/x60/greenFringeBL.bmp",
		"Assets/x60/greenFringeB.bmp",
		"Assets/x60/greenFringeB.bmp",
		"Assets/x60/greenFringeBR.bmp",
		"Assets/x60/weed.bmp",
		"Assets/x60/weed.bmp",
		"Assets/x60/treeLowLeft.bmp",
		"Assets/x60/treeLowRight.bmp",

		// row 8
		"Assets/x60/rails.bmp",
		"Assets/x60/rails.bmp",
		"Assets/x60/rails.bmp",
		"Assets/x60/rails.bmp",
		"Assets/x60/rails.bmp",
		"Assets/x60/rails.bmp",
		"Assets/x60/rails.bmp",
		"Assets/x60/rails.bmp",
		"Assets/x60/rails.bmp",
		"Assets/x60/rails.bmp",
	};

	for (U32 j = 0; j < 9; ++j)
	{
		for (U32 i = 0; i < 10; ++i)
		{
			U32 entityLocation = 10 * j + i;

			vec2 pos(30.5f + i, -8.5f - j);
			char* filename = tileFilenames[entityLocation];

			entities[entityLocation].position = pos;
			entities[entityLocation].scale = vec2(1.0f, 1.0f);

			I32 textureWidth = 0;
			I32 textureHeight = 0;
			I32 textureImageComponents = 0;
			I32 textureNumImageComponentsDesired = 4;
			U8 * textureData = LoadTexture(filename, &textureWidth, &textureHeight, &textureImageComponents, textureNumImageComponentsDesired); // Channel order?

			glActiveTexture(GL_TEXTURE0);
			glGenTextures(1, &(entities[entityLocation].texture));
			glBindTexture(GL_TEXTURE_2D, entities[entityLocation].texture);
			glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, textureWidth, textureHeight);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
		}
	}

	
	entities[linkEntityLocation].position = camera.position;
	entities[linkEntityLocation].scale = vec2(0.75f, 1.0f);

	I32 textureWidth = 0;
	I32 textureHeight = 0;
	I32 textureImageComponents = 0;
	I32 textureNumImageComponentsDesired = 4;
	U8 * textureData = LoadTexture("Assets/x60/link.bmp", &textureWidth, &textureHeight, &textureImageComponents, textureNumImageComponentsDesired); // Channel order?

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &(entities[linkEntityLocation].texture));
	glBindTexture(GL_TEXTURE_2D, entities[linkEntityLocation].texture);
	glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, textureWidth, textureHeight);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, textureData);

}

GLuint LoadShaderProgram(char* vertexShaderFilename, char* fragmentShaderFilename)
{
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	U64 vertexShaderFileSize = GetFileSize(vertexShaderFilename).fileSize;
	char* vertexShaderSource = (char *)malloc(sizeof(char)* vertexShaderFileSize);
	const GLint glSizeRead = ReadFile(vertexShaderFilename, vertexShaderSource, vertexShaderFileSize, 0).numberOfBytesRead;
	glShaderSource(vertexShader, 1, &vertexShaderSource, &glSizeRead);
	glCompileShader(vertexShader);
	verifyShaderReturnResult vertexVerification = verifyShader(vertexShader);
	if (!vertexVerification.compiled)
	{
		DebugPrint(vertexVerification.infoLog);
		return false;
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	U64 fragmentShaderFileSize = GetFileSize(fragmentShaderFilename).fileSize;
	char* fragmentShaderSource = (char *)malloc(sizeof(char)* fragmentShaderFileSize);
	const GLint glFragmentShaderSize = ReadFile(fragmentShaderFilename, fragmentShaderSource, fragmentShaderFileSize, 0).numberOfBytesRead;
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, &glFragmentShaderSize);
	glCompileShader(fragmentShader);
	verifyShaderReturnResult fragmentVerification = verifyShader(fragmentShader);
	if (!fragmentVerification.compiled)
	{
		DebugPrint(fragmentVerification.infoLog);
		return false;
	}

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	verifyProgramReturnResult programVerification = verifyProgram(shaderProgram);
	if (!programVerification.compiled)
	{
		DebugPrint(programVerification.infoLog);
		return false;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

bool GameInit()
{
	CollisionTestsRectRect2();

	glClearColor(0.32f, 0.18f, 0.66f, 0.0f);

	glGenVertexArrays(1, &texturedQuadVAO);
	glBindVertexArray(texturedQuadVAO);
	// NOTE: quad for testing, corrected for aspect ratio
	GLfloat vertices[numVertices * vertexDimensionality] =
	{
		-1.0f, -1.0f,
		 1.0f, -1.0f,
		 1.0f,  1.0f,
		-1.0f,  1.0f
	};

	GLfloat textureCoordinates[numVertices * textureSpaceDimensionality] =
	{
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};
	GLuint Buffers[1];
	glGenBuffers(1, Buffers);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(textureCoordinates), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(textureCoordinates), textureCoordinates);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)sizeof(vertices) /* NOTE: THIS IS A BYTE OFFSET*/);
	glEnableVertexAttribArray(1);

	texturedQuadShaderProgram = LoadShaderProgram("texturedQuad.vert", "texturedQuad.frag");
	solidColorQuadShaderProgram = LoadShaderProgram("solidColorQuad.vert", "solidColorQuad.frag");
	solidColorCircleInPointShaderProgram = LoadShaderProgram("solidColorCircleInPoint.vert", "solidColorCircleInPoint.frag");
	solidColorTriangleShaderProgram = LoadShaderProgram("solidColorTriangle.vert", "solidColorTriangle.frag");
	glUseProgram(texturedQuadShaderProgram);


	// textures
	// read in the texture
	I32 textureWidth = 0;
	I32 textureHeight = 0;
	I32 textureImageComponents = 0;
	I32 textureNumImageComponentsDesired = 4;
	U8 * textureData = LoadTexture("Assets/tile1.bmp", &textureWidth, &textureHeight, &textureImageComponents, textureNumImageComponentsDesired); // Channel order?
	
	// send the texture data to OpenGL memory
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, textureWidth, textureHeight);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, textureData);

	glGenSamplers(1, &textureSampler);
	glBindSampler(0, textureSampler);
	glSamplerParameteri(textureSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(textureSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(textureSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glSamplerParameteri(textureSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Set the spriteSampler sampler variable in the shader to fetch data from the 0th texture location
	spriteSamplerLocation = glGetUniformLocation(texturedQuadShaderProgram, "spriteSampler");
	glUniform1i(spriteSamplerLocation, 0);
	PCMLocation = glGetUniformLocation(texturedQuadShaderProgram, "PCM");

	solidColorQuadPCMLocation = glGetUniformLocation(solidColorQuadShaderProgram, "PCM");
	solidColorQuadQuadColorLocation = glGetUniformLocation(solidColorQuadShaderProgram, "quadColor");

	solidColorCircleInPointPCMLocation = glGetUniformLocation(solidColorCircleInPointShaderProgram, "PCM");
	solidColorCircleInPointCircleColorLocation = glGetUniformLocation(solidColorCircleInPointShaderProgram, "circleColor");

	solidColorTrianglePCMLocation = glGetUniformLocation(solidColorTriangleShaderProgram, "PCM");
	solidColorTriangleTriangleColorLocation = glGetUniformLocation(solidColorTriangleShaderProgram, "triangleColor");
	//InitScene();
#ifdef COLLISION2DOLD
	Init2DCollisionTestScene();
#elif COLLISION3D
	Init3DCollisionTestScene();
#elif COLLISION2DAPPLET
	Init2DCollisionTestScene();
	InitializeCollisionDetection2DApplet();
#endif

	return true;
}


vec4 darkRed = vec4(0.898f, 0.224f, 0.208f, 1.0f);
vec4 lightRed = vec4(0.957f, 0.263f, 0.212f, 1.0f);

vec4 darkGreen = vec4(0.263f, 0.627f, 0.278f, 1.0f);
vec4 lightGreen = vec4(0.298f, 0.686f, 0.314f, 1.0f);

enum csType
{
	cs_Rectangle,
	cs_Triangle,
	cs_Circle,

	cs_COUNT
};
csType cs1Type = cs_Rectangle;
vec2 cs1Pos = vec2(-1,2);
F32 cs1RotationAngle = 0;
vec4 cs1Color;

vec4 cs2Color;

U32 sampleNumber = 0;
bool detailsMode = true;
bool showcs2 = false;
bool smoothMode = true;

enum GJKMode
{
	gjk_Simple,
	//gjk_Casey,
	gjk_AllVoronoi,

	gjk_COUNT
};
GJKMode gjkMode;

void collisionScene2DUpdate(F32 deltaTime)
{
// 	if (GetKey(KeyCode_W))
// 	{
// 		entities[linkEntityLocation].position += vec2(0.0f, 2*deltaTime);
// 	}
// 	if (GetKey(KeyCode_S))
// 	{
// 		entities[linkEntityLocation].position += vec2(0.0f, 2 * -deltaTime);
// 	}
// 	if (GetKey(KeyCode_A))
// 	{
// 		entities[linkEntityLocation].position += vec2(2 * -deltaTime, 0.0f);
// 	}
// 	if (GetKey(KeyCode_D))
// 	{
// 		entities[linkEntityLocation].position += vec2(2 * deltaTime, 0.0f);
// 	}
// 
// 	camera.position = entities[linkEntityLocation].position;


	// NOTE: Setting the viewport each frame shouldnt happen
	//glViewport(0, 0, 600, 540);
	glViewport(0, 0, 800, 800);

	// NOTE: Clear the buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);




	// NOTE: Draw grid
	glUseProgram(solidColorQuadShaderProgram);
	glBindVertexArray(gridVAO);
	mat3 Ccamera = mat3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, collisionCamera.position.x, collisionCamera.position.y, 1.0f);
	mat3 Oprojection = mat3((2.0f / collisionCamera.viewArea.x), 0.0f, 0.0f, 0.0f, (2.0f / collisionCamera.viewArea.y), 0.0f, 0.0f, 0.0f, 1.0f);
	mat3 PCM = Oprojection * inverse(Ccamera) * mat3(1, 0, 0, 0, 1, 0, 0, 0, 1);
	glUniformMatrix3fv(solidColorQuadPCMLocation, 1, GL_FALSE, &PCM[0][0]);
	glUniform4fv(solidColorQuadQuadColorLocation, 1, &vec4(.933, .933, .933, 1)[0]);
	glDrawArrays(GL_LINES, 0, (numGridLines + numGridLines + 2) * pointsPerLine * gridLinePointDimensionality);



	// NOTE: Bind the VAO that holds the vertex information for the current object.
	glBindVertexArray(texturedQuadVAO);

	// NOTE: Bind the shader that will be used to draw it.
	//glUseProgram(texturedQuadShaderProgram);
	//glUseProgram(solidColorQuadShaderProgram);

	// NOTE: Bind the texture that represents the gameobject, also make sure the texture is active.
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, texture);

	// NOTE: Draw this bitch.
	//glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices);

// 	for (U32 i = 0; i < numEntities; ++i)
// 	{
// 		glActiveTexture(GL_TEXTURE0);
// 		glBindTexture(GL_TEXTURE_2D, entities[i].texture);
// 		
// 		mat3 Bmodel = mat3(1.0f*entities[i].scale.x, 0.0f, 0.0f, 0.0f, 1.0f*entities[i].scale.y, 0.0f, entities[i].position.x, entities[i].position.y, 1.0f);
// 		mat3 Ccamera = mat3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, camera.position.x, camera.position.y, 1.0f);
// 		mat3 Oprojection = mat3((2.0f / camera.viewArea.x), 0.0f, 0.0f, 0.0f, (2.0f / camera.viewArea.y), 0.0f, 0.0f, 0.0f, 1.0f);
// 		mat3 PCM = Oprojection * inverse(Ccamera) * Bmodel;
// 		glUniformMatrix3fv(PCMLocation, 1, GL_FALSE, &PCM[0][0]);
// 
// 		glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices);
// 	}

	if (GetMouseButtonDown(MouseCode_Right))
	{
		smoothMode = !smoothMode;
	}

	F32 angle = DegreesToRadians(45.0f);
 	if (GetKey(KeyCode_W))
 	{
		if (smoothMode)
		{
			cs1Pos.y += deltaTime * .5f;
		}
		else if (GetKeyDown(KeyCode_W))
		{
			cs1Pos.y += 0.5f;
		}
 	}
	if (GetKey(KeyCode_S))
 	{
		if (smoothMode)
		{
			cs1Pos.y -= deltaTime * .5f;
		}
		else if (GetKeyDown(KeyCode_S))
		{
			cs1Pos.y -= 0.5f;
		}
 	}
	if (GetKey(KeyCode_A))
 	{
		if (smoothMode)
		{
			cs1Pos.x -= deltaTime * .5f;
		}
		else if(GetKeyDown(KeyCode_A))
		{
			cs1Pos.x -= 0.5f;
		}
 	}
	if (GetKey(KeyCode_D))
 	{
		if (smoothMode)
		{
			cs1Pos.x += deltaTime * .5f;
		}
		else if (GetKeyDown(KeyCode_D))
		{
			cs1Pos.x += 0.5f;
		}
 	}
	if (GetKey(KeyCode_K))
	{
		if (smoothMode)
		{
			cs1RotationAngle += deltaTime * angle;
		}
		else if (GetKeyDown(KeyCode_K))
		{
			cs1RotationAngle += angle;
		}
	}
	if (GetKey(KeyCode_L))
	{
		if (smoothMode)
		{
			cs1RotationAngle -= deltaTime * angle;
		}
		else if (GetKeyDown(KeyCode_L))
		{
			cs1RotationAngle -= angle;
		}
	}
	if (GetMouseButtonDown(MouseCode_Left))
	{
		cs1Type = (csType)(cs1Type + 1);
		if (cs1Type == cs_COUNT)
		{
			cs1Type = cs_Rectangle;
		}
		DebugPrintf(512, "crRType = %i\n", cs1Type);
	}
	if (GetKeyDown(KeyCode_M))
	{
		detailsMode = !detailsMode;
	}
	if (GetKeyDown(KeyCode_N))
	{
		showcs2 = !showcs2;
	}
	if (GetKeyDown(KeyCode_B))
	{
		gjkMode = (GJKMode)(gjkMode + 1);
		if (gjkMode == cs_COUNT)
		{
			gjkMode = gjk_Simple;
		}
		DebugPrintf(512, "gjkMode = %i\n", gjkMode);
	}

	
	
	
	mat4 Bmodelcs2 = cs2Rectangle.transform;
	mat4 Bmodelcs1 = mat4(1, 0, 0,0,    0, 1, 0,0,   0,0,1,0,    cs1Pos.x, cs1Pos.y,0, 1) * mat4(cos(cs1RotationAngle), sin(cs1RotationAngle), 0,0,    -sin(cs1RotationAngle), cos(cs1RotationAngle), 0,0,   0,0,1,0,    0, 0,0, 1);
	cs1Rectangle.transform = Bmodelcs1;
	cs1Triangle.transform = Bmodelcs1;
	cs1Circle.origin = vec3(cs1Pos.x, cs1Pos.y, 0);
	bool collision = false;
	if (cs1Type == cs_Rectangle)
	{
		if (gjkMode == gjk_Simple)
		{
			collision = DetectCollision(cs1Rectangle, cs2Rectangle).collided;
		}
// 		else if (gjkMode == gjk_Casey)
// 		{
// 			collision = GJK_Casey(cs1Rectangle, cs2Rectangle);
// 		}
		else if (gjkMode == gjk_AllVoronoi)
		{
			collision = DetectCollision_AllVoronoi(cs1Rectangle, cs2Rectangle).collided;
		}
	}
	if (cs1Type == cs_Triangle)
	{
		collision = DetectCollision(cs1Triangle, cs2Rectangle).collided;
	}
	if (cs1Type == cs_Circle)
	{
		collision = DetectCollision(cs1Circle, cs2Rectangle).collided;
	}

	if (collision)
	{
		cs2Color = lightGreen;
		cs1Color = darkGreen;
	}
	else
	{
		cs2Color = lightRed;
		cs1Color = darkRed;
	}


	// cs2
	glUseProgram(solidColorQuadShaderProgram);
	glBindVertexArray(texturedQuadVAO);
	mat3 Bmodelcs2_in3 = mat3(vec3(Bmodelcs2[0].x, Bmodelcs2[0].y, Bmodelcs2[0].w), vec3(Bmodelcs2[1].x, Bmodelcs2[1].y, Bmodelcs2[1].w), vec3(Bmodelcs2[3].x, Bmodelcs2[3].y, Bmodelcs2[3].w));
	PCM = Oprojection * inverse(Ccamera) * Bmodelcs2_in3;
	glUniformMatrix3fv(solidColorQuadPCMLocation, 1, GL_FALSE, &PCM[0][0]);
	glUniform4fv(solidColorQuadQuadColorLocation, 1, &cs2Color[0]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices);
	
	// cs1
	glUseProgram(solidColorQuadShaderProgram);
	mat3 Bmodelcs1_in3 = mat3(vec3(Bmodelcs1[0].x, Bmodelcs1[0].y, Bmodelcs1[0].w), vec3(Bmodelcs1[1].x, Bmodelcs1[1].y, Bmodelcs1[1].w), vec3(Bmodelcs1[3].x, Bmodelcs1[3].y, Bmodelcs1[3].w));
	PCM = Oprojection * inverse(Ccamera) * Bmodelcs1_in3;
	glUniformMatrix3fv(solidColorQuadPCMLocation, 1, GL_FALSE, &PCM[0][0]);
	glUniform4fv(solidColorQuadQuadColorLocation, 1, &cs1Color[0]);
	if (cs1Type == cs_Rectangle)
	{
		// When cs1 is a rectangle
		glBindVertexArray(texturedQuadVAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices);
	}
	if (cs1Type == cs_Triangle)
	{
		// When cs1 is a triangle
		glBindVertexArray(equalateralTriangleVAO);
		glDrawArrays(GL_TRIANGLES, 0, numPointsInTriangle*equalateralTrianglePointDimensionality);
	}
	if (cs1Type == cs_Circle)
	{
		glUseProgram(solidColorCircleInPointShaderProgram);
		glBindVertexArray(circleVAO);
		glUniformMatrix3fv(solidColorCircleInPointPCMLocation, 1, GL_FALSE, &PCM[0][0]);
		glUniform4fv(solidColorCircleInPointCircleColorLocation, 1, &cs1Color[0]);
		F32 unitCircleSize = 800.0f / ((F32)numGridLines/2.0f);
		glPointSize(cs1Circle.radius * unitCircleSize);
		glDrawArrays(GL_POINTS, 0, 1);
	}

	// Print out collision information
	if (GetKeyDown(KeyCode_Spacebar))
	{
		DebugPrintf(512, "\n\n\nCOLLISION SAMPLE %u\n\n", sampleNumber++);
		(collision) ? DebugPrint("collision = TRUE\n\n") : DebugPrint("collision = FALSE\n\n");

		if (cs1Type == cs_Rectangle)
		{
			DebugPrint("COLLISION SHAPE 1(RECTANGLE)\n");

			if (detailsMode)
			{
				DebugPrintf(1024, "origin = (%f, %f)\n", cs1Rectangle.origin.x, cs1Rectangle.origin.y);
				DebugPrintf(1024, "halfDim = (%f, %f)\n\n", cs1Rectangle.halfDim.x, cs1Rectangle.halfDim.y);
				mat4 t = cs1Rectangle.transform;
				DebugPrintf(2048, "transform = %f %f %f %f\n            %f %f %f %f\n            %f %f %f %f\n            %f %f %f %f\n\n", t[0][0], t[1][0], t[2][0], t[3][0], t[0][1], t[1][1], t[2][1], t[3][1], t[0][2], t[1][2], t[2][2], t[3][2], t[0][3], t[1][3], t[2][3], t[3][3]);
			}

			vec4 cs1p0 = cs1Rectangle.transform * vec4(cs1Rectangle.origin.x - cs1Rectangle.halfDim.x, cs1Rectangle.origin.y - cs1Rectangle.halfDim.y, 0.0f, 1.0f);
			vec4 cs1p1 = cs1Rectangle.transform * vec4(cs1Rectangle.origin.x - cs1Rectangle.halfDim.x, cs1Rectangle.origin.y + cs1Rectangle.halfDim.y, 0.0f, 1.0f);
			vec4 cs1p2 = cs1Rectangle.transform * vec4(cs1Rectangle.origin.x + cs1Rectangle.halfDim.x, cs1Rectangle.origin.y - cs1Rectangle.halfDim.y, 0.0f, 1.0f);
			vec4 cs1p3 = cs1Rectangle.transform * vec4(cs1Rectangle.origin.x + cs1Rectangle.halfDim.x, cs1Rectangle.origin.y + cs1Rectangle.halfDim.y, 0.0f, 1.0f);
			DebugPrintf(1024, "Transformed Points\np[0] = (%f,%f)\np[1] = (%f,%f)\np[2] = (%f,%f)\np[3] = (%f,%f)\n\n", cs1p0.x, cs1p0.y, cs1p2.x, cs1p2.y, cs1p3.x, cs1p3.y, cs1p1.x, cs1p1.y);
		}
		else if (cs1Type == cs_Triangle)
		{
			DebugPrint("COLLISION SHAPE 1(TRIANGLE)\n");

			if (detailsMode)
			{
				DebugPrintf(1024, "origin = (%f, %f)\n\n", cs1Triangle.origin.x, cs1Triangle.origin.y);
				mat4 t = cs1Triangle.transform;
				DebugPrintf(2048, "transform = %f %f %f %f\n            %f %f %f %f\n            %f %f %f %f\n            %f %f %f %f\n\n", t[0][0], t[1][0], t[2][0], t[3][0], t[0][1], t[1][1], t[2][1], t[3][1], t[0][2], t[1][2], t[2][2], t[3][2], t[0][3], t[1][3], t[2][3], t[3][3]);
				vec3 p0 = cs1Triangle.points[0];
				vec3 p1 = cs1Triangle.points[1];
				vec3 p2 = cs1Triangle.points[2];
				DebugPrintf(2048, "Points\np[0] = (%f,%f)\np[1] = (%f,%f)\np[2] = (%f,%f)\n\n", p0.x, p0.y, p1.x, p1.y, p2.x, p2.y);
			}

			vec4 cs1p0 = cs1Triangle.transform * vec4(cs1Triangle.origin.x + cs1Triangle.points[0].x, cs1Triangle.origin.y + cs1Triangle.points[0].y, 0.0f, 1.0f);
			vec4 cs1p1 = cs1Triangle.transform * vec4(cs1Triangle.origin.x + cs1Triangle.points[1].x, cs1Triangle.origin.y + cs1Triangle.points[1].y, 0.0f, 1.0f);
			vec4 cs1p2 = cs1Triangle.transform * vec4(cs1Triangle.origin.x + cs1Triangle.points[2].x, cs1Triangle.origin.y + cs1Triangle.points[2].y, 0.0f, 1.0f);
			DebugPrintf(1024, "Transformed Points\np[0] = (%f,%f)\np[1] = (%f,%f)\np[2] = (%f,%f)\n\n", cs1p0.x, cs1p0.y, cs1p1.x, cs1p1.y, cs1p2.x, cs1p2.y);
		}
		else if (cs1Type == cs_Circle)
		{
			DebugPrint("COLLISION SHAPE 1(CIRCLE)\n");
			DebugPrintf(1024, "origin = (%f, %f)\n", cs1Circle.origin.x, cs1Circle.origin.y);
			DebugPrintf(512, "radius = %f\n\n", cs1Circle.radius);
		}
		
		if (showcs2)
		{
			vec4 cs2p0 = cs2Rectangle.transform * vec4(cs2Rectangle.origin.x - cs2Rectangle.halfDim.x, cs2Rectangle.origin.y - cs2Rectangle.halfDim.y, 0.0f, 1.0f);
			vec4 cs2p1 = cs2Rectangle.transform * vec4(cs2Rectangle.origin.x - cs2Rectangle.halfDim.x, cs2Rectangle.origin.y + cs2Rectangle.halfDim.y, 0.0f, 1.0f);
			vec4 cs2p2 = cs2Rectangle.transform * vec4(cs2Rectangle.origin.x + cs2Rectangle.halfDim.x, cs2Rectangle.origin.y - cs2Rectangle.halfDim.y, 0.0f, 1.0f);
			vec4 cs2p3 = cs2Rectangle.transform * vec4(cs2Rectangle.origin.x + cs2Rectangle.halfDim.x, cs2Rectangle.origin.y + cs2Rectangle.halfDim.y, 0.0f, 1.0f);
			DebugPrintf(1024, "COLLISION SHAPE 2(RECTANGLE)\nTransformed Points\np[0] = (%f,%f)\np[1] = (%f,%f)\np[2] = (%f,%f)\np[3] = (%f,%f)\n", cs2p0.x, cs2p0.y, cs2p2.x, cs2p2.y, cs2p3.x, cs2p3.y, cs2p1.x, cs2p1.y);
		}
	}
}


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

void collisionScene3DUpdate(F32 deltaTime)
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
	vec3 cb_x = normalize(cross(cb_z, vec3(0,1,0)));
	vec3 cb_y = normalize(cross(cb_x, cb_z));

	//mat4 Ccamera = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1, 0, camera3DPosition.x, camera3DPosition.y, camera3DPosition.z, 1);
	//mat4 cBasis = mat4(cb_x.x, cb_x.y, cb_x.z, 0, cb_y.x, cb_y.y, cb_y.z, 0, cb_z.x, cb_z.y, cb_z.z, 0, 0, 0, 0, 1);
	
	vec3 cameraPosition_Front = vec3(0,0,-10);
	vec3 cameraPosition_Right = vec3(10,0,0);
	vec3 cameraPosition_Top = vec3(0,10,0);

	// World
	mat4 cBasis_Front = mat4(vec4(1,0,0,0), vec4(0,1,0,0), vec4(0,0,1,0), vec4(0,0,0,1));  // x y  z w
	mat4 cBasis_Right = mat4(vec4(0,0,1,0), vec4(0,1,0,0), -vec4(1,0,0,0), vec4(0,0,0,1)); // z y -x w
	mat4 cBasis_Top = mat4(vec4(1,0,0,0), vec4(0,0,1,0), -vec4(0,1,0,0), vec4(0,0,0,1));   // x z -y w

	// CS1
	mat4 cBasis_Front_1 = mat4(normalize(Bmodel_cs1[0]), normalize(Bmodel_cs1[1]), normalize(Bmodel_cs1[2]), vec4(0,0,0,1));
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
		cPosition = vec3(0,0,0);
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
		vec4 orientationColor = vec4(0,1,0,1);
		glUseProgram(texturedQuadShaderProgram);
		glBindTexture(GL_TEXTURE_2D, frontTexture);
		glBindVertexArray(texturedQuadVAO);
		PCM = mat3(vec3(1,0,0), vec3(0,1,0), vec3(0,0,1)) * mat3(vec3(.2,0,0), vec3(0,.2,0), vec3(-.9,-.9,1));
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


// 	if (moved)
// 	{
		//DebugPrintf(512, "penetration depth: %f\n", penetrationDepth);
//	}
}

void GameUpdate(F32 deltaTime)
{
#ifdef COLLISION2DOLD
	collisionScene2DUpdate(deltaTime);
#elif COLLISION3D
	collisionScene3DUpdate(deltaTime);
#elif COLLISION2DAPPLET
	UpdateCollisionDetection2DApplet(deltaTime);
#endif
}

bool GameShutdown()
{
	glDeleteProgram(texturedQuadShaderProgram);
	return true;
}





verifyShaderReturnResult verifyShader(GLuint shader)
{
	verifyShaderReturnResult result = { true, NULL, 0 };

	GLint shaderCompileStatus = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompileStatus);
	if (shaderCompileStatus != GL_TRUE)
	{
		GLint infoLogLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLsizei returnedInfoLogLength = 0;
		GLchar *infoLog = (GLchar *)malloc(sizeof(GLchar)* infoLogLength);
		glGetShaderInfoLog(shader, infoLogLength, &returnedInfoLogLength, infoLog);

		result.compiled = false;
		result.infoLog = infoLog;
		result.infoLogLength = returnedInfoLogLength;
	}

	return result;
}

verifyProgramReturnResult verifyProgram(GLuint program)
{
	verifyProgramReturnResult result = { true, NULL, 0 };

	GLint programLinkStatus = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &programLinkStatus);
	if (programLinkStatus != GL_TRUE)
	{
		GLint infoLogLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLsizei returnedInfoLogLength = 0;
		GLchar *infoLog = (GLchar *)malloc(sizeof(GLchar)* infoLogLength);
		glGetShaderInfoLog(program, infoLogLength, &returnedInfoLogLength, infoLog);

		result.compiled = false;
		result.infoLog = infoLog;
		result.infoLogLength = returnedInfoLogLength;
	}

	return result;
}