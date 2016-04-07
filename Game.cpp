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
#include "CollisionDetection3D_Applet.h"

//#define COLLISION2DOLD 1
#define COLLISION3D 1
//#define COLLISION2DAPPLET 1

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

struct GameCamera
{
	vec2 position;
	vec2 viewArea;
};

U32 numCollisionEntities = 2;
Rectangle cs2Rectangle;
Rectangle cs1Rectangle;
Triangle  cs1Triangle;
Circle    cs1Circle;
GameCamera collisionCamera;

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








U32 numEntities = (10 * 9) + 1;
Entity* entities;
GameCamera  camera;
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
	InitializeCollisionDetection3DApplet();
#elif COLLISION2DAPPLET
	Init2DCollisionTestScene();
	InitializeCollisionDetection2DApplet();
#endif

	return true;
}



U32 sampleNumber = 0;
bool detailsMode = true;
bool showcs2 = false;
bool smoothMode = true;








void GameUpdate(F32 deltaTime)
{
#ifdef COLLISION2DOLD
	collisionScene2DUpdate(deltaTime);
#elif COLLISION3D
	UpdateCollisionDetection3DApplet(deltaTime);
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