#pragma once

#include "Types.h"
#include "Math.h"

#include "DebugAPI.h"

#include "AssetLoading.h"

#include "Transform.h"

#include "glew/GL/glew.h"
#include <gl/GL.h>


struct Texture
{
	GLuint glTextureID;

	size_t width;  // Px
	size_t height; // Px
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
	glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, texture->width, texture->height);
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






// NOTE: For future implementations that require more complete foundations
// Locations are denoted by a 
//	type(mat4, mat3, vec4, etc...)
//	name("PCM", "CircleColor", "QuadColor", "SpriteSampler", etc...)


struct GLCompileStatus
{
	bool compiled;
	char* infoLog;
	I64 infoLogLength;
};
GLCompileStatus verifyShader(GLuint shader)
{
	GLCompileStatus result = { true, NULL, 0 };

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
	GLCompileStatus result = { true, NULL, 0 };

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
};
LSPResult LoadShaderProgram(char* vertexShaderFilename, char* fragmentShaderFilename)
{
	LSPResult result;

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	U64 vertexShaderFileSize = GetFileSize(vertexShaderFilename).fileSize;
	char* vertexShaderSource = (char *)malloc(sizeof(char)* vertexShaderFileSize);
	const GLint glSizeRead = ReadFile(vertexShaderFilename, vertexShaderSource, vertexShaderFileSize, 0).numberOfBytesRead;
	glShaderSource(vertexShader, 1, &vertexShaderSource, &glSizeRead);
	glCompileShader(vertexShader);
	GLCompileStatus vertexVerification = verifyShader(vertexShader);
	if (!vertexVerification.compiled)
	{
		result.program = 0;
		result.status = vertexVerification;
		return result; // NOTE: Remove early out.
	}
	free(vertexVerification.infoLog);

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	U64 fragmentShaderFileSize = GetFileSize(fragmentShaderFilename).fileSize;
	char* fragmentShaderSource = (char *)malloc(sizeof(char)* fragmentShaderFileSize);
	const GLint glFragmentShaderSize = ReadFile(fragmentShaderFilename, fragmentShaderSource, fragmentShaderFileSize, 0).numberOfBytesRead;
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, &glFragmentShaderSize);
	glCompileShader(fragmentShader);
	GLCompileStatus fragmentVerification = verifyShader(fragmentShader);
	if (!fragmentVerification.compiled)
	{
		result.program = 0;
		result.status = fragmentVerification;
		return result; // NOTE: Remove early out.
	}
	free(fragmentVerification.infoLog);

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
	free(programVerification.infoLog);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	result.program = shaderProgram;
	result.status.infoLog = NULL;
	result.status.infoLogLength = 0;

	return result;
}






struct BasicShaderProgram2D
{
	GLuint program;

	GLCompileStatus status;

	GLuint location_PCM;
	GLuint location_Color;
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

inline void Activate(BasicShaderProgram2D* bsp)
{
	glUseProgram(bsp->program);
}

inline void Deactivate(BasicShaderProgram2D* bsp)
{
	glUseProgram(NULL);
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

inline void Activate(SpriteShaderProgram2D* ssp)
{
	glUseProgram(ssp->program);
}

inline void Deactivate(SpriteShaderProgram2D* ssp)
{
	glUseProgram(NULL);
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