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

inline U32 String_HashFunction(char* key, U32 numberOfIndices)
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

inline char* GetValue(SpriteFileTable* sft, char* key)
{
	char* result = NULL;

	// NOTE: How is this better?
	U32 i = (String_HashFunction(key, sft->numberOfIndices) & 0x7fffffff) % sft->numberOfIndices;
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
	char* keyCopy = (char*)malloc(sizeof(char)* (keyLength + 1));
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
	char* valueCopy = (char*)malloc(sizeof(char)* (valueLength + 1));
	for (size_t i = 0; i < valueLength; ++i)
	{
		valueCopy[i] = value[i];
	}
	valueCopy[valueLength] = '\0';

	if (GetValue(sft, key) == NULL)
	{

		U32 i = (String_HashFunction(key, sft->numberOfIndices) & 0x7fffffff) % sft->numberOfIndices;
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



struct Animation
{
	Texture* frames;
	U32 numberOfFrames;
	F32 animationTime;
};
typedef std::vector<Animation> Animations;

inline void Initialize(Animation* animation, char* animationFolderPath, U32 numberOfFrames, F32 animationTime)
{
	Initialize(&animation->frames, animationFolderPath, numberOfFrames);
	animation->animationTime = animationTime;
	animation->numberOfFrames = numberOfFrames;
}

inline void Destroy(Animation* animation)
{
	for (size_t i = 0; i < animation->numberOfFrames; ++i)
	{
		Destroy(&animation->frames[i]);
	}
	animation->numberOfFrames = 0;
	animation->animationTime = 0;
}

struct AnimationHashLink
{
	char* k;
	Animation* v;
};

struct AnimationHash
{
	U32 numberOfIndices;
	AnimationHashLink** table;
	U32* subListLength;

	size_t length;
};

inline void Initialize(AnimationHash* ah, U32 numberOfIndices)
{
	ah->numberOfIndices = numberOfIndices;
	ah->table = (AnimationHashLink**)malloc(sizeof(AnimationHashLink*) * numberOfIndices);
	ah->subListLength = (U32*)malloc(sizeof(U32) * numberOfIndices);

	for (size_t i = 0; i < numberOfIndices; ++i)
	{
		ah->table[i] = NULL;
		ah->subListLength[i] = 0;
	}

	ah->length = 0;
}

struct GetKeyIndexResult
{
	bool present;
	U32 i;
	U32 j;
};
inline GetKeyIndexResult GetKeyIndex(AnimationHash* ah, char* key)
{
	GetKeyIndexResult result = {false, 0, 0};

	U32 i = (String_HashFunction(key, ah->numberOfIndices) & 0x7fffffff) % ah->numberOfIndices;

	if (ah->subListLength[i] == 0)
	{
		return result;
	}

	for (U32 j = 0; j < ah->subListLength[i]; ++j)
	{
		bool match = Compare(key, ah->table[i][j].k);
		if (match)
		{
			result.present = true;
			result.i = i;
			result.j = j;
			break;
		}
	}

	return result;
}

inline Animation* GetValue(AnimationHash* ah, char* key)
{
	Animation* result = NULL;

	GetKeyIndexResult keyIndex = GetKeyIndex(ah, key);
	if (keyIndex.present)
	{
		result = ah->table[keyIndex.i][keyIndex.j].v;
	}

	return result;
}

inline void AddKVPair(AnimationHash* ah, char* key, Animation* val)
{
	char* keyCopy = Copy(key);

	// If the KV pair is not already in the hash table
	if (GetValue(ah, key) == NULL)
	{
		U32 i = (String_HashFunction(key, ah->numberOfIndices) & 0x7fffffff) % ah->numberOfIndices;

		++ah->subListLength[i];
		AnimationHashLink* newSubList = (AnimationHashLink*)malloc(sizeof(AnimationHashLink) * ah->subListLength[i]);
		AnimationHashLink* oldSubList = ah->table[i];

		// Copy old hash list to new hash list
		for (size_t j = 0; j < ah->subListLength[i] - 1; ++j)
		{
			newSubList[j] = oldSubList[j];
		}

		// Add new kv pair to the new hash list
		newSubList[ah->subListLength[i] - 1].k = keyCopy;
		newSubList[ah->subListLength[i] - 1].v = val;

		// Delete old hash list
		ah->table[i] = newSubList;
		free(oldSubList);

		++ah->length;
	}
}

inline void RemoveKVPair(AnimationHash* ah, char* key)
{
	GetKeyIndexResult ki = GetKeyIndex(ah, key);
	if (ki.present)
	{
		AnimationHashLink* oldSubList = ah->table[ki.i];

		if (ah->subListLength[ki.i] == 1)
		{
			ah->table[ki.i] = NULL;
		}
		else
		{
			AnimationHashLink* newSubList = (AnimationHashLink*)malloc(sizeof(AnimationHashLink)* (ah->subListLength[ki.i] - 1));
			for (U32 j = 0, newNext = 0; j < ah->subListLength[ki.i]; ++j)
			{
				if (j != ki.j)
				{
					newSubList[newNext] = oldSubList[j];
					++newNext;
				}
			}
			ah->table[ki.i] = newSubList;
		}
		free(oldSubList);
		--ah->subListLength[ki.i];
	}
}

inline size_t Length(AnimationHash* ah)
{
	size_t result;

	result = ah->length;

	return result;
}

inline void Destroy(AnimationHash* ah)
{
	for (size_t i = 0; i < ah->numberOfIndices; ++i)
	{
		for (size_t j = 0; j < ah->subListLength[i]; ++j)
		{
			free(ah->table[i][j].k);
			Destroy(ah->table[i][j].v);
		}
		free(ah->table[i]);
	}
	free(ah->table);

	ah->table = NULL;
	ah->numberOfIndices = 0;
	ah->length = 0;
}



struct Entity
{
	Transform transform;

	// Renderable
	vec2 spriteOffset;

	Texture sprite;

	AnimationHash animations;
	bool isAnimated;
	bool isAnimationLooped;
	bool isAnimationPlaying;
	bool isAnimationReversed;
	U32 activeAnimationKI_i;
	U32 activeAnimationKI_j;
	F32 elapsedTime = 0.0f;
};
typedef std::vector<Entity> Entities;

inline void AddSprite(Entity* entity, char* assetName, vec2 offset = vec2(0.0f, 0.0f))
{
	Initialize(&entity->sprite, assetName);
	entity->spriteOffset = offset;
}

inline void RemoveSprite(Entity* entity)
{
	Destroy(&entity->sprite);
}

inline void AddAnimation(Entity* entity, char* referenceName, char* assetName, U32 numberOfFrames, F32 animationTime)
{
	Animation* animation = (Animation*)malloc(sizeof(Animation));
	char* filePath = GetValue(&spriteFiles, assetName);
	Initialize(animation, filePath, numberOfFrames, animationTime);
	AddKVPair(&entity->animations, referenceName, animation);
	entity->isAnimated = true;
}

inline void RemoveAnimation(Entity* entity, char* referenceName)//U32 animationIndex)
{
	GetKeyIndexResult ki = GetKeyIndex(&entity->animations, referenceName);
	if (ki.present && ki.i == entity->activeAnimationKI_i && ki.j == entity->activeAnimationKI_j)
	{
		entity->isAnimationPlaying = false;
		entity->isAnimationLooped = false;
		entity->isAnimationReversed = false;
		entity->elapsedTime = 0.0f;
		entity->activeAnimationKI_i = 0;
		entity->activeAnimationKI_j = 0;
	}

	Animation* animation = GetValue(&entity->animations, referenceName);
	Destroy(animation);
	RemoveKVPair(&entity->animations, referenceName);


	if (Length(&entity->animations) == 0)
	{
		entity->isAnimated = false;
	}

}

inline bool StartAnimation(Entity* entity, char* referenceName, bool loopAnimation = true, F32 startTime = 0.0f)
{
	GetKeyIndexResult ki = GetKeyIndex(&entity->animations, referenceName);
	if (ki.present)
	{
		entity->activeAnimationKI_i = ki.i;
		entity->activeAnimationKI_j = ki.j;
		entity->elapsedTime = 0.0f;
		entity->isAnimationPlaying = true;
		return true;
	}

	return false;
}

inline void StopAnimation(Entity* entity)
{
	entity->isAnimationPlaying = false;
	entity->isAnimationLooped = false;
	entity->isAnimationReversed = false;
	entity->elapsedTime = 0.0f;
}

inline void PauseAnimation(Entity* entity)
{
	entity->isAnimationPlaying = false;
}

inline void ReverseAnimation(Entity* entity, bool reverse = true)
{
	entity->isAnimationReversed = reverse;
}

inline void SetAnimationTime(Entity* entity, F32 time)
{
	F32 animTime = entity->animations.table[entity->activeAnimationKI_i][entity->activeAnimationKI_j].v->animationTime;
	F32 newTime = time - ((I32)(time/animTime) * animTime);
	entity->elapsedTime = newTime;
}

// 0 == 0%, 100 == 100%
inline void SetAnimationTimeAsPercent(Entity* entity, F32 percent)
{
	F32 multiplier = (percent / 100.0f) - (I32)(percent / 100.0f);
	F32 animationLength = entity->animations.table[entity->activeAnimationKI_i][entity->activeAnimationKI_j].v->animationTime;
	F32 newTime = multiplier * animationLength;
	entity->elapsedTime = newTime;
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
Entities entities;
Camera camera;
Texture guiPanel;
Entity te;



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
	//Initialize(&e.sprite, "Assets/x60/Objects/tree_generic.bmp");
	AddSprite(&e, "tree_Generic");
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

	AddKVPair(&spriteFiles, "water_Deep", "Assets/x60/Objects/water_Deep");
	AddKVPair(&spriteFiles, "link_Right", "Assets/x60/Objects/link_Right");
	AddKVPair(&spriteFiles, "link_Left", "Assets/x60/Objects/link_Left");
	AddKVPair(&spriteFiles, "link_Up", "Assets/x60/Objects/link_Up");
	AddKVPair(&spriteFiles, "link_Down", "Assets/x60/Objects/link_Down");



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

	camera.position = chunks[13][13].position + vec2(chunkWidth / 2.0f, chunkHeight / 2.0f);

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

	te.transform.position = entities.back().transform.position - vec2(0,1);
	Initialize(&te.animations, 1);
	AddAnimation(&te, "right", "link_Right", 2, 0.33f);
	AddAnimation(&te, "left", "link_Left", 2, 0.33f);
	AddAnimation(&te, "up", "link_Up", 2, 0.33f);
	AddAnimation(&te, "down", "link_Down", 2, 0.33f);
	StartAnimation(&te, "right");

	RemoveAnimation(&te, "left");
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
	if (GetKeyDown(KeyCode_Left))
	{
		StartAnimation(&te, "left");
	}
	if (GetKeyDown(KeyCode_Right))
	{
		StartAnimation(&te, "right");
	}
	if (GetKeyDown(KeyCode_Down))
	{
		StartAnimation(&te, "down");
	}
	if (GetKeyDown(KeyCode_Up))
	{
		StartAnimation(&te, "up");
	}

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
	DrawRectangleOutline(chunks[1][13].position + vec2(0, chunkHeight), vec2(chunkWidth, chunkHeight), vec4(0, 1, 0, 1), &camera, .1f);

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
	if (te.elapsedTime >= te.animations.table[te.activeAnimationKI_i][te.activeAnimationKI_j].v->animationTime)
	{
		te.elapsedTime -= te.animations.table[te.activeAnimationKI_i][te.activeAnimationKI_j].v->animationTime;
	}
	U32 animationFrame = (U32)(te.elapsedTime / (te.animations.table[te.activeAnimationKI_i][te.activeAnimationKI_j].v->animationTime / (F32)te.animations.table[te.activeAnimationKI_i][te.activeAnimationKI_j].v->numberOfFrames));
	DrawSprite(&te.animations.table[te.activeAnimationKI_i][te.activeAnimationKI_j].v->frames[animationFrame], te.spriteOffset, te.transform, &camera);

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