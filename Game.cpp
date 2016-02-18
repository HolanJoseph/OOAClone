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

struct Entity
{
	vec2 position;
	GLuint texture;
};

struct Camera
{
	vec2 position;
	vec2 viewArea;
};

Entity* entities;
Camera  camera;

void InitScene()
{
	camera.position = vec2(35, -12.5f);
	camera.viewArea.x = 10;
	camera.viewArea.y = 9;

	entities = (Entity*)malloc(sizeof(Entity) * (10*9));

	char* tileFilenames[] = {
		// System Bar
		"Assets/sysBar.bmp",
		"Assets/sysBar.bmp",
		"Assets/sysBar.bmp",
		"Assets/sysBar.bmp",
		"Assets/sysBar.bmp",
		"Assets/sysBar.bmp",
		"Assets/sysBar.bmp",
		"Assets/sysBar.bmp",
		"Assets/sysBar.bmp",
		"Assets/sysBar.bmp",

		// row 1
		"Assets/treeLowRight.bmp",
		"Assets/treeLowLeft.bmp",
		"Assets/treeLowRight.bmp",
		"Assets/yellow.bmp",
		"Assets/yellow.bmp",
		"Assets/greenFringeBL.bmp",
		"Assets/green.bmp",
		"Assets/greenFringeR.bmp",
		"Assets/treeLowLeft.bmp",
		"Assets/treeLowRight.bmp",

		// row 2
		"Assets/treeHighRight.bmp",
		"Assets/greenFringeUL.bmp",
		"Assets/greenFringeU.bmp",
		"Assets/greenFringeUR.bmp",
		"Assets/yellow.bmp",
		"Assets/yellow.bmp",
		"Assets/greenFringeL.bmp",
		"Assets/greenFringeR.bmp",
		"Assets/treeHighLeft.bmp",
		"Assets/treeHighRight.bmp",

		// row 3
		"Assets/treeLowRight.bmp",
		"Assets/greenFringeL.bmp",
		"Assets/greenFlowers.bmp",
		"Assets/greenFringeR.bmp",
		"Assets/yellow.bmp",
		"Assets/greenFringeUL.bmp",
		"Assets/greenWeeds.bmp",
		"Assets/greenFringeBR.bmp",
		"Assets/treeLowLeft.bmp",
		"Assets/treeLowRight.bmp",
		
		// row 4
		"Assets/treeHighRight.bmp",
		"Assets/greenFringeBL.bmp",
		"Assets/greenFringeB.bmp",
		"Assets/greenWeeds.bmp",
		"Assets/greenFringeU.bmp",
		"Assets/green.bmp",
		"Assets/greenFringeBR.bmp",
		"Assets/yellow.bmp",
		"Assets/treeHighLeft.bmp",
		"Assets/treeHighRight.bmp",
		
		// row 5
		"Assets/treeLowRight.bmp",
		"Assets/weed.bmp",
		"Assets/weed.bmp",
		"Assets/greenFringeL.bmp",
		"Assets/green.bmp",
		"Assets/greenFringeR.bmp",
		"Assets/yellow.bmp",
		"Assets/weed.bmp",
		"Assets/treeLowLeft.bmp",
		"Assets/treeLowRight.bmp",

		// row 6
		"Assets/treeHighRight.bmp",
		"Assets/weed.bmp",
		"Assets/greenFringeUL.bmp",
		"Assets/green.bmp",
		"Assets/greenFlowers.bmp",
		"Assets/greenFringeR.bmp",
		"Assets/weed.bmp",
		"Assets/weed.bmp",
		"Assets/treeHighLeft.bmp",
		"Assets/treeHighRight.bmp",

		// row 7
		"Assets/treeLowRight.bmp",
		"Assets/yellow.bmp",
		"Assets/greenFringeBL.bmp",
		"Assets/greenFringeB.bmp",
		"Assets/greenFringeB.bmp",
		"Assets/greenFringeBR.bmp",
		"Assets/weed.bmp",
		"Assets/weed.bmp",
		"Assets/treeLowLeft.bmp",
		"Assets/treeLowRight.bmp",

		// row 8
		"Assets/rails.bmp",
		"Assets/rails.bmp",
		"Assets/rails.bmp",
		"Assets/rails.bmp",
		"Assets/rails.bmp",
		"Assets/rails.bmp",
		"Assets/rails.bmp",
		"Assets/rails.bmp",
		"Assets/rails.bmp",
		"Assets/rails.bmp",
	};

	for (U32 j = 0; j < 9; ++j)
	{
		for (U32 i = 0; i < 10; ++i)
		{
			vec2 pos(30.5f + i, -8.5f - j);
			char* filename = tileFilenames[10 * j + i];

			entities[10 * j + i].position = pos;

			I32 textureWidth = 0;
			I32 textureHeight = 0;
			I32 textureImageComponents = 0;
			I32 textureNumImageComponentsDesired = 4;
			U8 * textureData = stbi_load(filename, &textureWidth, &textureHeight, &textureImageComponents, textureNumImageComponentsDesired); // Channel order?

			glActiveTexture(GL_TEXTURE0);
			glGenTextures(1, &(entities[10 * j + i].texture));
			glBindTexture(GL_TEXTURE_2D, entities[10 * j + i].texture);
			glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, textureWidth, textureHeight);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
		}
	}

}

bool GameInit()
{
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

	

	// Set the spriteSampler sampler variable in the shader to fetch data from the 0th texture location
	spriteSamplerLocation = glGetUniformLocation(shaderProgram, "spriteSampler");
	glUniform1i(spriteSamplerLocation, 0);


	PCMLocation = glGetUniformLocation(shaderProgram, "PCM");

	InitScene();

	return true;
}

void GameUpdate(F32 deltaTime)
{

	// NOTE: Setting the viewport each frame shouldnt happen
	glViewport(0, 0, 600, 500);

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

	for (U32 i = 0; i < 10 * 9; ++i)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, entities[i].texture);
		
		mat3 Bmodel = mat3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, entities[i].position.x, entities[i].position.y, 1.0f);
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