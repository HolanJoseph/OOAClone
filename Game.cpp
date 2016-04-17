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

#include "Renderer.h"

#include "AssetLoading.h"
#include "CollisionDetection.h"
#include "CollisionDetection2D.h"

#include "CollisionDetection_Tests.h"
#include "StringAPI_Tests.h"

#include "CollisionDetection_Visualization.h"

#include "CollisionDetection2D_Applet.h"

#include <vector>

//#define RUN_UNIT_TESTS 1

//#define COLLISION2DAPPLET 1
#define GAME 1

struct Entity
{
	Transform transform;

	// Renderable
	vec2 spriteOffset;

	Texture sprite;
	Texture* sprites;

	bool spriteIsAnimated;
	U32 numberOfFrames = 4;
	F32 animationTime = 2.0;
	F32 elapsedTime = 0;
};
typedef std::vector<Entity> Entities;
Entities entities;

inline void InitializeAnimatedSprite(Texture** sprites, U32 numberOfSprites, char* spritesFolder)
{
	*sprites = (Texture*)malloc(sizeof(Texture) * numberOfSprites);

	size_t pathLength = 0;
	char* path = spritesFolder;
	while (*path)
	{
		++pathLength;
		path = path + 1;
	}
	
	size_t filenameLength = 7;
	size_t newLength = pathLength + 1 + filenameLength;
	
	char* newPath = (char*)malloc(sizeof(char) * newLength);
	for (size_t i = 0; i < pathLength; ++i)
	{
		newPath[i] = spritesFolder[i];
	}

	newPath[pathLength] = '/';
	++pathLength;

	char** files;
	char fileType[] = ".bmp";
	size_t fileTypeLength = 5;
	files = (char**)malloc(sizeof(char*) * numberOfSprites);
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

struct Chunk
{
	vec2 position;

	/*
	enum CameraStyle
	{
		CS_Static,
		CS_Scolling
	};

	enum CameraView
	{
		VA_TopDown,
		VA_Side
	};
	*/

	Entities entities;
};
const size_t numberOfHorizontalChunks = 14;
const size_t numberOfVerticalChunks = 14;
const I32 chunkWidth = 10;
const I32 chunkHeight = 8;
typedef Chunk Chunks[numberOfHorizontalChunks][numberOfVerticalChunks];
Chunks chunks;

Camera camera;

Texture guiPanel;
Entity te;


struct SpriteFileTableLink
{
	char* k;
	char* v;

	SpriteFileTableLink* next;
};

struct SpriteFileTable
{
	U32 numberOfIndices;
	SpriteFileTableLink* table;
};

inline void Initialize(SpriteFileTable* sft, U32 numberOfIndices)
{
	sft->numberOfIndices = numberOfIndices;
	sft->table = (SpriteFileTableLink*)malloc(sizeof(SpriteFileTableLink)* numberOfIndices);

	for (size_t i = 0; i < numberOfIndices; ++i)
	{
		sft->table[i] = { 0 };
	}
}

inline U32 SpriteFileTable_HashFunction(char* key, U32 numberOfIndices)
{
	U32 hash = 0;

	char* keyI = key;
	while (*keyI)
	{
		++keyI;
		// NOTE: How is this better ????
		//result += (U32)*keyI % numberOfIndices;
		hash = (31 * hash + (*keyI)) % numberOfIndices;
	}

	return hash;
}

inline char* GetValue(SpriteFileTable* sft, char* key)
{
	char* result = NULL;

	// NOTE: How is this better?
	U32 i = (SpriteFileTable_HashFunction(key, sft->numberOfIndices) & 0x7fffffff) % sft->numberOfIndices;
	SpriteFileTableLink* l = &sft->table[i];

	if (l->k == NULL)
	{
		return result;
	}

	while (l)
	{
		// if key and l->key match up to key length
		char* k = key;
		char* lk = l->k;
		bool match = true;
		while (*k && *lk)
		{
			if (*k != *lk)
			{
				match = false;
				break;
			}

			++k;
			++lk;
		}

		// Make sure lk is not longer than k
		if (match && (*k != NULL || *lk != NULL))
		{
			match = false;
		}

		if (match)
		{
			result = l->v;
			break;
		}

		l = l->next;
	}

	return result;
}

inline void AddKVPair(SpriteFileTable* sft, char* key, char* value)
{
	size_t keyLength = 0;
	char* keyI = key;
	while (*keyI)
	{
		++keyI;
		++keyLength;
	}
	char* keyCopy = (char*)malloc(sizeof(char) * (keyLength + 1));
	for (size_t i = 0; i < keyLength; ++i)
	{
		keyCopy[i] = key[i];
	}
	keyCopy[keyLength] = '\0';

	size_t valueLength = 0;
	char* valueI = value;
	while (*valueI)
	{
		++valueI;
		++valueLength;
	}
	char* valueCopy = (char*)malloc(sizeof(char) * (valueLength + 1));
	for (size_t i = 0; i < valueLength; ++i)
	{
		valueCopy[i] = value[i];
	}
	valueCopy[valueLength] = '\0';

	if (GetValue(sft, key) == NULL)
	{

		U32 i = (SpriteFileTable_HashFunction(key, sft->numberOfIndices) & 0x7fffffff) % sft->numberOfIndices;
		if (sft->table[i].k == NULL)
		{
			sft->table[i].k = keyCopy;
			sft->table[i].v = valueCopy;
			sft->table[i].next = NULL; // NOTE: For the consistency.
		}
		else
		{
			SpriteFileTableLink* l = &sft->table[i];
			while (l->next)
			{
				l = l->next;
			}
			l->next = (SpriteFileTableLink*)malloc(sizeof(SpriteFileTableLink));
			l->next->k = keyCopy;
			l->next->v = valueCopy;
			l->next->next = NULL;
		}

	}
}

inline void Destroy(SpriteFileTable* sft)
{
	for (size_t i = 0; i < sft->numberOfIndices; ++i)
	{
		SpriteFileTableLink* lPrev = NULL;
		SpriteFileTableLink* l = sft->table[i].next;
		while (l != NULL)
		{
			if (l->k)
			{
				free(l->k);
			}
			if (l->v)
			{
				free(l->v);
			}
			lPrev = l;
			l = l->next;

			free(lPrev);
		}

		free(sft->table[i].k);
		free(sft->table[i].v);
	}
	free(sft->table);

	sft->table = NULL;
	sft->numberOfIndices = 0;
}
SpriteFileTable spriteFiles;


inline void InitChunk_0_0()
{
	Chunk* chunk = &chunks[0][0];

	Texture t;
	Transform tr;

	Entity e;
	Initialize(&e.sprite, "Assets/x60/Objects/tree_Palm.bmp");
	e.transform.position = vec2(2,5);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_Palm.bmp");
	e.transform.position = vec2(4, 4);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_Palm.bmp");
	e.transform.position = vec2(8, 3);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_Palm.bmp");
	e.transform.position = vec2(10, 3);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(3.5, 6.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(4.5, 6.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(6.5, 6.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(4.5, 7.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(5.5, 7.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(6.5, 7.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(9.5, 7.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_LargeWater.bmp");
	e.transform.position = vec2(3.5, 7.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_LargeWater.bmp");
	e.transform.position = vec2(5.5, 6.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);
}

inline void InitChunk_0_1()
{
	Chunk* chunk = &chunks[0][1];

	Entity e;
	Initialize(&e.sprite, "Assets/x60/Objects/rocks_LargeWater.bmp");
	e.transform.position = vec2(6.5, .5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_LargeWater.bmp");
	e.transform.position = vec2(4.5, 6.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_LargeWater.bmp");
	e.transform.position = vec2(1.5, 7.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_LargeWater.bmp");
	e.transform.position = vec2(7.5, 7.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(3.5, .5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(4.5, .5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(5.5, .5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(9.5, .5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(9.5, 5.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(9.5, 6.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(9.5, 7.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(8.5, 7.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(6.5, 7.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(5.5, 7.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(4.5, 7.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(3.5, 7.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(2.5, 7.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);
}

inline void InitChunk_0_2()
{
	Chunk* chunk = &chunks[0][2];

	Entity e;
	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(1.5, .5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_LargeWater.bmp");
	e.transform.position = vec2(2.5, .5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(3.5, .5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_LargeWater.bmp");
	e.transform.position = vec2(4.5, .5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(5.5, .5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(6.5, .5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(7.5, .5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_LargeWater.bmp");
	e.transform.position = vec2(8.5, .5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(9.5, .5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(2.5, 1.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(3.5, 1.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/jelly.bmp");
	e.transform.position = vec2(1.5, 2.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/jelly.bmp");
	e.transform.position = vec2(2.5,3.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/jelly.bmp");
	e.transform.position = vec2(7.5, 3.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/jelly.bmp");
	e.transform.position = vec2(1.5, 4.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/jelly.bmp");
	e.transform.position = vec2(2.5, 4.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/jelly.bmp");
	e.transform.position = vec2(7.5, 4.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/jelly.bmp");
	e.transform.position = vec2(8.5, 4.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/jelly.bmp");
	e.transform.position = vec2(9.5, 4.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/jelly.bmp");
	e.transform.position = vec2(6.5, 5.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/jelly.bmp");
	e.transform.position = vec2(9.5, 5.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(1.5, 6.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(8.5, 6.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(9.5, 6.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(1.5, 7.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(8.5, 7.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_LargeWater.bmp");
	e.transform.position = vec2(9.5, 7.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);
}

inline void InitChunk_0_3()
{
	Chunk* chunk = &chunks[0][3];

	Entity e;
	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(1.5, .5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(8.5, .5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(9.5, .5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(1.5, 1.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(9.5, 1.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(8.5, 2.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_LargeWater.bmp");
	e.transform.position = vec2(9.5, 2.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_Palm.bmp");
	e.transform.position = vec2(4, 5);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_Palm.bmp");
	e.transform.position = vec2(2, 6);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);
}

inline void InitChunk_0_4()
{
	Chunk* chunk = &chunks[0][4];

	Entity e;

	Initialize(&e.sprite, "Assets/x60/Objects/tree_generic.bmp");
	e.transform.position = vec2(0, 2);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/treePlot.bmp");
	e.transform.position = vec2(1.5, 1.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_generic.bmp");
	e.transform.position = vec2(5, 3);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_generic.bmp");
	e.transform.position = vec2(0, 4);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_generic.bmp");
	e.transform.position = vec2(2, 4);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_generic.bmp");
	e.transform.position = vec2(4, 4);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_generic.bmp");
	e.transform.position = vec2(1, 5);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_generic.bmp");
	e.transform.position = vec2(0, 6);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_generic.bmp");
	e.transform.position = vec2(0, 8);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_generic.bmp");
	e.transform.position = vec2(10, 8);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);
}

inline void InitChunk_0_5()
{
	Chunk* chunk = &chunks[0][5];

	Entity e;
	Initialize(&e.sprite, "Assets/x60/Objects/tree_generic.bmp");
	e.transform.position = vec2(0, 2);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_generic.bmp");
	e.transform.position = vec2(0, 4);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_generic.bmp");
	e.transform.position = vec2(0, 6);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_generic.bmp");
	e.transform.position = vec2(0, 8);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_generic.bmp");
	e.transform.position = vec2(3, 4);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_generic.bmp");
	e.transform.position = vec2(8, 4);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/weed.bmp");
	e.transform.position = vec2(5.5, 1.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);
	
	Initialize(&e.sprite, "Assets/x60/Objects/weed.bmp");
	e.transform.position = vec2(4.5, 2.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/weed.bmp");
	e.transform.position = vec2(6.5, 2.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/weed.bmp");
	e.transform.position = vec2(5.5, 6.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/weed.bmp");
	e.transform.position = vec2(4.5, 5.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/weed.bmp");
	e.transform.position = vec2(6.5, 5.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_generic.bmp");
	e.transform.position = vec2(10, 8);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);
}

inline void InitChunk_0_6()
{
	Chunk* chunk = &chunks[0][6];

	Entity e;

	Initialize(&e.sprite, "Assets/x60/Objects/tree_generic.bmp");
	e.transform.position = vec2(0, 2);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_generic.bmp");
	e.transform.position = vec2(0, 4);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_generic.bmp");
	e.transform.position = vec2(0, 6);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_generic.bmp");
	e.transform.position = vec2(1, 8);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_generic.bmp");
	e.transform.position = vec2(3, 8);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_generic.bmp");
	e.transform.position = vec2(5, 8);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_generic.bmp");
	e.transform.position = vec2(7, 8);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_generic.bmp");
	e.transform.position = vec2(9, 8);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_generic.bmp");
	e.transform.position = vec2(4, 5);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/tree_generic.bmp");
	e.transform.position = vec2(6, 3);
	e.transform.scale = vec2(2);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/weed.bmp");
	e.transform.position = vec2(3.5, 3.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/weed.bmp");
	e.transform.position = vec2(4.5, 3.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/weed.bmp");
	e.transform.position = vec2(5.5, 4.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/weed.bmp");
	e.transform.position = vec2(6.5, 4.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);
}

inline void InitChunk_0_7()
{
	Chunk* chunk = &chunks[0][7];
}

inline void InitChunk_0_8()
{
	Chunk* chunk = &chunks[0][8];

	Entity e;
	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(6.5, 6.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

}

inline void InitChunk_0_9()
{
	Chunk* chunk = &chunks[0][9];

	Entity e;
	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(6.5, 1.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);
}

inline void InitChunk_0_10()
{
	Chunk* chunk = &chunks[0][10];

	Entity e;
	Initialize(&e.sprite, "Assets/x60/Objects/treePlot.bmp");
	e.transform.position = vec2(2.5, 2.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/weed.bmp");
	e.transform.position = vec2(3.5, 2.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/weed.bmp");
	e.transform.position = vec2(2.5, 3.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/weed.bmp");
	e.transform.position = vec2(3.5, 3.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/dirt.bmp");
	e.transform.position = vec2(7.5, 1.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/dirt.bmp");
	e.transform.position = vec2(8.5, 1.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/dirt.bmp");
	e.transform.position = vec2(9.5, 1.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);
}

inline void InitChunk_0_11()
{
	Chunk* chunk = &chunks[0][11];

	Entity e;
	Initialize(&e.sprite, "Assets/x60/Objects/dirt.bmp");
	e.transform.position = vec2(1.5, 3.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/dirt.bmp");
	e.transform.position = vec2(3.5, 3.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/dirt.bmp");
	e.transform.position = vec2(4.5, 3.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/dirt.bmp");
	e.transform.position = vec2(1.5, 4.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/dirt.bmp");
	e.transform.position = vec2(2.5, 4.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/portal.bmp");
	e.transform.position = vec2(4.5, 5.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(8.5, 7.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

	Initialize(&e.sprite, "Assets/x60/Objects/rocks_SmallWater.bmp");
	e.transform.position = vec2(9.5, 7.5);
	e.transform.scale = vec2(1);
	chunk->entities.push_back(e);

}

inline void InitChunk_0_12()
{
	Chunk* chunk = &chunks[0][12];

	Entity e;

}

inline void InitChunk_0_13()
{

	Chunk* chunk = &chunks[0][13];
}



inline void InitChunk_1_13()
{
	Chunk* chunk = &chunks[1][13];

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[0].sprite, "Assets/x60/Objects/pit.bmp");
	chunk->entities[0].transform.position = vec2(8.5, 6.5);
}

inline void InitChunk_2_13()
{
	Chunk* chunk = &chunks[2][13];

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[0].sprite, "Assets/x60/Objects/house_Red.bmp");
	chunk->entities[0].transform.position = vec2(2.5, 6.5);
	chunk->entities[0].transform.scale = vec2(3);

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[1].sprite, "Assets/x60/Objects/flower_Blue.bmp");
	chunk->entities[1].transform.position = vec2(1.5, 4.5);

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[2].sprite, "Assets/x60/Objects/flower_Blue.bmp");
	chunk->entities[2].transform.position = vec2(3.5, 4.5);

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[3].sprite, "Assets/x60/Objects/flower_Blue.bmp");
	chunk->entities[3].transform.position = vec2(4.5, 4.5);

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[4].sprite, "Assets/x60/Objects/flower_Blue.bmp");
	chunk->entities[4].transform.position = vec2(4.5, 5.5);

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[5].sprite, "Assets/x60/Objects/flower_Blue.bmp");
	chunk->entities[5].transform.position = vec2(4.5, 6.5);

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[6].sprite, "Assets/x60/Objects/tree_Generic.bmp");
	chunk->entities[6].transform.position = vec2(10, 0);
	chunk->entities[6].transform.scale = vec2(2);
}

inline void InitChunk_3_13()
{
	Chunk* chunk = &chunks[3][13];

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[0].sprite, "Assets/x60/Objects/dungeon_SkullDungeon.bmp");
	chunk->entities[0].transform.position = vec2(5, 6);
	chunk->entities[0].transform.scale = vec2(6, 4);

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[1].sprite, "Assets/x60/Objects/tree_generic.bmp");
	chunk->entities[1].transform.position = vec2(0, 0);
	chunk->entities[1].transform.scale = vec2(2);

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[2].sprite, "Assets/x60/Objects/tree_generic.bmp");
	chunk->entities[2].transform.position = vec2(5, 0);
	chunk->entities[2].transform.scale = vec2(2);

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[3].sprite, "Assets/x60/Objects/tree_generic.bmp");
	chunk->entities[3].transform.position = vec2(10, 0);
	chunk->entities[3].transform.scale = vec2(2);
}

inline void InitChunk_4_13()
{
	Chunk* chunk = &chunks[4][13];

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[0].sprite, "Assets/x60/Objects/tree_generic.bmp");
	chunk->entities[0].transform.position = vec2(0, 0);
	chunk->entities[0].transform.scale = vec2(2);

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[1].sprite, "Assets/x60/Objects/flower_Blue.bmp");
	chunk->entities[1].transform.position = vec2(5.5, 6.5);

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[2].sprite, "Assets/x60/Objects/flower_Blue.bmp");
	chunk->entities[2].transform.position = vec2(5.5, 5.5);

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[3].sprite, "Assets/x60/Objects/flower_Blue.bmp");
	chunk->entities[3].transform.position = vec2(5.5, 4.5);

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[4].sprite, "Assets/x60/Objects/flower_Blue.bmp");
	chunk->entities[4].transform.position = vec2(6.5, 4.5);

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[5].sprite, "Assets/x60/Objects/flower_Blue.bmp");
	chunk->entities[5].transform.position = vec2(8.5, 4.5);

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[6].sprite, "Assets/x60/Objects/house_Red.bmp");
	chunk->entities[6].transform.position = vec2(7.5,  6.5);
	chunk->entities[6].transform.scale = vec2(3);
}

inline void InitChunk_5_13()
{
	Chunk* chunk = &chunks[5][13];

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[0].sprite, "Assets/x60/Objects/treePlot.bmp");
	chunk->entities[0].transform.position = vec2(6.5, 5.5);
}

inline void InitChunk_6_13()
{
	Chunk* chunk = &chunks[6][13];

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[0].sprite, "Assets/x60/Objects/tree_generic.bmp");
	chunk->entities[0].transform.position = vec2(4, 4);
	chunk->entities[0].transform.scale = vec2(2);

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[1].sprite, "Assets/x60/Objects/flower_Red.bmp");
	chunk->entities[1].transform.position = vec2(2.5, 4.5);

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[2].sprite, "Assets/x60/Objects/flower_Red.bmp");
	chunk->entities[2].transform.position = vec2(4.5, 5.5);

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[3].sprite, "Assets/x60/Objects/flower_Red.bmp");
	chunk->entities[3].transform.position = vec2(5.5, 3.5);

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[4].sprite, "Assets/x60/Objects/weed.bmp");
	chunk->entities[4].transform.position = vec2(2.5, 3.5);

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[5].sprite, "Assets/x60/Objects/weed.bmp");
	chunk->entities[5].transform.position = vec2(3.5, 5.5);

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[6].sprite, "Assets/x60/Objects/weed.bmp");
	chunk->entities[6].transform.position = vec2(5.5, 4.5);
}

inline void InitChunk_7_13()
{
	Chunk* chunk = &chunks[7][13];

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[0].sprite, "Assets/x60/Objects/tree_generic.bmp");
	chunk->entities[0].transform.position = vec2(6, 3);
	chunk->entities[0].transform.scale = vec2(2);

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[1].sprite, "Assets/x60/Objects/flower_Red.bmp");
	chunk->entities[1].transform.position = vec2(4.5, 3.5);

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[2].sprite, "Assets/x60/Objects/flower_Red.bmp");
	chunk->entities[2].transform.position = vec2(4.5, 2.5);

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[3].sprite, "Assets/x60/Objects/flower_Red.bmp");
	chunk->entities[3].transform.position = vec2(5.5, 4.5);

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[4].sprite, "Assets/x60/Objects/flower_Red.bmp");
	chunk->entities[4].transform.position = vec2(6.5, 4.5);

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[5].sprite, "Assets/x60/Objects/flower_Red.bmp");
	chunk->entities[5].transform.position = vec2(7.5, 3.5);

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[6].sprite, "Assets/x60/Objects/flower_Red.bmp");
	chunk->entities[6].transform.position = vec2(7.5, 2.5);
}

inline void InitChunk_8_13()
{
}

inline void InitChunk_9_13()
{
	Chunk* chunk = &chunks[9][13];

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[0].sprite, "Assets/x60/Objects/dungeon_MoblinCastle.bmp");
	chunk->entities[0].transform.position = vec2(5.5, 5);
	chunk->entities[0].transform.scale = vec2(5, 4);
}

inline void InitChunk_10_13()
{
}

inline void InitChunk_11_13()
{
}

inline void InitChunk_12_13()
{
}

inline void InitChunk_13_13()
{
	Chunk* chunk = &chunks[13][13];

	chunk->entities.push_back(Entity());
	Initialize(&chunk->entities[0].sprite, "Assets/x60/Objects/heartPiece.bmp");
	chunk->entities[0].transform.position = vec2(7.5, 4.5);
}

inline void InitChunks()
{
	InitChunk_0_0();
	InitChunk_0_1();
	InitChunk_0_2();
	InitChunk_0_3();
	InitChunk_0_4();
	InitChunk_0_5();
	InitChunk_0_6();
	InitChunk_0_7();
	InitChunk_0_8();
	InitChunk_0_9();
	InitChunk_0_10();
	InitChunk_0_11();
	InitChunk_0_12();

	InitChunk_0_13();
	InitChunk_1_13();
	InitChunk_2_13();
	InitChunk_3_13();
	InitChunk_4_13();
	InitChunk_5_13();
	InitChunk_6_13();
	InitChunk_7_13();
	InitChunk_8_13();
	InitChunk_9_13();
	InitChunk_10_13();
	InitChunk_11_13();
	InitChunk_12_13();
	InitChunk_13_13();

}



void InitScene()
{
	SetWindowTitle("Oracle of Ages Clone");
	SetWindowDimensions(vec2(600, 540));
	SetViewport({ vec2(0, 0), vec2(600, 540) });
	SetClearColor(vec4(0.32f, 0.18f, 0.66f, 0.0f));

	Initialize(&guiPanel, "Assets/x60/Objects/guiPanel.bmp");

	

	Initialize(&spriteFiles, 10);

	AddKVPair(&spriteFiles, "weed", "Assets/x60/Objects/weed.bmp");
	AddKVPair(&spriteFiles, "treePlot", "Assets/x60/Objects/treePlot.bmp");
	AddKVPair(&spriteFiles, "tree_Generic", "Assets/x60/Objects/tree_generic.bmp");
	AddKVPair(&spriteFiles, "tree_Palm", "Assets/x60/Objects/tree_Palm.bmp");
	AddKVPair(&spriteFiles, "tree_Spooky", "Assets/x60/Objects/tree_Spooky.bmp");

	char* w = GetValue(&spriteFiles, "weed");
	char* tp = GetValue(&spriteFiles, "tree_Palm");



	camera.halfDim = vec2(5, 4); // *2.0f;
	camera.position = vec2(0, -10);
	camera.rotationAngle = 0.0f;
	camera.scale = 1.0f;

	vec2 chunkPos = vec2(-(numberOfHorizontalChunks / 2.0f) * chunkWidth, -(numberOfVerticalChunks / 2.0f) * chunkHeight);
	for (size_t y = 0; y < numberOfVerticalChunks; ++y)
	{
		for (size_t x = 0; x < numberOfHorizontalChunks; ++x)
		{
			chunks[x][y].position = chunkPos;
			chunkPos.x = chunkPos.x + chunkWidth;
		}
		chunkPos.x = -70;
		chunkPos.y = chunkPos.y + chunkHeight;
	}

	InitChunks();

	camera.position = chunks[0][0].position + vec2(chunkWidth / 2.0f, chunkHeight / 2.0f);

	// Add all chunks to entities
	for (size_t y = 0; y < numberOfVerticalChunks; ++y)
	{
		for (size_t x = 0; x < numberOfHorizontalChunks; ++x)
		{
			for (size_t i = 0; i < chunks[x][y].entities.size(); ++i)
			{
				Entity e = chunks[x][y].entities[i];
				e.transform.position = chunks[x][y].position + vec2(e.transform.position.x, e.transform.position.y);

				entities.push_back(e);
			}
		}
	}

	te.transform.position = entities.back().transform.position;
	te.spriteIsAnimated = true;
	te.numberOfFrames = 4;
	te.animationTime = 2.0f;
	InitializeAnimatedSprite(&te.sprites, te.numberOfFrames, "Assets/x60/Objects/water_Deep");
}


bool GameInit()
{
	// Initialize game sub systems.
	InitializeRenderer();



#ifdef RUN_UNIT_TESTS
	CollisionTestsRectRect2();
#endif
	

	// Dispatch applet type.
#ifdef COLLISION2DAPPLET
	InitializeCollisionDetection2DApplet();
#elif GAME
	InitScene();
#endif

	return true;
}




void UpdateGamestate_PrePhysics(F32 dt)
{
	if (GetKeyDown(KeyCode_Equal))
	{
		camera.halfDim /= 2.0f;
	}
	if (GetKeyDown(KeyCode_Minus))
	{
		camera.halfDim *= 2.0f;
	}
	if (GetKeyDown(KeyCode_A))
	{
		camera.position.x -= chunkWidth;
	}
	if (GetKeyDown(KeyCode_D))
	{
		camera.position.x += chunkWidth;
	}
	if (GetKeyDown(KeyCode_W))
	{
		camera.position.y += chunkHeight;
	}
	if (GetKeyDown(KeyCode_S))
	{
		camera.position.y -= chunkHeight;
	}
	camera.position.x = ClampRange(camera.position.x, chunks[0][0].position.x + chunkWidth / 2.0f, chunks[13][0].position.x + chunkWidth / 2.0f);
	camera.position.y = ClampRange(camera.position.y, chunks[0][0].position.y + chunkHeight / 2.0f, chunks[0][13].position.y + chunkHeight / 2.0f);
}

void UpdateGamestate_PostPhysics(F32 dt)
{
	SetViewport({ vec2(0, 0), vec2(600, 480) });

	for (size_t i = 0; i < entities.size(); ++i)
	{
		// NOTE: sort by x position
		// NOTE: render higher x first
		DrawSprite(&entities[i].sprite, entities[i].spriteOffset, entities[i].transform, &camera);
	}

	for (size_t x = 0; x < numberOfHorizontalChunks; ++x)
	{
		for (size_t y = 0; y < numberOfVerticalChunks; ++y)
		{
			DrawRectangleOutline(chunks[x][y].position + vec2(0, chunkHeight), vec2(chunkWidth, chunkHeight), vec4(1, 0, 0, 1), &camera);
		}
	}
	DrawRectangleOutline(chunks[1][13].position + vec2(0, chunkHeight), vec2(chunkWidth, chunkHeight), vec4(0, 1, 0, 1), &camera, .1);

	// NOTE: Camera space grid, corresponding to tiles
	F32 initialXPos = camera.position.x - chunkWidth / 2.0f;
	F32 endXPos = initialXPos + chunkWidth;
	F32 posY = camera.position.y + chunkHeight / 2.0f;
	F32 negY = camera.position.y - chunkHeight / 2.0f;
	vec4 gridLineColor = vec4(0, 0, 0, 1);
	for (F32 xPos = initialXPos; xPos <= endXPos; ++xPos)
	{
		DrawLine(vec2(xPos, posY), vec2(xPos, negY), gridLineColor, &camera);
	}

	F32 initialYPos = camera.position.y - chunkHeight / 2.0f;
	F32 endYPos = initialYPos + chunkHeight;
	F32 posX = camera.position.x + chunkWidth / 2.0f;
	F32 negX = camera.position.x - chunkWidth / 2.0f;
	for (F32 yPos = initialYPos; yPos <= endYPos; ++yPos)
	{
		DrawLine(vec2(posX, yPos), vec2(negX, yPos), gridLineColor, &camera);
	}

	te.elapsedTime += dt;
	if (te.elapsedTime >= te.animationTime)
	{
		te.elapsedTime -= te.animationTime;
	}
	U32 animationFrame = (U32)(te.elapsedTime / (te.animationTime / (F32)te.numberOfFrames));
	DrawSprite(&te.sprites[animationFrame], te.spriteOffset, te.transform, &camera);

	DrawUVRectangleScreenSpace(&guiPanel, vec2(0, 0), vec2(guiPanel.width, guiPanel.height));
}

void GameUpdate(F32 deltaTime)
{
#ifdef COLLISION2DAPPLET
	UpdateCollisionDetection2DApplet(deltaTime);
#elif GAME
	Clear();
	UpdateGamestate_PrePhysics(deltaTime);
	// something something physics and collision detection
	UpdateGamestate_PostPhysics(deltaTime);
#endif
}

bool GameShutdown()
{
#ifdef COLLISION2DAPPLET
	ShutdownCollisionDetection2DApplet();
#endif

	Destroy(&spriteFiles);

	ShutdownRenderer();
	return true;
}