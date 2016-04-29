#pragma once

#include "Types.h"
#include "Math.h"

#include "DebugAPI.h"

#include "AssetLoading.h"

#include "WindowAPI.h"

#include "glew/GL/glew.h"
#include <gl/GL.h>


inline F32 GetPointSize()
{
	F32 result;

	glGetFloatv(GL_POINT_SIZE, &result);

	return result;
}

inline void SetPointSize(F32 newPointSize)
{
	glPointSize(newPointSize);
}

inline vec4 GetClearColor()
{
	F32 clearColor[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE, clearColor);

	vec4 result = vec4(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
	return result;
}

inline void SetClearColor(vec4 newClearColor)
{
	glClearColor(newClearColor.r, newClearColor.g, newClearColor.b, newClearColor.a);
}

inline void Clear()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

struct ViewportRectangle
{
	vec2 offset;
	vec2 dimensions;

	ViewportRectangle() : offset(vec2()), dimensions(vec2()) {};
};
inline ViewportRectangle GetViewport()
{
	I32 vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);

	ViewportRectangle result;

	result.offset = vec2(vp[0], vp[1]);
	result.dimensions = vec2(vp[2], vp[3]);

	return result;
}

inline void SetViewport(ViewportRectangle newViewport)
{
	glViewport((I32)newViewport.offset.x, (I32)newViewport.offset.y, (I32)newViewport.dimensions.x, (I32)newViewport.dimensions.y);
}

inline void SetViewport(vec2 offset, vec2 dimensions)
{
	ViewportRectangle t;
	t.offset = offset;
	t.dimensions = dimensions;

	SetViewport(t);
}






struct Texture
{
	GLuint glTextureID;

	size_t width;  // Px
	size_t height; // Px

	Texture() : glTextureID(0), width(0), height(0) {};
};

inline void Initialize(Texture* texture, char* filename)
{
	TextureData textureData = LoadTexture(filename);

	texture->width = textureData.width;
	texture->height = textureData.height;

	// send the texture data to OpenGL memory
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texture->glTextureID);
	glBindTexture(GL_TEXTURE_2D, texture->glTextureID);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, texture->width, texture->height); // 4 is 1????
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->width, texture->height, GL_RGBA, GL_UNSIGNED_BYTE, textureData.data);
	free(textureData.data);
}

inline void Destroy(Texture* texture)
{
	glDeleteTextures(1, &texture->glTextureID);

	texture->glTextureID = 0; // NOTE: This is probably redundant
	texture->width = 0;
	texture->height = 0;
}





#define SHADER_POSITION_LOCATION 0
#define SHADER_TEXTURECOORDINATES_LOCATION 1

inline void ClearVertexData()
{
	glBindVertexArray(0);
}

struct VertexData_Pos2D
{
	static const size_t FloatsPerPosition = 2;
	static const size_t PositionSizeInBytes = sizeof(F32)* FloatsPerPosition;

	GLuint vao;
	GLuint vbo;

	size_t numberOfVertices;

	VertexData_Pos2D() : vao(0), vbo(0), numberOfVertices(0) {};
};

inline void Initialize(VertexData_Pos2D* vd, vec2* positions, size_t numberOfVertices)
{
	glGenVertexArrays(1, &vd->vao);
	glBindVertexArray(vd->vao);

	glGenBuffers(1, &vd->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vd->vbo);

	vd->numberOfVertices = numberOfVertices;

	glBufferData(GL_ARRAY_BUFFER, VertexData_Pos2D::PositionSizeInBytes * vd->numberOfVertices, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, VertexData_Pos2D::PositionSizeInBytes * vd->numberOfVertices, positions);

	glVertexAttribPointer(SHADER_POSITION_LOCATION, VertexData_Pos2D::FloatsPerPosition, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(SHADER_POSITION_LOCATION);

	glBindVertexArray(0);
}

inline void Destroy(VertexData_Pos2D* vd)
{
	glDeleteVertexArrays(1, &vd->vao);
	glDeleteBuffers(1, &vd->vbo);

	vd->vao = 0;
	vd->vbo = 0;
	vd->numberOfVertices = 0;
}

inline void SetVertexData(VertexData_Pos2D* vd)
{
	glBindVertexArray(vd->vao);
}

inline void RebufferVertexData(VertexData_Pos2D* vd, vec2* positions, size_t numberOfVertices)
{
	glBindVertexArray(vd->vao);
	glBindBuffer(GL_ARRAY_BUFFER, vd->vbo);

	// NOTE: Untested, should work?
	if (numberOfVertices > vd->numberOfVertices)
	{
		glBufferData(GL_ARRAY_BUFFER, VertexData_Pos2D::PositionSizeInBytes * numberOfVertices, NULL, GL_STATIC_DRAW);
	}

	glBufferSubData(GL_ARRAY_BUFFER, 0, VertexData_Pos2D::PositionSizeInBytes * numberOfVertices, positions);

	vd->numberOfVertices = numberOfVertices;

	glBindVertexArray(0);
}



struct VertexData_Pos2D_UV
{
	static const size_t FloatsPerPosition = 2;
	static const size_t PositionSizeInBytes = sizeof(F32)* FloatsPerPosition;

	static const size_t FloatsPerUVPair = 2;
	static const size_t UVSizeInBytes = sizeof(F32)* FloatsPerUVPair;

	GLuint vao;
	GLuint vbo;

	size_t numberOfVertices;

	VertexData_Pos2D_UV() : vao(0), vbo(0), numberOfVertices(0) {};
};

inline void Initialize(VertexData_Pos2D_UV* vd, vec2* positions, vec2* textureCoordinates, size_t numberOfVertices)
{
	glGenVertexArrays(1, &vd->vao);
	glBindVertexArray(vd->vao);

	glGenBuffers(1, &vd->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vd->vbo);

	vd->numberOfVertices = numberOfVertices;

	glBufferData(GL_ARRAY_BUFFER, (VertexData_Pos2D_UV::PositionSizeInBytes * vd->numberOfVertices) + (VertexData_Pos2D_UV::UVSizeInBytes * vd->numberOfVertices), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, VertexData_Pos2D_UV::PositionSizeInBytes * vd->numberOfVertices, positions);
	glBufferSubData(GL_ARRAY_BUFFER, VertexData_Pos2D_UV::PositionSizeInBytes * vd->numberOfVertices, VertexData_Pos2D_UV::UVSizeInBytes * vd->numberOfVertices, textureCoordinates);

	glVertexAttribPointer(SHADER_POSITION_LOCATION, VertexData_Pos2D_UV::FloatsPerPosition, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(SHADER_POSITION_LOCATION);

	glVertexAttribPointer(SHADER_TEXTURECOORDINATES_LOCATION, VertexData_Pos2D_UV::FloatsPerUVPair, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(VertexData_Pos2D_UV::PositionSizeInBytes * numberOfVertices) /* NOTE: THIS IS A BYTE OFFSET*/);
	glEnableVertexAttribArray(SHADER_TEXTURECOORDINATES_LOCATION);

	glBindVertexArray(0);
}

inline void Destroy(VertexData_Pos2D_UV* vd)
{
	glDeleteVertexArrays(1, &vd->vao);
	glDeleteBuffers(1, &vd->vbo);

	vd->vao = 0;
	vd->vbo = 0;
	vd->numberOfVertices = 0;
}

inline void SetVertexData(VertexData_Pos2D_UV* vd)
{
	glBindVertexArray(vd->vao);
}



struct VertexData_Indexed_Pos2D_UV
{
	static const size_t FloatsPerPosition = 2;
	static const size_t PositionSizeInBytes = sizeof(F32)* FloatsPerPosition;

	static const size_t FloatsPerUVPair = 2;
	static const size_t UVSizeInBytes = sizeof(F32)* FloatsPerUVPair;

	static const size_t IndexSizeInBytes = sizeof(U32);

	GLuint vao;
	GLuint vbo;
	GLuint ibo; // Index Buffer Object

	size_t numberOfVertices;
	size_t numberOfIndices;

	VertexData_Indexed_Pos2D_UV() : vao(0), vbo(0), ibo(0), numberOfVertices(0), numberOfIndices(0) {};
};

inline void Initialize(VertexData_Indexed_Pos2D_UV* vd, vec2* positions, vec2* textureCoordinates, size_t numberOfVertices, U32* indices, size_t numberOfIndices)
{
	glGenVertexArrays(1, &vd->vao);
	glBindVertexArray(vd->vao);



	vd->numberOfVertices = numberOfVertices;

	glGenBuffers(1, &vd->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vd->vbo);

	glBufferData(GL_ARRAY_BUFFER, (VertexData_Indexed_Pos2D_UV::PositionSizeInBytes * vd->numberOfVertices) + (VertexData_Indexed_Pos2D_UV::UVSizeInBytes * vd->numberOfVertices), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, VertexData_Indexed_Pos2D_UV::PositionSizeInBytes * vd->numberOfVertices, positions);
	glBufferSubData(GL_ARRAY_BUFFER, VertexData_Indexed_Pos2D_UV::PositionSizeInBytes * vd->numberOfVertices, VertexData_Indexed_Pos2D_UV::UVSizeInBytes * vd->numberOfVertices, textureCoordinates);

	glVertexAttribPointer(SHADER_POSITION_LOCATION, VertexData_Indexed_Pos2D_UV::FloatsPerPosition, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(SHADER_POSITION_LOCATION);

	glVertexAttribPointer(SHADER_TEXTURECOORDINATES_LOCATION, VertexData_Indexed_Pos2D_UV::FloatsPerUVPair, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(VertexData_Indexed_Pos2D_UV::PositionSizeInBytes * vd->numberOfVertices) /* NOTE: THIS IS A BYTE OFFSET*/);
	glEnableVertexAttribArray(SHADER_TEXTURECOORDINATES_LOCATION);



	vd->numberOfIndices = numberOfIndices;

	glGenBuffers(1, &vd->ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vd->ibo);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, VertexData_Indexed_Pos2D_UV::IndexSizeInBytes*vd->numberOfIndices, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, VertexData_Indexed_Pos2D_UV::IndexSizeInBytes*vd->numberOfIndices, indices);



	glBindVertexArray(0);
}

inline void Destroy(VertexData_Indexed_Pos2D_UV* vd)
{
	glDeleteVertexArrays(1, &vd->vao);
	glDeleteBuffers(1, &vd->vbo);
	glDeleteBuffers(1, &vd->ibo);

	vd->vao = 0;
	vd->vbo = 0;
	vd->numberOfVertices = 0;
	vd->ibo = 0;
	vd->numberOfIndices = 0;
}

inline void SetVertexData(VertexData_Indexed_Pos2D_UV* vd)
{
	glBindVertexArray(vd->vao);
}






// NOTE: For future implementations that require more complete foundations
// Locations are denoted by a 
//	type(mat4, mat3, vec4, etc...)
//	name("PCM", "CircleColor", "QuadColor", "SpriteSampler", etc...)
struct GLCompileStatus
{
	bool compiled;
	char* infoLog;
	I64 infoLogLength;

	GLCompileStatus() : compiled(false), infoLog(NULL), infoLogLength(0) {};
	GLCompileStatus(bool initialCompileStatus) : compiled(initialCompileStatus), infoLog(NULL), infoLogLength(0) {};
};
GLCompileStatus verifyShader(GLuint shader)
{
	GLCompileStatus result = GLCompileStatus(true);

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

GLCompileStatus verifyProgram(GLuint program)
{
	GLCompileStatus result = GLCompileStatus(true);

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


struct LSPResult
{
	GLuint program;
	GLCompileStatus status;

	LSPResult() : program(0), status() {};
};
LSPResult LoadShaderProgram(char* vertexShaderFilename, char* fragmentShaderFilename)
{
	LSPResult result;

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	size_t vertexShaderFileSize = GetFileSize(vertexShaderFilename).fileSize;
	char* vertexShaderSource = (char *)malloc(sizeof(char)* vertexShaderFileSize);
	const GLint glSizeRead = (const GLint)ReadFile(vertexShaderFilename, vertexShaderSource, vertexShaderFileSize, 0).numberOfBytesRead; // NOTE: Shader files are not that big so this is safe
	glShaderSource(vertexShader, 1, &vertexShaderSource, &glSizeRead);
	glCompileShader(vertexShader);
	GLCompileStatus vertexVerification = verifyShader(vertexShader);
	if (!vertexVerification.compiled)
	{
		result.program = 0;
		result.status = vertexVerification;
		return result; // NOTE: Remove early out.
	}
	free(vertexVerification.infoLog); // NOTE: Why the hell is this here?? this should always be null if we get here

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	size_t fragmentShaderFileSize = GetFileSize(fragmentShaderFilename).fileSize;
	char* fragmentShaderSource = (char *)malloc(sizeof(char)* fragmentShaderFileSize);
	const GLint glFragmentShaderSize = (const GLint)ReadFile(fragmentShaderFilename, fragmentShaderSource, fragmentShaderFileSize, 0).numberOfBytesRead; // NOTE: Shader files are not that big so this is safe
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, &glFragmentShaderSize);
	glCompileShader(fragmentShader);
	GLCompileStatus fragmentVerification = verifyShader(fragmentShader);
	if (!fragmentVerification.compiled)
	{
		result.program = 0;
		result.status = fragmentVerification;
		return result; // NOTE: Remove early out.
	}
	free(fragmentVerification.infoLog); // NOTE: Why the hell is this here?? this should always be null if we get here

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	GLCompileStatus programVerification = verifyProgram(shaderProgram);
	if (!programVerification.compiled)
	{
		result.program = 0;
		result.status = programVerification;
		return result; // NOTE: Remove early out.
	}
	free(programVerification.infoLog); // NOTE: Why the hell is this here?? this should always be null if we get here

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	result.program = shaderProgram;
	result.status = GLCompileStatus(true);

	return result;
}


inline void ClearShaderProgram()
{
	glUseProgram(NULL);
}





struct BasicShaderProgram2D
{
	GLuint program;

	GLCompileStatus status;

	GLuint location_PCM;
	GLuint location_Color;

	BasicShaderProgram2D() : program(0), status(), location_PCM(), location_Color() {};
};

inline void Initialize(BasicShaderProgram2D* bsp, char* vertexShaderFilename, char* fragmentShaderFilename)
{
	LSPResult loadResult = LoadShaderProgram(vertexShaderFilename, fragmentShaderFilename);
	bsp->program = loadResult.program;
	bsp->status = loadResult.status;

	if (bsp->status.compiled)
	{
		bsp->location_PCM = glGetUniformLocation(bsp->program, "PCM");
		if (bsp->location_PCM == -1)
		{
			bsp->status.compiled = false;
			bsp->status.infoLog = "PCM location not found";
			bsp->status.infoLogLength = 23;
		}

		bsp->location_Color = glGetUniformLocation(bsp->program, "Color");
		if (bsp->location_Color == -1)
		{
			bsp->status.compiled = false;
			bsp->status.infoLog = "Color location not found";
			bsp->status.infoLogLength = 25;
		}
	}
}

inline void Destroy(BasicShaderProgram2D* bsp)
{
	glDeleteProgram(bsp->program);
}

inline void SetShaderProgram(BasicShaderProgram2D* bsp)
{
	glUseProgram(bsp->program);
}

inline void SetPCM(BasicShaderProgram2D* bsp, mat3* PCM)
{
	glUniformMatrix3fv(bsp->location_PCM, 1, GL_FALSE, &(*PCM)[0][0]);
}

inline void SetColor(BasicShaderProgram2D* bsp, vec4* color)
{
	glUniform4fv(bsp->location_Color, 1, &(*color)[0]);
}



struct SpriteShaderProgram2D
{
	GLuint program;

	GLCompileStatus status;

	GLuint location_PCM;
	GLuint location_SpriteSampler;

	GLuint textureSampler_SpriteSampler;

	SpriteShaderProgram2D() : program(0), status(), location_PCM(), location_SpriteSampler(), textureSampler_SpriteSampler() {};
};

inline void Initialize(SpriteShaderProgram2D* ssp, char* vertexShaderFilename, char* fragmentShaderFilename)
{
	LSPResult loadResult = LoadShaderProgram(vertexShaderFilename, fragmentShaderFilename);
	ssp->program = loadResult.program;
	ssp->status = loadResult.status;

	if (ssp->status.compiled)
	{
		ssp->location_PCM = glGetUniformLocation(ssp->program, "PCM");
		if (ssp->location_PCM == -1)
		{
			ssp->status.compiled = false;
			ssp->status.infoLog = "PCM location not found";
			ssp->status.infoLogLength = 23;
		}

		ssp->location_SpriteSampler = glGetUniformLocation(ssp->program, "spriteSampler");
		// Set the spriteSampler sampler variable in the shader to fetch data from the 0th texture location
		glUniform1i(ssp->location_SpriteSampler, 0);

		// Set up spriteSampler state
		glGenSamplers(1, &ssp->textureSampler_SpriteSampler);
		glBindSampler(0, ssp->textureSampler_SpriteSampler);
		glSamplerParameteri(ssp->textureSampler_SpriteSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(ssp->textureSampler_SpriteSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(ssp->textureSampler_SpriteSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glSamplerParameteri(ssp->textureSampler_SpriteSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		if (ssp->location_SpriteSampler == -1)
		{
			ssp->status.compiled = false;
			ssp->status.infoLog = "spriteSampler location not found";
			ssp->status.infoLogLength = 33;
		}
	}
}

inline void Destroy(SpriteShaderProgram2D* ssp)
{
	glDeleteProgram(ssp->program);

	glDeleteSamplers(1, &ssp->textureSampler_SpriteSampler);
	ssp->textureSampler_SpriteSampler = 0;

	ssp->program = 0;
	ssp->status.compiled = false;
}

inline void SetShaderProgram(SpriteShaderProgram2D* ssp)
{
	glUseProgram(ssp->program);
}

inline void SetPCM(SpriteShaderProgram2D* ssp, mat3* PCM)
{
	glUniformMatrix3fv(ssp->location_PCM, 1, GL_FALSE, &(*PCM)[0][0]);
}

inline void SetSprite(SpriteShaderProgram2D* ssp, GLuint spriteTexture)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, spriteTexture);
}






inline void DrawAsPoints(size_t numberOfVertices)
{
	glDrawArrays(GL_POINTS, 0, numberOfVertices);
}

inline void DrawAsLines(size_t numberOfVertices)
{
	glDrawArrays(GL_LINES, 0, numberOfVertices);
}

inline void DrawAsTriangles(size_t numberOfVertices)
{
	glDrawArrays(GL_TRIANGLES, 0, numberOfVertices);
}



inline void DrawAsPointsIndexed(size_t numberOfIndices)
{
	glDrawElements(GL_POINTS, numberOfIndices, GL_UNSIGNED_INT, NULL);
}

inline void DrawAsLinesIndexed(size_t numberOfIndices)
{
	glDrawElements(GL_LINES, numberOfIndices, GL_UNSIGNED_INT, NULL);
}

inline void DrawAsTrianglesIndexed(size_t numberOfIndices)
{
	glDrawElements(GL_TRIANGLES, numberOfIndices, GL_UNSIGNED_INT, NULL);
}
