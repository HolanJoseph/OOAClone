#pragma once

#include "Types.h"
#include "Math.h"

#include "_SystemAPI.h"

#include "Util.h"

#include "RenderingShim_OpenGL.h"
#include "Transform.h"



struct Camera
{
	vec2 halfDim;

	vec2 position;
	F32  rotationAngle;
	F32  scale;

	Camera() : halfDim(), position(), rotationAngle(0), scale(1) {};

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



struct TextureHandle
{
	U32 loadNumber;
	U32 poolIndex;

	TextureHandle() : loadNumber(0), poolIndex(0) {};
	TextureHandle(U32 lN, U32 pI) : loadNumber(lN), poolIndex(pI) {};
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
	const char* k;
	TextureHandle v;

	ProcessedTexturesHashLink* next;
};

struct ProcessedTexturesHash
{
	U32 numberOfIndices;
	ProcessedTexturesHashLink** table;

	size_t length;

	ProcessedTexturesHash() : numberOfIndices(0), table(NULL), length(0) {};
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

inline U32 GetKeyIndex(ProcessedTexturesHash* pth, const char* key)
{
	U32 i = (String_HashFunction(key, pth->numberOfIndices) & 0x7fffffff) % pth->numberOfIndices;
	return i;
}

inline TextureHandle GetValue(ProcessedTexturesHash* pth, const char* key)
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

inline void AddKVPair(ProcessedTexturesHash* pth, const char* key, TextureHandle val)
{
	const char* keyCopy = Copy(key);

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

inline void RemoveKVPair(ProcessedTexturesHash* pth, const char* key)
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
			free((void*)curr->k);
			
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
			free((void*)curr->k);
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

			bool isDataLoaded;

			const char* filepath;
			Texture data;
		};

		PooledTexture* nextInFreeList;
	};
};

U32 texturePoolSize;
U32 numberOfTexturesAllocated;
U32 numberOfAllocatedBytes;
U32 maxNumberOfAllocatableBytes;
PooledTexture* firstFreePooledTexture;
PooledTexture* texturePool;
ProcessedTexturesHash processedTextures;



inline void StartUpTexturePool(U32 maxNumberOfTextures)
{
	numberOfAllocatedBytes = 0;
	maxNumberOfAllocatableBytes = U32_MAX; // NOTE: When LRU is actually implemented this will be changed.

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

	numberOfAllocatedBytes = 0;
	maxNumberOfAllocatableBytes = U32_MAX;
};

inline U32 GenerateLoadNumberTexturePool()
{
	U32 result;

	result = numberOfTexturesAllocated++;

	return result;
}

inline Texture* GetTexture(TextureHandle handle)
{
	Texture* result = NULL;


	result = &texturePool[handle.poolIndex].data;

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

inline bool IsTextureLoaded(TextureHandle th)
{
	bool result = false;

	PooledTexture* pt = &(texturePool[th.poolIndex]);
	result = pt->isDataLoaded;

	return result;
}

inline void LoadTexture(TextureHandle th)
{
	PooledTexture* pt = &(texturePool[th.poolIndex]);
	const char* filepath = pt->filepath;
	Initialize(&pt->data, filepath);
	pt->isDataLoaded = true;

	if (numberOfAllocatedBytes + (pt->data.width * pt->data.height) > maxNumberOfAllocatableBytes)
	{
		// LRU = GetLRU
		// destroy LRUs texture
		// set LRUs loaded to false
		// loop until we are under the maxNumberOfAllocatableBytes
	}
}

inline void UnloadTexture(TextureHandle th)
{

	PooledTexture* pt = &(texturePool[th.poolIndex]);
	const char* filepath = pt->filepath;
	U32 numberOfBytes = pt->data.width * pt->data.height;

	Destroy(&pt->data);

	pt->isDataLoaded = false;
	numberOfAllocatedBytes -= numberOfBytes;
}

inline TextureHandle AddToTexturePool(const char* filepath)
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
			pt->isDataLoaded = false;
			pt->filepath = Copy(filepath);
			result = TextureHandle(pt->loadNumber, pt->poolIndex);
			LoadTexture(result);

			AddKVPair(&processedTextures, filepath, result);
			Texture* t = GetTexture(result);
			numberOfAllocatedBytes += (t->width * t->height);
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

	vec2 window = GetClientWindowDimensions();

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

	StartUpTexturePool(300);


	/*AddToTexturePool("Assets/x60/Objects/boulder.bmp");
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
	TextureHandle th3 = AddToTexturePool("Assets/x60/Objects/pillar.bmp");*/
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

inline void DrawSprite(TextureHandle texture, vec2 spriteOffset, Transform transform, Camera* camera)
{
	bool loaded = IsTextureLoaded(texture);
	
	transform.position += spriteOffset;
	transform.scale /= 2.0f;

	if (loaded)
	{
		Texture* t = GetTexture(texture);
		DrawUVRectangle(t, transform, camera);
	}
	else
	{
		vec4 unloadedTextureColor = vec4(1.0f, 0.5f, 0.0f, 1.0f);
		DrawRectangle(vec2(0.5f, 0.5f), transform, unloadedTextureColor, camera);
	}
}

// NOTE: starts from upper left corner
inline void DrawUVRectangleScreenSpace(Texture* bitmap, vec2 position, vec2 dimensions)
{
	ViewportRectangle prevViewport = GetViewport();

	vec2 winDim = GetClientWindowDimensions();

	// Flip y so y increases down(0 at to, height at bottom)
	position.y = winDim.y - position.y - dimensions.y;

	SetViewport(position, dimensions);
	Camera ssCamera;
	ssCamera.halfDim = vec2(1, 1);
	ssCamera.position = vec2(0, 0);
	ssCamera.rotationAngle = 0.0f;
	ssCamera.scale = 1.0f;
	DrawUVRectangle(bitmap, Transform(), &ssCamera);

	SetViewport(prevViewport);
}