#include "stb_image.h"

#include "glew/GL/glew.h"
#include <gl/GL.h>

#include "Types.h"
#include "Math.h"
#include "BitManip.h"
#include "Util.h"

#include "GameAPI.h"
#include "DebugAPI.h"
#include "InputAPI.h"
#include "FileAPI.h"


struct Square
{
	mat3 transform;

	vec2 origin; // NOTE: this is in "model" space
	vec2 halfDim;
};

vec2 Support(Square* A, vec2 direction)
{
	vec3 AiPoints[4] = {
		A->transform * vec3(A->origin.x - A->halfDim.x, A->origin.y - A->halfDim.y, 1.0f),
		A->transform * vec3(A->origin.x - A->halfDim.x, A->origin.y + A->halfDim.y, 1.0f),
		A->transform * vec3(A->origin.x + A->halfDim.x, A->origin.y - A->halfDim.y, 1.0f),
		A->transform * vec3(A->origin.x + A->halfDim.x, A->origin.y + A->halfDim.y, 1.0f)
	};
	F32 AiDots[4] = {
		dot(direction, vec2(AiPoints[0].x, AiPoints[0].y)),
		dot(direction, vec2(AiPoints[1].x, AiPoints[1].y)),
		dot(direction, vec2(AiPoints[2].x, AiPoints[2].y)),
		dot(direction, vec2(AiPoints[3].x, AiPoints[3].y))
	};

	U32 maxPositionAi = 0;
	F32 maxDotAi = -1000000.0f;
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



struct Circle
{
	vec2 origin;
	F32  radius;
};

vec2 Support(Circle* A, vec2 direction)
{
	direction = normalize(direction);
	vec2 maxA = A->origin + (A->radius * direction);

	return maxA;
}



struct Triangle
{
	mat3 transform;
	
	vec2 origin; // NOTE: this is in "model" space
	vec2 points[3];
};

vec2 Support(Triangle* A, vec2 direction)
{
	vec3 AiPoints[3] = {
		A->transform * vec3(A->origin.x - A->points[0].x, A->origin.y - A->points[0].y, 1.0f),
		A->transform * vec3(A->origin.x - A->points[1].x, A->origin.y + A->points[1].y, 1.0f),
		A->transform * vec3(A->origin.x + A->points[2].x, A->origin.y - A->points[2].y, 1.0f)
	};
	F32 AiDots[3] = {
		dot(direction, vec2(AiPoints[0].x, AiPoints[0].y)),
		dot(direction, vec2(AiPoints[1].x, AiPoints[1].y)),
		dot(direction, vec2(AiPoints[2].x, AiPoints[2].y))
	};

	U32 maxPositionAi = 0;
	F32 maxDotAi = -1000000.0f;
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



struct OrientedBoundingBox
{
	mat4 transform;
	
	vec3 origin; // NOTE: this is in "model" space
	vec3 halfDim;
};

vec3 Support(OrientedBoundingBox* A, vec3 direction)
{
	vec4 AiPoints[8] = {
		A->transform * vec4(A->origin.x - A->halfDim.x, A->origin.y - A->halfDim.y, A->origin.z + A->halfDim.z, 1.0f),
		A->transform * vec4(A->origin.x - A->halfDim.x, A->origin.y + A->halfDim.y, A->origin.z + A->halfDim.z, 1.0f),
		A->transform * vec4(A->origin.x + A->halfDim.x, A->origin.y - A->halfDim.y, A->origin.z + A->halfDim.z, 1.0f),
		A->transform * vec4(A->origin.x + A->halfDim.x, A->origin.y + A->halfDim.y, A->origin.z + A->halfDim.z, 1.0f),
		A->transform * vec4(A->origin.x - A->halfDim.x, A->origin.y - A->halfDim.y, A->origin.z - A->halfDim.z, 1.0f),
		A->transform * vec4(A->origin.x - A->halfDim.x, A->origin.y + A->halfDim.y, A->origin.z - A->halfDim.z, 1.0f),
		A->transform * vec4(A->origin.x + A->halfDim.x, A->origin.y - A->halfDim.y, A->origin.z - A->halfDim.z, 1.0f),
		A->transform * vec4(A->origin.x + A->halfDim.x, A->origin.y + A->halfDim.y, A->origin.z - A->halfDim.z, 1.0f),
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
	F32 maxDotAi = -1000000.0f;
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

vec3 Support(Sphere* A, vec3 direction)
{
	direction = normalize(direction);
	vec3 maxA = A->origin + (A->radius * direction);

	return maxA;
}



struct Capsule
{
	mat4 transform;

	vec3 points[2]; // NOTE: these are in "model" space
	F32 radius;
};

vec3 Support(Capsule* A, vec3 direction)
{
	vec4 AiPoints[2] = {
		A->transform * vec4(A->points[0].x, A->points[0].y, A->points[0].z, 1.0f),
		A->transform * vec4(A->points[1].x, A->points[1].y, A->points[1].z, 1.0f)
	};
	F32 AiDots[2] = {
		dot(direction, vec3(AiPoints[0])),
		dot(direction, vec3(AiPoints[1]))
	};

	U32 maxPositionAi = 0;
	F32 maxDotAi = -1000000.0f;
	for (U32 i = 0; i < 3; ++i)
	{
		if (AiDots[i] > maxDotAi)
		{
			maxPositionAi = i;
			maxDotAi = AiDots[i];
		}
	}

	vec3 maxLinePointA = vec3(AiPoints[maxPositionAi].x, AiPoints[maxPositionAi].y, AiPoints[maxPositionAi].z);
	vec3 maxA = maxLinePointA + (normalize(direction) * A->radius);

	return maxA;
}


/*
	COMBO SUPPORTS
*/
vec2 Support(Square* A, Square* B, vec2* direction)
{
	// find the point in A that has the largest value with dot(Ai, Direction)
	vec2 maxA = Support(A, *direction);

	// find the point in B that has the largest value with dot(Bj, Direction)
	vec2 maxB = Support(B, -*direction);
	vec2 result = maxA - maxB;
	return result;
}



enum SimplexType
{
	Simplex_Point = 0,
	Simplex_Line = 1,
	Simplex_Triangle = 2,
	Simplex_Tetrahedron = 3
};

bool DoSimplexLineCasey(vec2* simplex, SimplexType* simplexType, vec2* D)
{
	bool result = false;

	vec3 ab = vec3(simplex[0].x - simplex[1].x, simplex[0].y - simplex[1].y, 0);
	vec3 ag = vec3(0-simplex[1].x, 0-simplex[1].y, 0);
	if (dot(ab, ag) > 0)
	{
		vec3 DinR3 = cross(cross(ab, ag), ab);
		*D = vec2(DinR3.x, DinR3.y);
	}
	else
	{
		Assert(false);
		simplex[0] = simplex[1];
		simplex[1] = vec2();
		*simplexType = Simplex_Point;
	}

	return result;
}

bool DoSimplexTriangleCasey(vec2* simplex, SimplexType* simplexType, vec2* D)
{
	bool result = false;

	vec3 ab = vec3((simplex[1] - simplex[2]), 0);
	vec3 ac = vec3((simplex[0] - simplex[2]), 0);
	vec3 ag = vec3((vec2(0,0) - simplex[2]), 0);
	vec3 abc = cross(ab, ac);

	if (dot(cross(abc, ac), ag) > 0)
	{
		if (dot(ac, ag) > 0)
		{
			simplex[1] = simplex[2];
			simplex[2] = vec2();
			*simplexType = Simplex_Line;
			vec3 dInR3 = cross(cross(ac, ag), ac);
			*D = vec2(dInR3.x, dInR3.y);
		}
		else
		{
			// Magical special fun times case
			if (dot(ab, ag) > 0)
			{
				*simplexType = Simplex_Line;
				simplex[1] = simplex[2];
				simplex[2] = vec2();
				vec3 dInR3 = cross(cross(ab, ag), ab);
				*D = vec2(dInR3.x, dInR3.y);
			}
			else
			{
				Assert(false);
				*simplexType = Simplex_Point;
				simplex[0] = simplex[2];
				simplex[1] = vec2();
				simplex[2] = vec2();
				*D = vec2(ag.x, ag.y);
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
				*simplexType = Simplex_Line;
				simplex[1] = simplex[2];
				simplex[2] = vec2();
				vec3 dInR3 = cross(cross(ab, ag), ab);
				*D = vec2(dInR3.x, dInR3.y);
			}
			else
			{
				Assert(false);
				*simplexType = Simplex_Point;
				simplex[0] = simplex[2];
				simplex[1] = vec2();
				simplex[2] = vec2();
				*D = vec2(ag.x, ag.y);
			}
		}
		else
		{
			// NOTE: In the 2D case this means the origin is within the triangle.
			result = true;
			// NOTE: This is for the 3D case.
			/*if (dot(abc, ag) > 0)
			{
				*D = abc;
			}
			else
			{
				// NOTE: Swap points so plane normal will be in the direction of the new point.
				vec3 t = simplex[1];
				simplex[1] = simplex[0];
				simplex[0] = t;
				*D = -abc;
			}*/
		}
	}

	return result;
}

bool DoSimplexTetrahedron(vec2* simplex, SimplexType* simplexType, vec2* D)
{
	bool result = false;

	return result;
}

bool DoSimplex(vec2* simplex, SimplexType* simplexType, vec2* D)
{
	bool result = false;

	switch (*simplexType)
	{
	case Simplex_Line:
	{
						 result = DoSimplexLineCasey(simplex, simplexType, D);
						 break;
	}

	case Simplex_Triangle:
	{
							 result = DoSimplexTriangleCasey(simplex, simplexType, D);
							 break;
	}

	case Simplex_Tetrahedron:
	{
								result = DoSimplexTetrahedron(simplex, simplexType, D);
								break;
	}

	default:
	{
			   break;
	}
	}

	return result;
}

void CollisionDetection()
{
// 	vec2 d = vec2(1, -2);
// 	vec2 p1 = vec2(-3, 6);
// 	vec2 p2 = vec2(-6, 1 ); 
// 	vec2 p3 = vec2(-6, -5);
// 	vec2 p4 = vec2(0, -5);
// 	vec2 p5 = vec2(6, -5);
// 	vec2 p6 = vec2(6, 1);
// 	vec2 p7 = vec2(3, 6);
// 
// 	F32 d1 = dot(d, p1);
// 	F32 d2 = dot(d, p2);
// 	F32 d3 = dot(d, p3);
// 	F32 d4 = dot(d, p4);
// 	F32 d5 = dot(d, p5);
// 	F32 d6 = dot(d, p6);
// 	F32 d7 = dot(d, p7);

	bool collisionDetected = false;

	Square shapeA;
	shapeA.origin = vec2(2.0f, 2.0f);
	shapeA.halfDim = vec2(1.0f, 1.0f);

	Square shapeB;
	shapeB.origin = vec2(3.0f, 3.0f);
	shapeB.halfDim = vec2(1.0f, 1.0f);

	vec2 S = Support(&shapeA, &shapeB, &vec2(1.0f, -1.0f));
	vec2 simplex[4];
	SimplexType simplexType = Simplex_Point;
	simplex[0] = S;
	vec2 D = -S;

	for (;;)
	{
		vec2 A = Support(&shapeA, &shapeB, &D);
		if (dot(A, D) < 0)
		{
			collisionDetected = false;
			break;
		}
		simplexType = (SimplexType)(simplexType + 1);
		simplex[simplexType] = A;
		if (DoSimplex(simplex, &simplexType, &D))
		{
			collisionDetected = true;
			break;
		}
	}
}



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


GLuint shaderProgram;

GLuint spriteSamplerLocation;
GLuint PCMLocation;

GLuint texture;

GLuint textureSampler;

struct Entity
{
	vec2 position;
	vec2 scale;
	GLuint texture;
};

struct Camera
{
	vec2 position;
	vec2 viewArea;
};

U32 numEntities = (10 * 9) + 1;
Entity* entities;
Camera  camera;
U32 linkEntityLocation = numEntities - 1;

void InitScene()
{
	camera.position = vec2(35, -12.5f);
	camera.viewArea.x = 10;
	camera.viewArea.y = 9;

	entities = (Entity*)malloc(sizeof(Entity) * ((10*9)+1));

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
			U8 * textureData = stbi_load(filename, &textureWidth, &textureHeight, &textureImageComponents, textureNumImageComponentsDesired); // Channel order?

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
	U8 * textureData = stbi_load("Assets/x60/link.bmp", &textureWidth, &textureHeight, &textureImageComponents, textureNumImageComponentsDesired); // Channel order?

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &(entities[linkEntityLocation].texture));
	glBindTexture(GL_TEXTURE_2D, entities[linkEntityLocation].texture);
	glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, textureWidth, textureHeight);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, textureData);

}

bool GameInit()
{
	CollisionDetection();

	glClearColor(0.32f, 0.18f, 0.66f, 0.0f);

	glGenVertexArrays(1, &texturedQuadVAO);
	glBindVertexArray(texturedQuadVAO);
	// NOTE: quad for testing, corrected for aspect ratio
	GLfloat vertices[numVertices * vertexDimensionality] =
	{
		-0.50f, -0.50f,
		 0.50f, -0.50f,
		 0.50f,  0.50f,
		-0.50f,  0.50f
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



	char* vertexShaderFile = "triangles.vert";
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	U64 vertexShaderFileSize = GetFileSize(vertexShaderFile).fileSize;
	char* vertexShaderSource = (char *)malloc(sizeof(char)* vertexShaderFileSize);
	const GLint glSizeRead = ReadFile(vertexShaderFile, vertexShaderSource, vertexShaderFileSize, 0).numberOfBytesRead;
	glShaderSource(vertexShader, 1, &vertexShaderSource, &glSizeRead);
	glCompileShader(vertexShader);
	verifyShaderReturnResult vertexVerification = verifyShader(vertexShader);
	if (!vertexVerification.compiled)
	{
		DebugPrint(vertexVerification.infoLog);
		return false;
	}

	char* fragmentShaderFile = "triangles.frag";
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	U64 fragmentShaderFileSize = GetFileSize(fragmentShaderFile).fileSize;
	char* fragmentShaderSource = (char *)malloc(sizeof(char)* fragmentShaderFileSize);
	const GLint glFragmentShaderSize = ReadFile(fragmentShaderFile, fragmentShaderSource, fragmentShaderFileSize, 0).numberOfBytesRead;
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, &glFragmentShaderSize);
	glCompileShader(fragmentShader);
	verifyShaderReturnResult fragmentVerification = verifyShader(fragmentShader);
	if (!fragmentVerification.compiled)
	{
		DebugPrint(fragmentVerification.infoLog);
		return false;
	}

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	verifyProgramReturnResult programVerification = verifyProgram(shaderProgram);
	if (!programVerification.compiled)
	{
		DebugPrint(programVerification.infoLog);
		return false;
	}
	glUseProgram(shaderProgram);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader); 

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)sizeof(vertices) /* NOTE: THIS IS A BYTE OFFSET*/);
	glEnableVertexAttribArray(1);

	stbi_set_flip_vertically_on_load(1);

	// textures
	// read in the texture
	I32 textureWidth = 0;
	I32 textureHeight = 0;
	I32 textureImageComponents = 0;
	I32 textureNumImageComponentsDesired = 4;
	U8 * textureData = stbi_load("Assets/tile1.bmp", &textureWidth, &textureHeight, &textureImageComponents, textureNumImageComponentsDesired); // Channel order?
	
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
	spriteSamplerLocation = glGetUniformLocation(shaderProgram, "spriteSampler");
	glUniform1i(spriteSamplerLocation, 0);


	PCMLocation = glGetUniformLocation(shaderProgram, "PCM");

	InitScene();

	return true;
}



void GameUpdate(F32 deltaTime)
{
	if (GetKey(KeyCode_W))
	{
		entities[linkEntityLocation].position += vec2(0.0f, 2*deltaTime);
	}
	if (GetKey(KeyCode_S))
	{
		entities[linkEntityLocation].position += vec2(0.0f, 2 * -deltaTime);
	}
	if (GetKey(KeyCode_A))
	{
		entities[linkEntityLocation].position += vec2(2 * -deltaTime, 0.0f);
	}
	if (GetKey(KeyCode_D))
	{
		entities[linkEntityLocation].position += vec2(2 * deltaTime, 0.0f);
	}

	camera.position = entities[linkEntityLocation].position;


	// NOTE: Setting the viewport each frame shouldnt happen
	glViewport(0, 0, 600, 540);

	// NOTE: Clear the buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// NOTE: Bind the VAO that holds the vertex information for the current object.
	glBindVertexArray(texturedQuadVAO);

	// NOTE: Bind the shader that will be used to draw it.
	glUseProgram(shaderProgram);

	// NOTE: Bind the texture that represents the gameobject, also make sure the texture is active.
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, texture);

	// NOTE: Draw this bitch.
	//glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices);

	for (U32 i = 0; i < numEntities; ++i)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, entities[i].texture);
		
		mat3 Bmodel = mat3(1.0f*entities[i].scale.x, 0.0f, 0.0f, 0.0f, 1.0f*entities[i].scale.y, 0.0f, entities[i].position.x, entities[i].position.y, 1.0f);
		mat3 Ccamera = mat3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, camera.position.x, camera.position.y, 1.0f);
		mat3 Oprojection = mat3((2.0f / camera.viewArea.x), 0.0f, 0.0f, 0.0f, (2.0f / camera.viewArea.y), 0.0f, 0.0f, 0.0f, 1.0f);
		mat3 PCM = Oprojection * inverse(Ccamera) * Bmodel;
		glUniformMatrix3fv(PCMLocation, 1, GL_FALSE, &PCM[0][0]);

		glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices);
	}
}

bool GameShutdown()
{
	glDeleteProgram(shaderProgram);
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