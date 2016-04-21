#pragma once

#include "Types.h"
#include "Math.h"

#include "DebugAPI.h"

#include "WindowAPI.h"

#include "AssetLoading.h"

#include "Transform.h"

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
	glViewport(newViewport.offset.x, newViewport.offset.y, newViewport.dimensions.x, newViewport.dimensions.y);
}





struct Camera
{
	vec2 halfDim;

	vec2 position;
	F32  rotationAngle;
	F32  scale;

	Camera()
	{
		halfDim = vec2(0, 0);

		position = vec2(0, 0);
		rotationAngle = 0;
		scale = 1;
	}

	void ResizeViewArea(vec2 halfDim)
	{
		this->halfDim = halfDim;
	}

	mat3 GetProjectionMatrix()
	{
		mat3 result = ScaleMatrix(1.0f / halfDim);
		return result;
	}
};






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
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, texture->width, texture->height); // 4 is 1????
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->width, texture->height, GL_RGBA, GL_UNSIGNED_BYTE, textureData.data);
	free(textureData.data);
}

inline void Initialize(Texture** sprites, char* spritesFolder, U32 numberOfSprites)
{
	*sprites = (Texture*)malloc(sizeof(Texture)* numberOfSprites);

	size_t pathLength = 0;
	char* path = spritesFolder;
	while (*path)
	{
		++pathLength;
		path = path + 1;
	}

	size_t filenameLength = 7;
	size_t newLength = pathLength + 1 + filenameLength;

	char* newPath = (char*)malloc(sizeof(char)* newLength);
	for (size_t i = 0; i < pathLength; ++i)
	{
		newPath[i] = spritesFolder[i];
	}

	newPath[pathLength] = '/';
	++pathLength;

	char** files;
	char fileType[] = ".bmp";
	size_t fileTypeLength = 5;
	files = (char**)malloc(sizeof(char*)* numberOfSprites);
	for (size_t i = 0; i < numberOfSprites; ++i)
	{
		files[i] = (char*)malloc(sizeof(char)* filenameLength);

		char* iAsString = I32ToString(i);
		size_t c = 0;
		while (iAsString[c])
		{
			files[i][c] = iAsString[c];
			++c;
		}
		free(iAsString);

		for (size_t j = 0; j < fileTypeLength; ++j)
		{
			files[i][c + j] = fileType[j];
		}
	}

	for (size_t x = 0; x < numberOfSprites; ++x)
	{
		size_t i = 0;
		for (; i < newLength && files[x][i]; ++i)
		{
			newPath[pathLength + i] = files[x][i];
		}
		newPath[pathLength + i] = '\0';

		Initialize((*sprites) + x, newPath);
	}

	for (size_t i = 0; i < numberOfSprites; ++i)
	{
		free(files[i]);
	}
	free(files);
	free(newPath);
}

inline void Destroy(Texture* texture)
{
	glDeleteTextures(1, &texture->glTextureID);

	texture->glTextureID = 0; // NOTE: This is probably redundant
	texture->width = 0;
	texture->height = 0;
}

struct TextureHandle
{
	U32 loadNumber;
	U32 poolIndex;
};

inline bool operator==(const TextureHandle& lhs, const TextureHandle& rhs)
{
	bool result = true;

	if (lhs.loadNumber != rhs.loadNumber)
	{
		result = false;
	}
	if (rhs.poolIndex != rhs.poolIndex)
	{
		result = false;
	}

	return result;
}

inline bool operator!=(const TextureHandle& lhs, const TextureHandle& rhs)
{
	bool result;

	result = !(lhs == rhs);

	return result;
}

struct ProcessedTexturesHashLink
{
	char* k;
	TextureHandle v;

	ProcessedTexturesHashLink* next;
};

struct ProcessedTexturesHash
{
	U32 numberOfIndices;
	ProcessedTexturesHashLink** table;

	size_t length;
};

inline void Initialize(ProcessedTexturesHash* pth, U32 numberOfIndices)
{
	pth->numberOfIndices = numberOfIndices;
	pth->table = (ProcessedTexturesHashLink**)malloc(sizeof(ProcessedTexturesHashLink*)* numberOfIndices);

	for (size_t i = 0; i < numberOfIndices; ++i)
	{
		pth->table[i] = NULL;
	}

	pth->length = 0;
}

extern U32 String_HashFunction(char*, U32);
inline U32 GetKeyIndex(ProcessedTexturesHash* pth, char* key)
{
	U32 i = (String_HashFunction(key, pth->numberOfIndices) & 0x7fffffff) % pth->numberOfIndices;
	return i;
}

inline TextureHandle GetValue(ProcessedTexturesHash* pth, char* key)
{
	TextureHandle result = TextureHandle();

	U32 keyIndex = GetKeyIndex(pth, key);
	ProcessedTexturesHashLink* curr = pth->table[keyIndex];
	while (curr != NULL)
	{
		bool match = Compare(key, curr->k);
		if (match)
		{
			result = curr->v;
			break;
		}

		curr = curr->next;
	}

	return result;
}

inline void AddKVPair(ProcessedTexturesHash* pth, char* key, TextureHandle val)
{
	char* keyCopy = Copy(key);

	// If the KV pair is not already in the hash table
	TextureHandle th = GetValue(pth, key);
	if (th == TextureHandle())
	{
		U32 i = (String_HashFunction(key, pth->numberOfIndices) & 0x7fffffff) % pth->numberOfIndices;

		ProcessedTexturesHashLink* newPair = (ProcessedTexturesHashLink*)malloc(sizeof(ProcessedTexturesHashLink));
		newPair->k = keyCopy;
		newPair->v = val;
		newPair->next = NULL;

		ProcessedTexturesHashLink* curr = pth->table[i];
		if (curr == NULL)
		{
			pth->table[i] = newPair;
		}
		else
		{
			while (curr->next != NULL)
			{
				curr = curr->next;
			}
			curr->next = newPair;
		}


		++pth->length;
	}
}

inline void RemoveKVPair(ProcessedTexturesHash* pth, char* key)
{
	U32 i = GetKeyIndex(pth, key);
	ProcessedTexturesHashLink* prev = NULL;
	ProcessedTexturesHashLink* curr = pth->table[i];
	while (curr != NULL)
	{
		bool match = Compare(key, curr->k);
		if (match)
		{
			--pth->length;
			free(curr->k);
			
			if (curr == pth->table[i])
			{
				// NOTE: Again what to do about TextureHandles
				pth->table[i] = curr->next;
			}
			else
			{
				prev->next = curr->next;
			}

			break;
		}

		prev = curr;
		curr = curr->next;
	}
}

inline size_t Length(ProcessedTexturesHash* pth)
{
	size_t result;

	result = pth->length;

	return result;
}

inline void Destroy(ProcessedTexturesHash* pth)
{
	for (size_t i = 0; i < pth->numberOfIndices; ++i)
	{


		ProcessedTexturesHashLink* curr = pth->table[i];
		ProcessedTexturesHashLink* next = curr;
		while (curr != NULL)
		{
			free(curr->k);
			// NOTE: What about the texturehandle?
				// Don't free it but how do we handle the texture in the pool?

			next = curr->next;
			free(curr);
			curr = next;
		}
	}
	free(pth->table);

	pth->table = NULL;
	pth->numberOfIndices = 0;
	pth->length = 0;
}


struct PooledTexture
{
	bool initialized;
	union 
	{
		struct  
		{
			U32 loadNumber;
			U32 poolIndex;

			Texture data;
		};

		PooledTexture* nextInFreeList;
	};
};

U32 texturePoolSize;
U32 numberOfTexturesAllocated;
PooledTexture* firstFreePooledTexture;
PooledTexture* texturePool;
ProcessedTexturesHash processedTextures;



inline void StartUpTexturePool(U32 maxNumberOfTextures)
{
	numberOfTexturesAllocated = 1; // NOTE: 0 will be used as a null state.
	texturePoolSize = maxNumberOfTextures;
	texturePool = (PooledTexture*)malloc(sizeof(PooledTexture) * texturePoolSize);
	firstFreePooledTexture = texturePool;

	for (U32 i = 0; i < texturePoolSize - 1; ++i)
	{
		texturePool[i].initialized = false;
		texturePool[i].nextInFreeList = &texturePool[i + 1];
	}
	texturePool[texturePoolSize - 1].initialized = false;
	texturePool[texturePoolSize - 1].nextInFreeList = NULL;

	Initialize(&processedTextures, maxNumberOfTextures / 2);

};

inline void ShutDownTexturePool()
{
	Destroy(&processedTextures);

	// NOTE: Instead of raw destroy here should probably call RemoveFromTexturePool
	for (U32 i = 0; i < texturePoolSize; ++i)
	{
		Destroy(&texturePool[i].data);
	}
	free(texturePool);

	texturePool = NULL;
	firstFreePooledTexture = NULL;
	numberOfTexturesAllocated = 0;
	texturePoolSize = 0;
};

inline U32 GenerateLoadNumberTexturePool()
{
	U32 result;

	result = numberOfTexturesAllocated++;

	return result;
}

inline bool IsValidTextureHandle(TextureHandle th)
{
	Assert(th.loadNumber < numberOfTexturesAllocated);
	Assert(th.poolIndex < texturePoolSize);

	bool result = false;

	if (texturePool[th.poolIndex].loadNumber == th.loadNumber)
	{
		result = true;
	}

	return result;
}

inline TextureHandle AddToTexturePool(char* filepath)
{
	TextureHandle result;

	TextureHandle th = GetValue(&processedTextures, filepath);
	if (th != TextureHandle())
	{
		result = th;
	}
	else
	{
		// If the first free is pointing to NULL all of our texture have been allocated
		if (firstFreePooledTexture == NULL)
		{
			// NOTE: FOR NOW just return that we cannot allocate anymore textures
			//			EVENTUALLY we want to remove the least recently used(or something)
			//			 texture and allocate the texture.
			result = TextureHandle();
		}
		else
		{
			PooledTexture* pt = firstFreePooledTexture;
			firstFreePooledTexture = pt->nextInFreeList;

			pt->initialized = true;
			pt->loadNumber = GenerateLoadNumberTexturePool();
			pt->poolIndex = pt - texturePool;
			Initialize(&pt->data, filepath);

			result = { pt->loadNumber, pt->poolIndex };
			AddKVPair(&processedTextures, filepath, result);
		}
	}

	return result;
};

// NOTE: Does not remove from processedTextures hash
inline void RemoveFromTexturePool(TextureHandle th)
{
	bool isValid = IsValidTextureHandle(th);
	if (isValid)
	{
		texturePool[th.poolIndex].loadNumber = 0;
		texturePool[th.poolIndex].poolIndex = 0;
		Destroy(&texturePool[th.poolIndex].data);

		texturePool[th.poolIndex].initialized = false;
		texturePool[th.poolIndex].nextInFreeList = firstFreePooledTexture;
		firstFreePooledTexture = &texturePool[th.poolIndex];
	}
};

inline Texture* GetTexture(TextureHandle handle)
{
	Texture* result = NULL;

	
	//result = &texturePool[handle.poolIndex].data;

	return result;
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





VertexData_Pos2D	debugPoint;
VertexData_Pos2D	debugLine;
VertexData_Pos2D	circleInPoint;
VertexData_Indexed_Pos2D_UV texturedQuad;

SpriteShaderProgram2D uvMapped2DProgram;
BasicShaderProgram2D singleColor2DProgram;
BasicShaderProgram2D solidColorCircleInPointProgram; // NOTE: More accurate name?
BasicShaderProgram2D singleColor3DProgram;

#define unitCircleSize_px 160.0f

inline vec2 WorldPointToScreenSpace(vec2 p, Camera* camera)
{
	vec2 result;

	mat3 P_projection = camera->GetProjectionMatrix();
	mat3 C_camera = TranslationMatrix(camera->position);
	mat3 M_model = mat3(1, 0, 0, 0, 1, 0, 0, 0, 1); // NOTE: again this can totally be dropped
	mat3 PCM = P_projection * inverse(C_camera) * M_model;
	vec3 p3 = vec3(p.x, p.y, 1.0f);
	vec2 pCamera = vec2(PCM * p3);

	vec2 window = GetWindowDimensions();

	result.x = (pCamera.x + 1) * (window.x / 2.0f);
	result.y = -((pCamera.y + 1) * (window.y / 2.0f));
	return result;
}

inline void InitializeRenderer()
{
	Initialize(&uvMapped2DProgram, "uvMapped2D.vert", "uvMapped2D.frag");
	Initialize(&singleColor2DProgram, "singleColor2D.vert", "singleColor2D.frag");
	Initialize(&solidColorCircleInPointProgram, "solidColorCircleInPoint.vert", "solidColorCircleInPoint.frag");
	Initialize(&singleColor3DProgram, "singleColor3D.vert", "singleColor3D.frag");



	// Circle
	vec2 circlePositions[1] =
	{
		vec2(0.0f, 0.0f)
	};
	Initialize(&circleInPoint, circlePositions, 1);

	vec2 debugPointPositions[1] =
	{
		vec2(0.0f, 0.0f)
	};
	Initialize(&debugPoint, debugPointPositions, 1);

	vec2 debugLinePositions[2] =
	{
		vec2(0.0f, 0.0f),
		vec2(1.0f, 1.0f)
	};
	Initialize(&debugLine, debugLinePositions, 2);

	const size_t tqNumVertices = 4;
	vec2 tqiPositions[tqNumVertices] =
	{
		vec2(-1.0f, -1.0f),
		vec2(1.0f, -1.0f),
		vec2(1.0f, 1.0f),
		vec2(-1.0f, 1.0f)
	};

	vec2 tqiUVs[tqNumVertices] =
	{
		vec2(0.0f, 0.0f),
		vec2(1.0f, 0.0f),
		vec2(1.0f, 1.0f),
		vec2(0.0f, 1.0f)
	};

	const size_t tqiNumIndices = 6;
	U32 tqiIndices[tqiNumIndices] = { 0, 1, 2, 0, 2, 3 };
	Initialize(&texturedQuad, tqiPositions, tqiUVs, tqNumVertices, tqiIndices, tqiNumIndices);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	StartUpTexturePool(10);


	AddToTexturePool("Assets/x60/Objects/boulder.bmp");
	TextureHandle th1 = AddToTexturePool("Assets/x60/Objects/boulder_Dark.bmp");
	AddToTexturePool("Assets/x60/Objects/boulder_Graveyard.bmp");
	AddToTexturePool("Assets/x60/Objects/coconut.bmp");
	AddToTexturePool("Assets/x60/Objects/dirt.bmp");
	AddToTexturePool("Assets/x60/Objects/dirt_Dark.bmp");

	TextureHandle th4 = AddToTexturePool("Assets/x60/Objects/boulder.bmp");

	RemoveFromTexturePool(th1);

	AddToTexturePool("Assets/x60/Objects/flower_Blue.bmp");
	AddToTexturePool("Assets/x60/Objects/flower_Red.bmp");
	AddToTexturePool("Assets/x60/Objects/fountain.bmp");
	AddToTexturePool("Assets/x60/Objects/gravestone.bmp");

	TextureHandle th2 = AddToTexturePool("Assets/x60/Objects/jelly.bmp");
	TextureHandle th3 = AddToTexturePool("Assets/x60/Objects/pillar.bmp");
}

inline void ShutdownRenderer()
{
	ShutDownTexturePool();

	Destroy(&uvMapped2DProgram);
	Destroy(&singleColor2DProgram);
	Destroy(&solidColorCircleInPointProgram);
	Destroy(&singleColor3DProgram);
}

inline void DrawPoint(vec2 p, F32 pointSize, vec4 color, Camera* camera)
{
	mat3 P_projection = camera->GetProjectionMatrix();
	mat3 C_camera = TranslationMatrix(camera->position);
	mat3 M_model = mat3(1, 0, 0, 0, 1, 0, 0, 0, 1);
	mat3 PCM = P_projection * inverse(C_camera) * M_model;

	SetShaderProgram(&singleColor2DProgram);
	SetPCM(&singleColor2DProgram, &PCM);
	SetColor(&singleColor2DProgram, &color);

	glPointSize(pointSize);

	RebufferVertexData(&debugPoint, &p, 1);
	SetVertexData(&debugPoint);
	DrawAsPoints(debugPoint.numberOfVertices);
	ClearVertexData();

	ClearShaderProgram();
}

inline void DrawLine(vec2 a, vec2 b, vec4 color, Camera* camera)
{
	mat3 P_projection = camera->GetProjectionMatrix();
	mat3 C_camera = TranslationMatrix(camera->position);
	mat3 M_model = mat3(1, 0, 0, 0, 1, 0, 0, 0, 1);
	mat3 PCM = P_projection * inverse(C_camera) * M_model;

	SetShaderProgram(&singleColor2DProgram);
	SetPCM(&singleColor2DProgram, &PCM);
	SetColor(&singleColor2DProgram, &color);

	vec2 linePositions[2] = { a, b };
	RebufferVertexData(&debugLine, linePositions, 2);
	SetVertexData(&debugLine);
	DrawAsLines(debugLine.numberOfVertices);
	ClearVertexData();

	ClearShaderProgram();
}

inline void DrawRectangle(vec2 halfDim, Transform transform, vec4 color, Camera* camera)
{
	mat3 P_projection = camera->GetProjectionMatrix();
	mat3 C_camera = TranslationMatrix(camera->position);
	mat3 M_model = transform.LocalToWorldTransform() * ScaleMatrix(halfDim);
	mat3 PCM = P_projection * inverse(C_camera) * M_model;

	SetShaderProgram(&singleColor2DProgram);
	SetPCM(&singleColor2DProgram, &PCM);
	SetColor(&singleColor2DProgram, &color);

	SetVertexData(&texturedQuad);
	DrawAsTrianglesIndexed(texturedQuad.numberOfIndices);
	ClearVertexData();

	ClearShaderProgram();
}

inline void DrawCircle(F32 radius, Transform transform, vec4 color, Camera* camera)
{
	mat3 P_projection = camera->GetProjectionMatrix();
	mat3 C_camera = TranslationMatrix(camera->position);
	mat3 M_model = transform.LocalToWorldTransform();
	mat3 PCM = P_projection * inverse(C_camera) * M_model;

	SetShaderProgram(&solidColorCircleInPointProgram);
	SetPCM(&solidColorCircleInPointProgram, &PCM);
	SetColor(&solidColorCircleInPointProgram, &color);

	glPointSize(transform.scale.x * radius * unitCircleSize_px);

	SetVertexData(&circleInPoint);
	DrawAsPoints(1);
	ClearVertexData();

	ClearShaderProgram();
}



// World Space
// - offset == outset
// + offset == inset
inline void DrawRectangleOutline(vec2 upperLeft, vec2 dimensions, vec4 color, Camera* camera, F32 offset = 0.0f)
{
	upperLeft.x += offset;
	upperLeft.y -= offset;
	
	dimensions.x -= offset * 2;
	dimensions.y -= offset * 2;

	vec2 x = vec2(dimensions.x, 0);
	vec2 y = vec2(0, -dimensions.y);
	vec2 xy = vec2(dimensions.x, -dimensions.y);
	DrawLine(upperLeft, upperLeft + x, color, camera);
	DrawLine(upperLeft, upperLeft + y, color, camera);
	DrawLine(upperLeft + y, upperLeft + xy, color, camera);
	DrawLine(upperLeft + x, upperLeft + xy, color, camera);
}

inline void DrawUVRectangle(Texture* texture, Transform transform, Camera* camera)
{
	mat3 P_projection = camera->GetProjectionMatrix();
	mat3 C_camera = TranslationMatrix(camera->position);
	mat3 M_model = transform.LocalToWorldTransform(); 
	mat3 PCM = P_projection * inverse(C_camera) * M_model;

	SetShaderProgram(&uvMapped2DProgram);
	SetPCM(&uvMapped2DProgram, &PCM);
	SetSprite(&uvMapped2DProgram, texture->glTextureID);

	SetVertexData(&texturedQuad);
	DrawAsTrianglesIndexed(texturedQuad.numberOfIndices);
	ClearVertexData();

	ClearShaderProgram();
}

inline void DrawSprite(Texture* texture, vec2 spriteOffset, Transform transform, Camera* camera)
{
	transform.position += spriteOffset;
	transform.scale /= 2.0f;
	DrawUVRectangle(texture, transform, camera);
}

// NOTE: starts from upper left corner
inline void DrawUVRectangleScreenSpace(Texture* bitmap, vec2 position, vec2 dimensions)
{
	ViewportRectangle prevViewport = GetViewport();

	vec2 winDim = GetWindowDimensions();

	// Flip y so y increases down(0 at to, height at bottom)
	position.y = winDim.y - position.y - dimensions.y;

	SetViewport({ position, dimensions });
	Camera ssCamera;
	ssCamera.halfDim = vec2(1, 1);
	ssCamera.position = vec2(0, 0);
	ssCamera.rotationAngle = 0.0f;
	ssCamera.scale = 1.0f;
	DrawUVRectangle(bitmap, Transform(), &ssCamera);

	SetViewport(prevViewport);
}