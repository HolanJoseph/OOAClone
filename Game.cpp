#include "Types.h"
#include "Math.h"

#include "_GameAPI.h"
#include "_SystemAPI.h"

#include "BitManip.h"
#include "Util.h"
#include "RandomNumberGenerator.h"
#include "String.h"
#include "MergeSort.h"

#include "AssetLoading.h"
#include "AssetNameToFilepathTable.h"

#include "Renderer.h"
//#include "CollisionDetection2D.h"
#include "CollisionDetection2DObj.h"

#include "StringAPI_Tests.h"

//#include "CollisionDetection2D_Applet.h"

#include <vector>
using std::vector;
#include <typeinfo>



//#define RUN_UNIT_TESTS 1
//#define COLLISION2DAPPLET 1
#define GAME 1



const vec2 TileDimensions = vec2(0.5f, 0.5f);



/*
 * Forward Declarations
 */
void DebugDrawPoint(vec2 p, F32 pointSize, vec4 color);
void DebugDrawLine(vec2 a, vec2 b, vec4 color);
void DebugDrawRectangleOutline(vec2 upperLeft, vec2 dimensions, vec4 color, F32 offset = 0.0f);
void DebugDrawRectangleSolidColor(vec2 halfDim, Transform transform, vec4 color);



StringStringHashTable spriteAssetFilepathTable;
inline void ReadInSpriteAssets(StringStringHashTable* hash, char* filename)
{
	GetFileSizeReturnType assetFileSize = GetFileSize(filename);
	if (assetFileSize.fileExists)
	{
		char* fullFile = (char*)malloc(sizeof(char) * (assetFileSize.fileSize + 1));
		ReadFileReturnType readStats = ReadFile(filename, fullFile, assetFileSize.fileSize);
		fullFile[assetFileSize.fileSize] = '\0';
		SplitResult lines = Split(fullFile, assetFileSize.fileSize, "\r\n", 3);
		free(fullFile);

		for (size_t i = 0; i < lines.numberOfComponents; ++i)
		{
			SplitResult lineParts = Split(lines.components[i], lines.componentLengths[i], '=');
			Assert(lineParts.numberOfComponents == 2);

			const char* assetName = lineParts.components[0];
			const char* assetPath = lineParts.components[1];
			hash->AddKVPair(assetName, assetPath);
			
			lineParts.Destroy();
		}
		lines.Destroy();
	}
};

const char* AssetNameToFilepath(const char* assetName)
{
	const char* result = spriteAssetFilepathTable.GetValue(assetName);
	return result;
}



/*
 * GameObject Components
 */
struct Sprite
{
	TextureHandle texture;
	vec2 offset;

	Sprite()
	{
		texture = TextureHandle();
		offset = vec2();
	}

	void Initialize(const char* assetName, vec2 offset = vec2(0.0f, 0.0f))
	{
		const char* filePath = AssetNameToFilepath(assetName);
		this->texture = AddToTexturePool(filePath);
		this->offset = offset;
	}
};

/*
*  Animations
*/
struct Animation
{
	TextureHandle* frames;
	U32 numberOfFrames;
	F32 animationTime;

	Animation() : frames(NULL), numberOfFrames(0), animationTime(0) {};

	void Initialize(const char* animationFolderPath, U32 numberOfFrames, F32 animationTime)
	{
		this->frames = (TextureHandle*)malloc(sizeof(TextureHandle)* numberOfFrames);

		char** files;
		char fileType[] = ".bmp";
		size_t fileTypeLength = 5;
		files = (char**)malloc(sizeof(char*)* numberOfFrames);
		for (size_t i = 0; i < numberOfFrames; ++i)
		{
			char* iAsString = ToString(i);
			files[i] = Concat(iAsString, fileType);
			free(iAsString);
		}

		char* slashPath = Concat(animationFolderPath, "/");
		for (size_t x = 0; x < numberOfFrames; ++x)
		{
			char* frameXFullPath = Concat(slashPath, files[x]);
			TextureHandle frameX = AddToTexturePool(frameXFullPath);
			free(frameXFullPath);

			this->frames[x] = frameX;
		}
		free(slashPath);

		for (size_t i = 0; i < numberOfFrames; ++i)
		{
			free(files[i]);
		}
		free(files);

		this->animationTime = animationTime;
		this->numberOfFrames = numberOfFrames;
	}

	void Destroy()
	{
		for (size_t i = 0; i < this->numberOfFrames; ++i)
		{
			//RemoveFromTexturePool(animation->frames[i]);
		}
		free(this->frames);

		this->numberOfFrames = 0;
		this->animationTime = 0;
	}
};

struct AnimationHash
{
	struct AnimationHashLink
	{
		const char* k;
		Animation* v;
	};

	U32 numberOfIndices;
	AnimationHashLink** table;
	U32* subListLength;

	size_t length;

	AnimationHash() : numberOfIndices(0), table(NULL), subListLength(NULL), length(0) {};

	void Initialize(U32 numberOfIndices)
	{
		this->numberOfIndices = numberOfIndices;
		this->table = (AnimationHashLink**)malloc(sizeof(AnimationHashLink*)* numberOfIndices);
		this->subListLength = (U32*)malloc(sizeof(U32)* numberOfIndices);

		for (size_t i = 0; i < numberOfIndices; ++i)
		{
			this->table[i] = NULL;
			this->subListLength[i] = 0;
		}

		this->length = 0;
	}

	struct GetKeyIndexResult
	{
		bool present;
		U32 i;
		U32 j;

		GetKeyIndexResult() : present(false), i(0), j(0) {};
	};
	GetKeyIndexResult GetKeyIndex(const char* key)
	{
		GetKeyIndexResult result = GetKeyIndexResult();

		U32 i = (String_HashFunction(key, this->numberOfIndices) & 0x7fffffff) % this->numberOfIndices;

		if (this->subListLength[i] == 0)
		{
			return result;
		}

		for (U32 j = 0; j < this->subListLength[i]; ++j)
		{
			bool match = Compare(key, this->table[i][j].k);
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

	const char* GetKey(size_t i, size_t j)
	{
		const char* result = NULL;

		if (i < this->numberOfIndices && j < this->subListLength[i])
		{
			result = table[i][j].k;
		}

		return result;
	}

	Animation* GetValue(const char* key)
	{
		Animation* result = NULL;

		GetKeyIndexResult keyIndex = GetKeyIndex(key);
		if (keyIndex.present)
		{
			result = this->table[keyIndex.i][keyIndex.j].v;
		}

		return result;
	}

	void AddKVPair(const char* key, Animation* val)
	{
		const char* keyCopy = Copy(key);

		// If the KV pair is not already in the hash table
		if (GetValue(key) == NULL)
		{
			U32 i = (String_HashFunction(key, this->numberOfIndices) & 0x7fffffff) % this->numberOfIndices;

			++this->subListLength[i];
			AnimationHashLink* newSubList = (AnimationHashLink*)malloc(sizeof(AnimationHashLink)* this->subListLength[i]);
			AnimationHashLink* oldSubList = this->table[i];

			// Copy old hash list to new hash list
			for (size_t j = 0; j < this->subListLength[i] - 1; ++j)
			{
				newSubList[j] = oldSubList[j];
			}

			// Add new kv pair to the new hash list
			newSubList[this->subListLength[i] - 1].k = keyCopy;
			newSubList[this->subListLength[i] - 1].v = val;

			// Delete old hash list
			this->table[i] = newSubList;
			free(oldSubList);

			++this->length;
		}
	}

	void RemoveKVPair(const char* key)
	{
		GetKeyIndexResult ki = GetKeyIndex(key);
		if (ki.present)
		{
			AnimationHashLink* oldSubList = this->table[ki.i];

			if (this->subListLength[ki.i] == 1)
			{
				this->table[ki.i] = NULL;
			}
			else
			{
				AnimationHashLink* newSubList = (AnimationHashLink*)malloc(sizeof(AnimationHashLink)* (this->subListLength[ki.i] - 1));
				for (U32 j = 0, newNext = 0; j < this->subListLength[ki.i]; ++j)
				{
					if (j != ki.j)
					{
						newSubList[newNext] = oldSubList[j];
						++newNext;
					}
				}
				this->table[ki.i] = newSubList;
			}
			free(oldSubList);
			--this->subListLength[ki.i];
			--this->length;
		}
	}

	size_t Length()
	{
		size_t result;

		result = this->length;

		return result;
	}

	void Destroy()
	{
		for (size_t i = 0; i < this->numberOfIndices; ++i)
		{
			for (size_t j = 0; j < this->subListLength[i]; ++j)
			{
				free((void*)this->table[i][j].k);
				this->table[i][j].v->Destroy();
			}
			free(this->table[i]);
		}
		free(this->table);

		this->table = NULL;
		this->numberOfIndices = 0;
		this->length = 0;
	}
};

struct AnimationController
{
	AnimationHash animations;
	U32 activeAnimationKI_i;
	U32 activeAnimationKI_j;
	bool isAnimated;
	bool isAnimationPlaying;
	bool isAnimationLooped;
	bool isAnimationReversed;
	F32 elapsedTime;
	vec2 spriteOffset;

	AnimationController()
	{
		this->animations = AnimationHash();
		this->isAnimated = false;
		this->isAnimationLooped = false;
		this->isAnimationPlaying = false;
		this->isAnimationReversed = false;
		this->activeAnimationKI_i = 0;
		this->activeAnimationKI_j = 0;
		this->elapsedTime = 0.0f;
		this->spriteOffset = vec2(0.0f, 0.0f);
	}

	void Initialize()
	{
		this->animations = AnimationHash();
		this->animations.Initialize(1);
		this->isAnimated = false;
		this->isAnimationLooped = false;
		this->isAnimationPlaying = false;
		this->isAnimationReversed = false;
		this->activeAnimationKI_i = 0;
		this->activeAnimationKI_j = 0;
		this->elapsedTime = 0.0f;
		this->spriteOffset = vec2(0.0f, 0.0f);
	}

	/* NOTE: Should this chain be returning const char *???*/
	const char * ActiveAnimation()
	{
		const char * result = this->animations.GetKey(this->activeAnimationKI_i, this->activeAnimationKI_j);
		return result;
	}

	void AddAnimation(const char* referenceName, const char* assetName, U32 numberOfFrames, F32 animationTime)
	{
		const char* filePath = spriteAssetFilepathTable.GetValue(assetName);
		Animation* animation = new Animation();
		animation->Initialize(filePath, numberOfFrames, animationTime);
		this->animations.AddKVPair(referenceName, animation);
		this->isAnimated = true;
	}

	void RemoveAnimation(const char* referenceName)
	{
		const char* currentAnimationName = this->animations.table[this->activeAnimationKI_i][this->activeAnimationKI_j].k;

		AnimationHash::GetKeyIndexResult ki = this->animations.GetKeyIndex(referenceName);
		if (ki.present && ki.i == this->activeAnimationKI_i && ki.j == this->activeAnimationKI_j)
		{
			this->isAnimationPlaying = false;
			this->isAnimationLooped = false;
			this->isAnimationReversed = false;
			this->elapsedTime = 0.0f;
			this->activeAnimationKI_i = 0;
			this->activeAnimationKI_j = 0;
		}

		Animation* animation = animations.GetValue(referenceName);
		animation->Destroy();
		animations.RemoveKVPair(referenceName);

		AnimationHash::GetKeyIndexResult currentAnimationNewKeyIndex = animations.GetKeyIndex(currentAnimationName);
		this->activeAnimationKI_i = currentAnimationNewKeyIndex.i;
		this->activeAnimationKI_j = currentAnimationNewKeyIndex.j;

		if (animations.Length() == 0)
		{
			this->isAnimated = false;
		}

	}

	void StartAnimation()
	{
		if (this->isAnimated &&
			this->isAnimationPlaying == false &&
			this->animations.length > 0)
		{
			this->isAnimationPlaying = true;

			F32 animationLength = this->animations.table[this->activeAnimationKI_i][this->activeAnimationKI_j].v->animationTime;
			if (this->elapsedTime == 0.0f && this->isAnimationReversed)
			{
				this->elapsedTime = animationLength;
			}
			if (this->elapsedTime == animationLength && !this->isAnimationReversed)
			{
				this->elapsedTime = 0.0f;
			}
		}
	}

	bool StartAnimation(const char* referenceName, bool restartAnimation = true, bool loopAnimation = true, bool playInReverse = false, F32 startTime = 0.0f)
	{
		AnimationHash::GetKeyIndexResult ki = this->animations.GetKeyIndex(referenceName);
		if (restartAnimation == false && this->IsPaused() == false && ki.i == this->activeAnimationKI_i && ki.j == this->activeAnimationKI_j)
		{
			return true;
		}

		if (ki.present)
		{
			this->activeAnimationKI_i = ki.i;
			this->activeAnimationKI_j = ki.j;
			if (playInReverse)
			{
				this->isAnimationReversed = true;

				if (startTime != 0.0f)
				{
					this->elapsedTime = startTime;
				}
				else
				{
					this->elapsedTime = this->animations.table[this->activeAnimationKI_i][this->activeAnimationKI_j].v->animationTime;
				}
			}
			else
			{
				// NOTE: If we forward declare StepElapsedAnimationTime we could use that here and it would be 
				//			the correct way of doing this, BUT seeing as it is called every frame before displaying
				//			an erroneous value will be dealt with before and animation frames are displayed anyway.
				this->elapsedTime = startTime;
			}
			this->isAnimationPlaying = true;
			this->isAnimationLooped = loopAnimation;
			return true;
		}

		return false;
	}

	bool IsPaused()
	{
		bool result = !this->isAnimationPlaying;
		return result;
	}

	void PauseAnimation()
	{
		this->isAnimationPlaying = false;
	}

	void StopAnimation()
	{
		this->isAnimationPlaying = false;
		// NOTE: Maybe leave these in? Only time will tell.
		//entity->isAnimationLooped = false;
		//entity->isAnimationReversed = false;
		this->elapsedTime = 0.0f;
	}

	// ?? Useful?
	void ReverseAnimation(bool reverse = true)
	{
		this->isAnimationReversed = reverse;
	}

	void StepElapsedAnimationTime(F32 time)
	{
		if (this->isAnimated && this->isAnimationPlaying)
		{
			F32 animationLength = this->animations.table[this->activeAnimationKI_i][this->activeAnimationKI_j].v->animationTime;

			if (this->isAnimationReversed)
			{
				this->elapsedTime -= time;
			}
			else
			{
				this->elapsedTime += time;
			}

			if (this->isAnimationLooped)
			{
				if (this->elapsedTime > animationLength || this->elapsedTime < 0.0f)
				{
					I32 completionCount = (I32)(this->elapsedTime / animationLength);
					F32 deadTime = completionCount * animationLength;
					this->elapsedTime -= deadTime;

					if (this->elapsedTime < 0.0f)
					{
						this->elapsedTime += animationLength;
					}
				}
			}
			else
			{
				if (this->elapsedTime > animationLength || this->elapsedTime < 0.0f)
				{
					this->elapsedTime = ClampRange_F32(this->elapsedTime, 0.0f, animationLength);
					this->PauseAnimation();
				}
			}
		}
	}

	void SetElapsedAnimationTime(F32 time)
	{
		F32 animTime = this->animations.table[this->activeAnimationKI_i][this->activeAnimationKI_j].v->animationTime;
		F32 newTime = time - ((I32)(time / animTime) * animTime);
		this->elapsedTime = newTime;
	}

	// 0 == 0%, 100 == 100%
	void SetElapsedAnimationTimeAsPercent(F32 percent)
	{
		F32 multiplier = (percent / 100.0f) - (I32)(percent / 100.0f);
		F32 animationLength = this->animations.table[this->activeAnimationKI_i][this->activeAnimationKI_j].v->animationTime;
		F32 newTime = multiplier * animationLength;
		this->elapsedTime = newTime;
	}

	size_t NumberOfAnimations()
	{
		size_t result;
		result = this->animations.Length();
		return result;
	}

	TextureHandle GetCurrentAnimationFrame()
	{
		TextureHandle result = TextureHandle();

		if (this->isAnimated && this->animations.length > 0)
		{
			F32 animationLength = this->animations.table[this->activeAnimationKI_i][this->activeAnimationKI_j].v->animationTime;
			U32 numberOfFrames = this->animations.table[this->activeAnimationKI_i][this->activeAnimationKI_j].v->numberOfFrames;
			F32 elapsedTime = this->elapsedTime;
			U32 animationFrame = (U32)(elapsedTime / (animationLength / (F32)numberOfFrames));
			animationFrame = ClampRange_U32(animationFrame, 0, numberOfFrames - 1);
			AnimationHash::AnimationHashLink** t = this->animations.table;
			U32 indI = this->activeAnimationKI_i;
			U32 indJ = this->activeAnimationKI_j;
			result = t[indI][indJ].v->frames[animationFrame];
		}

		return result;
	};

	void SetSpriteOffset(vec2 spriteOffset)
	{
		this->spriteOffset = spriteOffset;
	}

	vec2 GetSpriteOffset()
	{
		vec2 result;
		result = this->spriteOffset;
		return result;
	}
};

/*
 * RigidBodies
 */
#define DEFAULT_DAMPING_FACTOR 1.0f
struct RigidBody
{
	vec2 position;
	vec2 velocity; // m/s
	vec2 forceAccumulator;

	F32 inverseMass; // Kg
	F32 dampingFactor;

	vec2 frameVelocity; // m/s // NOTE: This velocity is the velocity for the object for this frame, used to query velocity after integration.

	RigidBody()
	{
		this->position = vec2(0.0f, 0.0f);
		this->velocity = vec2(0.0f, 0.0f);
		this->forceAccumulator = vec2(0.0f, 0.0f);
		this->inverseMass = 0.0f;
		this->dampingFactor = DEFAULT_DAMPING_FACTOR;

		this->frameVelocity = vec2(0.0f, 0.0f);
	}

	void Initialize(
		vec2 position = vec2(0.0f, 0.0f),
		vec2 velocity = vec2(0.0f, 0.0f),
		vec2 force = vec2(0.0f, 0.0f),
		F32 inverseMass = 0.0f,
		F32 dampingFactor = DEFAULT_DAMPING_FACTOR)
	{
		this->position = position;
		this->velocity = velocity;
		this->forceAccumulator = force;

		this->inverseMass = inverseMass;
		this->dampingFactor = dampingFactor;

		this->frameVelocity = vec2(0.0f, 0.0f);
	};

	void Destroy()
	{
		this->position = vec2(0.0f, 0.0f);
		this->velocity = vec2(0.0f, 0.0f);
		this->forceAccumulator = vec2(0.0f, 0.0f);

		this->inverseMass = 0.0f;
		this->dampingFactor = DEFAULT_DAMPING_FACTOR;

		this->frameVelocity = vec2(0.0f, 0.0f);
	};

	F32 GetMass()
	{
		F32 result;

		if (this->inverseMass == 0.0f)
		{
			result = 0.0f;
		}
		else
		{
			result = 1.0f / this->inverseMass;
		}

		return result;
	}

	vec2 GetVelocityForFrame()
	{
		vec2 result;
		result = this->frameVelocity;
		return result;
	}

	vec2 GetAcceleration()
	{
		vec2 result = this->inverseMass * this->forceAccumulator;
		return result;
	}

	void SetPosition(vec2 position)
	{
		this->position = position;
	}

	void Integrate(F32 deltaTime)
	{
		vec2 acceleration = this->GetAcceleration();
		this->frameVelocity = this->velocity * deltaTime; // NOTE: Honestly not sure if this is accurate. Though it should be because of the next line.
		this->position = this->position + this->frameVelocity; /*(this->velocity * deltaTime);*/
		this->velocity = (this->velocity * pow(this->dampingFactor, deltaTime)) + (acceleration * deltaTime);
		this->forceAccumulator = vec2(0.0f, 0.0f);
	}

	void ApplyForce(vec2 direction, F32 power)
	{
		vec2 scaledForce = direction * power;
		this->forceAccumulator += scaledForce;
	}

	// directions should be normalized
	void ApplyImpulse(vec2 direction, F32 power)
	{
		vec2 scaledVelocity = direction * power;
		this->velocity += scaledVelocity;
	}
};



/*
 * Events
 */
enum ArgumentType
{
	AT_NULL,

	AT_Pointer,
	AT_I32,
	AT_U32,
	AT_F32,
	AT_Bool,
	AT_Vec2,

	AT_COUNT
};

struct EventArgument
{
	ArgumentType type;
	union 
	{
		void* asPointer;
		I32 asI32;
		U32 asU32;
		F32 asF32;
		bool asBool;
		F32 asVec2[2];
	};

	EventArgument()
	{
		this->type = AT_NULL;
	}

	EventArgument(void* pointer)
	{
		this->type = AT_Pointer;
		this->asPointer = pointer;
	}

	EventArgument(I32 i32)
	{
		this->type = AT_I32;
		this->asI32 = i32;
	}

	EventArgument(U32 u32)
	{
		this->type = AT_U32;
		this->asU32 = u32;
	}

	EventArgument(F32 f32)
	{
		this->type = AT_F32;
		this->asF32 = f32;
	}

	EventArgument(bool b)
	{
		this->type = AT_Bool;
		this->asBool = b;
	}

	EventArgument(vec2 vec)
	{
		this->type = AT_Vec2;
		this->asVec2[0] = vec.x;
		this->asVec2[1] = vec.y;
	}

	ArgumentType GetType()
	{
		ArgumentType result;
		result = this->type;
		return result;
	}

	void* AsPointer()
	{
		void* result;
		result = this->asPointer;
		return result;
	}

	I32 AsI32()
	{
		I32 result;
		result = this->asI32;
		return result;
	}

	U32 AsU32()
	{
		U32 result;
		result = this->asU32;
		return result;
	}

	F32 AsF32()
	{
		F32 result;
		result = this->asF32;
		return result;
	}

	bool AsBool()
	{
		bool result;
		result = this->asBool;
		return result;
	}

	vec2 AsVec2()
	{
		vec2 result;
		result.x = this->asVec2[0];
		result.y = this->asVec2[1];
		return result;
	}
};

enum EventType
{
	ET_NULL,

	ET_OnCollisionEnter,
	ET_OnCollision,
	ET_OnCollisionExit,

	ET_COUNT
};

struct Event
{
	EventType type;
	static const size_t MAXARGUMENTCOUNT = 8;
	EventArgument arguments[MAXARGUMENTCOUNT];

	Event()
	{
		this->type = ET_NULL;
	}

	EventType GetType()
	{
		EventType result;
		result = this->type;
		return result;
	}

	void SetType(EventType type)
	{
		this->type = type;
	}
};



/*
 * GameObject
 */
struct GameObject
{
	enum Type
	{
		Null,

		PlayerCharacter,
		StaticEnvironmentPiece,
		PlayerCamera,
		Button,
		/*// Player Character
		Link,

		// Key NPCs
		Impa,
		Ralph,
		Nayru,
		Veran,
		MakuTree,
		BabyMakuTree,
		
		// Friendly NPCs
		Monkey,
		Parrot,
		Bear,
		Bunny,
		Villager,
		Foreman,
		Soldier,
		Worker,
		StoreKeeper,
		
		// Enemies
		RedOctorok,
		Keese,
		Zol,
		Gel,
		BlueStalfos,
		RedMoblin,
		Crow,
		Rope,
		Ghini,
		Wallmaster,
		
		// Mini Bosses
		GiantGhini,
		MiniGhini,
		
		// Bosses
		PumpkinHead,

		// Environment
		Weed,
		DancingFlowers,
		Sign,
		Shrub,
		TriforceGate,
		Portal,
		Dirt,
		Tree,
		SpookyTree,
		Jar,
		TallGrass,
		Brazier,
		Rock,
		Stairs,
		StairPortal,
		Hole,
		MoveableBlock,
		Door,
		KeyDoor,
		BossKeyDoor,
		FloorSwitch,
		EyeStatue,
		TreePlot,
		GraveyardGate,
		ColorCube,
		WallRoots,
		Crystal,
		Chest,
		MovingPlatform,
		Cliff,
		MinibossPortal,
		
		// Pickups
		KeyItemPickup, // HeartPiece, HeartContainer, EssenceOfTime, Key, BossKey 
		ItemPickup,    // Rupee, Heart
		Fairy,*/

		
		COUNT
	};

	enum Tags
	{
		// Friendliness
		Hero,
		Friendly,
		Enemy,
		Environment,
		Background,
		Effect,

		// Equipment Interactions
		Readable,
		Cutable,
		Diggable,
		Openable,
		Burnable,
		Lightweight,
	};

	// Debug Variables
	bool debugDraw;

	Type type;
	U64  tags;

	Transform transform;
	Sprite*   sprite;
	AnimationController* animator;
	RigidBody* rigidbody;
	Shape_2D* collisionShape;

	// State
	// NOTE: This needs to be dealt with.
	vec2 facing;
	bool moving;
	bool pushingForward;

	GameObject()
	{
		this->type = Null;
		this->tags = 0;
		this->transform = Transform();
		this->sprite = NULL;
		this->animator = NULL;
		this->rigidbody = NULL;
		this->collisionShape = NULL;
	}

	void SetType(Type type);
	Type GetType();

	void AddTag(Tags tag);
	void RemoveTag(Tags tag);
	bool HasTag(Tags tag);

	void AddSprite(const char * sprite);
	void AddAnimator();
	void AddRigidbody(F32 invmass = 0.0f, F32 dampingfactor = 0.0f, vec2 velocity = vec2(0.0f, 0.0f), vec2 force = vec2(0.0f));
	template <typename S>
	void AddCollisionShape(S shape);

	void Update_PrePhysics(F32 dt);
	void Update_PostPhysics(F32 dt);
	void HandleEvent(Event* e);
	void DebugDraw();

	bool GetDebugState();
	void SetDebugState(bool debugState);
	void ToggleDebugState();

	static vector<GameObject*> gameObjects;
	static vector<GameObject*> physicsGameObjects;
	static vector<GameObject*> collisionGameObjects;
	static vector<GameObject*> staticCollisionGameObjects;

};

void GameObject::SetType(Type type)
{
	this->type = type;
}

GameObject::Type GameObject::GetType()
{
	Type result;

	result = this->type;

	return result;
}

void GameObject::AddTag(Tags tag)
{
	SetBit(&this->tags, (U8)tag, 1);
}

void GameObject::RemoveTag(Tags tag)
{
	SetBit(&tags, (U8)tag, 0);
}

bool GameObject::HasTag(Tags tag)
{
	bool result = IsBitSet(&tags, (U8)tag);
	return result;
}

void GameObject::AddSprite(const char * sprite)
{
	this->sprite = new Sprite();
	this->sprite->Initialize(sprite);
}

void GameObject::AddAnimator()
{
	this->animator = new AnimationController();
	this->animator->Initialize();
}

void GameObject::AddRigidbody(F32 invmass, F32 dampingfactor, vec2 velocity, vec2 force)
{
	this->rigidbody = new RigidBody();
	this->rigidbody->Initialize(this->transform.position, vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), 1.0f, 0.0f);
	physicsGameObjects.push_back(this);
}

template <typename S>
void GameObject::AddCollisionShape(S shape)
{
	S* collider = new S(shape);
	this->collisionShape = collider;
	if (this->HasTag(Environment))
	{
		staticCollisionGameObjects.push_back(this);
	}
	else
	{
		collisionGameObjects.push_back(this);
	}
	//collisionGameObjects.push_back(this);
}



/*
 * Forward declarations for the update functions
 */
GameObject* RaycastFirst_Line_2D(vec2 position, vec2 direction, F32 distance);
vector<GameObject*> RaycastAll_Line_2D(vec2 position, vec2 direction, F32 distance);

void GameObject::Update_PrePhysics(F32 dt)
{
	switch (this->GetType())
	{
	case PlayerCharacter:
	{
							bool getKey_A = GetKey(KeyCode_A);
							bool getKey_D = GetKey(KeyCode_D);
							bool getKey_W = GetKey(KeyCode_W);
							bool getKey_S = GetKey(KeyCode_S);
							bool getKeys_WD = getKey_W && getKey_D;
							bool getKeys_SD = getKey_S && getKey_D;
							bool getKeys_WA = getKey_W && getKey_A;
							bool getKeys_SA = getKey_S && getKey_A;

							if (getKeys_WD)
							{
								this->rigidbody->ApplyImpulse(normalize(vec2(1.0f, 1.0f)), 1.5f);
							}
							if (getKeys_SD)
							{
								this->rigidbody->ApplyImpulse(normalize(vec2(1.0f, -1.0f)), 1.5f);
							}
							if (getKeys_WA)
							{
								this->rigidbody->ApplyImpulse(normalize(vec2(-1.0f, 1.0f)), 1.5f);
							}
							if (getKeys_SA)
							{
								this->rigidbody->ApplyImpulse(normalize(vec2(-1.0f, -1.0f)), 1.5f);
							}
							if (!getKeys_WA && !getKeys_SA && getKey_A)
							{
								this->rigidbody->ApplyImpulse(vec2(-1.0f, 0.0f), 1.5f);
							}
							if (!getKeys_WD && !getKeys_SD && getKey_D)
							{
								this->rigidbody->ApplyImpulse(vec2(1.0f, 0.0f), 1.5f);
							}
							if (!getKeys_WD && !getKeys_WA && getKey_W)
							{
								this->rigidbody->ApplyImpulse(vec2(0.0f, 1.0f), 1.5f);
							}
							if (!getKeys_SD && !getKeys_SA && getKey_S)
							{
								this->rigidbody->ApplyImpulse(vec2(0.0f, -1.0f), 1.5f);
							}

							if (this->rigidbody->velocity == vec2(0.0f, 0.0f))
							{
								this->moving = false;
								this->animator->StopAnimation();
							}
							else
							{
								this->moving = true;
								//this->animator->StartAnimation();
							}

							vec2 velocityDirection = this->moving ? normalize(this->rigidbody->velocity) : vec2(0.0f, 0.0f);
							F32 facingDOTvelocityDirection = dot(this->facing, velocityDirection);
							vec2 newFacing = this->facing;
							if (facingDOTvelocityDirection > 0.0f || !this->moving)
							{
								// Keep the current facing direction.
							}
							else
							{
								vec2 rotatedFacing = vec2(RotationMatrix_2D(90.0f) * vec3(this->facing.x, this->facing.y, 0.0f));
								rotatedFacing.x = round(rotatedFacing.x);
								rotatedFacing.y = round(rotatedFacing.y);
								F32 rotatedFacingDOTvelocityDirection = dot(rotatedFacing, velocityDirection);

								if (rotatedFacingDOTvelocityDirection > 0.0f)
								{
									newFacing = rotatedFacing;
								}
								else if (rotatedFacingDOTvelocityDirection == 0.0f)
								{
									newFacing = -this->facing;
								}
								else
								{
									newFacing = -rotatedFacing;
								}
							}

							// NOTE: The next 21 lines are the the implementation that works for successfully starting the correct animation for facing directions
							//if (this->moving && (newFacing != this->facing))
							//{
							//	if (newFacing == vec2(1.0f, 0.0f))
							//	{
							//		this->animator->StartAnimation("right");
							//	}
							//	else if (newFacing == vec2(-1.0f, 0.0f))
							//	{
							//		this->animator->StartAnimation("left");
							//	}
							//	else if (newFacing == vec2(0.0f, 1.0f))
							//	{
							//		this->animator->StartAnimation("up");
							//	}
							//	else if (newFacing == vec2(0.0f, -1.0f))
							//	{
							//		this->animator->StartAnimation("down");
							//	}
							//}
							//this->facing = newFacing;
							//this->pushingForward = false;
							this->facing = newFacing;
							this->pushingForward = false;
	}
	break;

	default:
	break;
	}
}

void GameObject::Update_PostPhysics(F32 dt)
{
	switch (this->GetType())
	{
	case PlayerCharacter:
	{
							if (this->moving)
							{
								if (this->pushingForward)
								{
									if (this->facing == vec2(1.0f, 0.0f))
									{
										this->animator->StartAnimation("pushRight", false);
									}
									else if (this->facing == vec2(-1.0f, 0.0f))
									{
										this->animator->StartAnimation("pushLeft", false);
									}
									else if (this->facing == vec2(0.0f, 1.0f))
									{
										this->animator->StartAnimation("pushUp", false);
									}
									else if (this->facing == vec2(0.0f, -1.0f))
									{
										this->animator->StartAnimation("pushDown", false);
									}
								}
								else
								{
									if (this->facing == vec2(1.0f, 0.0f))
									{
										this->animator->StartAnimation("right", false);
									}
									else if (this->facing == vec2(-1.0f, 0.0f))
									{
										this->animator->StartAnimation("left", false);
									}
									else if (this->facing == vec2(0.0f, 1.0f))
									{
										this->animator->StartAnimation("up", false);
									}
									else if (this->facing == vec2(0.0f, -1.0f))
									{
										this->animator->StartAnimation("down", false);
									}
								}
							}
							else
							{
								if (this->facing == vec2(1.0f, 0.0f))
								{
									this->animator->StartAnimation("right", false);
								}
								else if (this->facing == vec2(-1.0f, 0.0f))
								{
									this->animator->StartAnimation("left", false);
								}
								else if (this->facing == vec2(0.0f, 1.0f))
								{
									this->animator->StartAnimation("up", false);
								}
								else if (this->facing == vec2(0.0f, -1.0f))
								{
									this->animator->StartAnimation("down", false);
								}
								this->animator->StopAnimation();
							}
							//DebugPrintf(512, "Player Facing: (%f, %f)\n", this->facing.x, this->facing.y);
							//DebugPrintf(512, "Pushing forward: %s\n", (this->pushingForward ? "true" : "false"));

							//vector<GameObject*> infront = RaycastAll_Line_2D(this->transform.position, this->facing, 1000.0f);
							//for (size_t i = 0; i < infront.size(); ++i)
							//{
							//	GameObject* go = infront[i];
							//	go->SetDebugState(true);
							//}
							GameObject* infront = RaycastFirst_Line_2D(this->transform.position, this->facing, 1000.0f);
							if (infront != NULL)
							{
								infront->SetDebugState(true);
							}
	}
	break;

	default:
	break;
	}
}

void GameObject::HandleEvent(Event* e)
{
	Assert(e->GetType() != ET_NULL && e->GetType() != ET_COUNT);

	switch (this->GetType())
	{
	case PlayerCharacter:
	{
							switch (e->GetType())
							{
							case ET_OnCollisionEnter:
							{
														//DebugPrintf(512, "PC   OnCollisionEnter\n");
							}
							break;

							case ET_OnCollision:
							{
												   GameObject* collidedWith = (GameObject*)e->arguments[1].AsPointer();
												   vec2 collisionNormal = e->arguments[2].AsVec2();

												   //DebugPrintf(512, "Collision Normal = (%f, %f)\n", collisionNormal.x, collisionNormal.y);
												   //DebugPrintf(512, "Player Facing = (%f, %f)\n", this->facing.x, this->facing.y);

												   if (!collidedWith->collisionShape->IsPhantom() && collisionNormal == this->facing)
												   {
													   this->pushingForward = true;
												   }
							}
							break;

							case ET_OnCollisionExit:
							{
													   //DebugPrintf(512, "PC   OnCollisionExit\n");
							}
							break;

							default:
							break;
							}
	}
	break;

	case StaticEnvironmentPiece:
	{

	}
	break;

	case PlayerCamera:
	{

	}
	break;

	case Button:
	{
				   switch (e->GetType())
				   {
				   case ET_OnCollisionEnter:
				   {
											   DebugPrintf(512, "Button   OnCollisionEnter\n");
				   }
				   break;

				   case ET_OnCollision:
				   {
										  DebugPrintf(512, "Button   OnCollision\n");
				   }
				   break;

				   case ET_OnCollisionExit:
				   {
											  DebugPrintf(512, "Button   OnCollisionExit\n");
				   }
				   break;

				   default:
				   break;
				   }
	}
	break;

	default:
	break;
	}
}

void GameObject::DebugDraw()
{
	switch (this->GetType())
	{
	case PlayerCharacter:
	{
							
	}
	break;

	case Button:
	{
				   DebugDrawRectangleSolidColor(((Rectangle_2D*)this->collisionShape)->halfDim, this->transform, vec4(0.0f, 0.0f, 1.0f, 0.3f));
	}
	break;

	default:
	break;
	}
}

bool GameObject::GetDebugState()
{
	bool result;
	result = this->debugDraw;
	return result;
}

void GameObject::SetDebugState(bool debugState)
{
	this->debugDraw = debugState;
}

void GameObject::ToggleDebugState()
{
	this->debugDraw = !this->debugDraw;
}

vector<GameObject*> GameObject::gameObjects;
vector<GameObject*> GameObject::physicsGameObjects;
vector<GameObject*> GameObject::collisionGameObjects;
vector<GameObject*> GameObject::staticCollisionGameObjects;



/*
 * Physics Updating
 */
/* NOTE: Probably include this in GameObject. */
void IntegratePhysicsObjects(F32 dt)
{
	for (size_t i = 0; i < GameObject::physicsGameObjects.size(); ++i)
	{
		GameObject* go = GameObject::physicsGameObjects[i];
		go->rigidbody->SetPosition(go->transform.position);
		go->rigidbody->Integrate(dt);
		go->transform.position = go->rigidbody->position;
	}
}

struct CollisionPair 
{
	GameObject* go1;
	GameObject* go2;
	CollisionInfo_2D info;

	CollisionPair()
	{
		this->go1 = NULL;
		this->go2 = NULL;
		this->info = CollisionInfo_2D();
	}

	CollisionPair(GameObject* go1, GameObject* go2, CollisionInfo_2D info)
	{
		this->go1 = go1;
		this->go2 = go2;
		this->info = info;
	}
};

vector<CollisionPair> GenerateContacts()
{
	vector<CollisionPair> result;

	for (size_t i = 0; i < GameObject::collisionGameObjects.size(); ++i)
	{
		for (size_t j = i + 1; j < GameObject::collisionGameObjects.size(); ++j)
		{
			GameObject* go1 = GameObject::collisionGameObjects[i];
			GameObject* go2 = GameObject::collisionGameObjects[j];
			
			/*
			 * NOTE: Still not sure the best way to handle pessimistic biases
			 * ISSUES: ideally we would like to just subtract off the amount we added to the scale from 
			 *				the distance returned on success instead of rerunning collision detection with
			 *				the regular transform after successful detection with the bias
			 *
			 *		   Should we be adding to the scale or multiplying???
			 */
			Transform biasedTransform = go1->transform;
			biasedTransform.scale += vec2(0.2f, 0.2f);
			CollisionInfo_2D ci = DetectCollision_2D(go1->collisionShape, biasedTransform, go2->collisionShape, go2->transform);
			if (ci.collided)
			{
				ci = DetectCollision_2D(go1->collisionShape, go1->transform, go2->collisionShape, go2->transform);
				result.push_back(CollisionPair(go1, go2, ci));
			}
		}

		for (size_t j = 0; j < GameObject::staticCollisionGameObjects.size(); ++j)
		{
			GameObject* go1 = GameObject::collisionGameObjects[i];
			GameObject* go2 = GameObject::staticCollisionGameObjects[j];

			/*
			* NOTE: Still not sure the best way to handle pessimistic biases
			* ISSUES: ideally we would like to just subtract off the amount we added to the scale from
			*				the distance returned on success instead of rerunning collision detection with
			*				the regular transform after successful detection with the bias
			*
			*		   Should we be adding to the scale or multiplying???
			*/
			Transform biasedTransform = go1->transform;
			biasedTransform.scale += vec2(0.2f, 0.2f);
			CollisionInfo_2D ci = DetectCollision_2D(go1->collisionShape, biasedTransform, go2->collisionShape, go2->transform);
			if (ci.collided)
			{
				ci = DetectCollision_2D(go1->collisionShape, go1->transform, go2->collisionShape, go2->transform);
				result.push_back(CollisionPair(go1, go2, ci));
			}
		}
	}

	return result;
}

bool ComparePenetrationDepth(CollisionPair a, CollisionPair b)
{
	bool result = false;

	if (a.info.distance > b.info.distance)
	{
		result = true;
	}

	return result;
}

struct Displacements 
{
	vec2 displacement1;
	vec2 displacement2;

	Displacements()
	{
		this->displacement1 = vec2(0.0f, 0.0f);
		this->displacement2 = vec2(0.0f, 0.0f);
	}
};
Displacements FixInterpenetration(GameObject* go1, GameObject* go2, CollisionInfo_2D collisionInfo)
{
	Displacements result;

	// NOTE: inverseMass == 0 == infinite mass
	if (go1->HasTag(GameObject::Environment))
	{
		vec2 displacement = collisionInfo.normal * collisionInfo.distance;
		go2->transform.position += displacement;

		result.displacement2 = displacement;
	}
	else if (go2->HasTag(GameObject::Environment))
	{
		vec2 displacement = collisionInfo.normal * collisionInfo.distance;
		go1->transform.position -= displacement;

		result.displacement1 = -displacement;
	}
	else
	{
		// NOTE: There is only 1 movable game object in the scene to testing this atm will have to wait.
		F32 pm1Mass = 1.0f;   // / pointMass1->inverseMass;
		F32 pm2Mass = 1.0f;   // / pointMass2->inverseMass;
		vec2 pm1Displacement = (pm2Mass / (pm1Mass + pm2Mass)) * collisionInfo.normal * collisionInfo.distance;
		vec2 pm2Displacement = (pm1Mass / (pm1Mass + pm2Mass)) * -collisionInfo.normal * collisionInfo.distance;
		go1->transform.position -= pm1Displacement;
		go2->transform.position -= pm2Displacement;

		result.displacement1 = -pm1Displacement;
		result.displacement2 = -pm2Displacement;
	}

	return result;
}

struct CollisionEventPair
{
	GameObject* go1;
	GameObject* go2;
	vec2 collisionNormal; // This is WRT go1

	CollisionEventPair(GameObject* go1, GameObject* go2, vec2 collisionNormal)
	{
		this->go1 = go1;
		this->go2 = go2;
		this->collisionNormal = collisionNormal;
	}

};
bool operator==(const CollisionEventPair& lhs, const CollisionEventPair& rhs)
{
	bool result = true;

	if (lhs.go1 != rhs.go1)
	{
		result = false;
	}
	else if (lhs.go2 != rhs.go2)
	{
		result = false;
	}
	// NOTE: Not sure this is a good check. If the collision normal were to change while an object is
	//			grinding around the outside of another object it could cause false negatives.
	//else if (lhs.collisionNormal != rhs.collisionNormal)
	//{
	//	result = false;
	//}

	return result;
}
bool operator!=(const CollisionEventPair& lhs, const CollisionEventPair& rhs)
{
	bool result;
	result = !(lhs == rhs);
	return result;
}

vector<CollisionEventPair> interpenetrationsFixedLastFrame = vector<CollisionEventPair>();
void FixAllInterpenetrations()
{
	vector<CollisionEventPair> interpenetrationsFixed;

	vector<CollisionPair> collisions = GenerateContacts();

	// Remove all phantoms from the contacts list because we
	//	don't want to push objects out of them we just want to know what is inside of them.
	for (size_t i = 0; i < collisions.size(); ++i)
	{
		GameObject* go1 = collisions[i].go1;
		GameObject* go2 = collisions[i].go2;
		if (go1->collisionShape->IsPhantom() || go2->collisionShape->IsPhantom())
		{
			interpenetrationsFixed.push_back(CollisionEventPair(collisions[i].go1, collisions[i].go2, collisions[i].info.normal));
			collisions.erase(collisions.begin() + i);
		}
	}


	size_t maxNumberOfIterations = collisions.size() * 10;
	size_t i = 0;
	while (i < maxNumberOfIterations)
	{
		I32 index = -1;
		F32 penetrationDepth = -1.0f;
		for (size_t j = 0; j < collisions.size(); ++j)
		{
			if (collisions[j].info.collided && collisions[j].info.distance > penetrationDepth)
			{
				index = j;
				penetrationDepth = collisions[j].info.distance;
			}
		}

		if (index == -1 || penetrationDepth <= 0.0f)
		{
			// We have fixed all interpenetrations.
			break;
		}

		interpenetrationsFixed.push_back(CollisionEventPair(collisions[index].go1, collisions[index].go2, collisions[index].info.normal));
		Displacements d = FixInterpenetration(collisions[index].go1, collisions[index].go2, collisions[index].info);
		//Assert(length(d.displacement1) + length(d.displacement2) == collisions[index].info.distance);
		collisions[index].info.distance = 0.0f;
		for (size_t k = 0; k < collisions.size(); k++)
		{
			if (k == index)
			{
				continue;
			}

			GameObject* go1 = collisions[k].go1;
			GameObject* go2 = collisions[k].go2;
			
			GameObject* t1 = collisions[index].go1;

			// We must project the displacement for that object onto the normal and then add it to the distance
			bool b1 = (go1 == collisions[index].go1);
			bool b2 = (go2 == collisions[index].go1);
			if (b1)
			{
				collisions[k].info.distance += dot(collisions[k].info.normal, d.displacement1);
			}
			if (b2)
			{
				collisions[k].info.distance -= dot(collisions[k].info.normal, d.displacement1);
			}

			bool b3 = (go1 == collisions[index].go2);
			bool b4 = (go2 == collisions[index].go2);
			if (b3)
			{
				collisions[k].info.distance += dot(collisions[k].info.normal, d.displacement2);
			}
			if (b4)
			{
				collisions[k].info.distance -= dot(collisions[k].info.normal, d.displacement2);
			}
		}

		++i;
	}


	/*
	 * Send collision events
	 */
	for (size_t i = 0; i < interpenetrationsFixed.size(); ++i)
	{
		bool processedLastFrame = false;
		for (size_t j = 0; j < interpenetrationsFixedLastFrame.size(); ++j)
		{
			if (interpenetrationsFixed[i] == interpenetrationsFixedLastFrame[j])
			{
				processedLastFrame = true;
				interpenetrationsFixedLastFrame.erase(interpenetrationsFixedLastFrame.begin() + j);
				break;
			}
		}
		if (!processedLastFrame)
		{
			GameObject* go1 = interpenetrationsFixed[i].go1;
			GameObject* go2 = interpenetrationsFixed[i].go2;
			vec2 normal = interpenetrationsFixed[i].collisionNormal;
			// Send GameObjects OnCollisionEnter Event
			Event event1;
			event1.SetType(ET_OnCollisionEnter);
			event1.arguments[0] = EventArgument((void*)go1);
			event1.arguments[1] = EventArgument((void*)go2);
			event1.arguments[2] = EventArgument(normal);
			go1->HandleEvent(&event1);

			Event event2;
			event2.SetType(ET_OnCollisionEnter);
			event2.arguments[0] = EventArgument((void*)go2);
			event2.arguments[1] = EventArgument((void*)go1);
			event2.arguments[2] = EventArgument(-normal);
			go2->HandleEvent(&event2);
		}

		// Send GameObjects OnCollision Event regardless(this should be received along with OnCollisionEnter on the first frame of a collision)
		GameObject* go1 = interpenetrationsFixed[i].go1;
		GameObject* go2 = interpenetrationsFixed[i].go2;
		vec2 normal = interpenetrationsFixed[i].collisionNormal;
		// Send GameObjects OnCollisionEnter Event
		Event event1;
		event1.SetType(ET_OnCollision);
		event1.arguments[0] = EventArgument((void*)go1);
		event1.arguments[1] = EventArgument((void*)go2);
		event1.arguments[2] = EventArgument(normal);
		go1->HandleEvent(&event1);

		Event event2;
		event2.SetType(ET_OnCollision);
		event2.arguments[0] = EventArgument((void*)go2);
		event2.arguments[1] = EventArgument((void*)go1);
		event2.arguments[2] = EventArgument(-normal);
		go2->HandleEvent(&event2);


		CollisionEventPair collision = interpenetrationsFixed[i];
		//DebugPrintf(1024, "collision processed (%i, %i)\n", collision.go1->type, collision.go2->type);
	}

	// Because we removed all of the interpenetrations that were fixed this frame too, this
	//	 list only contains interpenetrations that didn't happen this frame
	for (size_t i = 0; i < interpenetrationsFixedLastFrame.size(); ++i)
	{
		// Send GameObjects OnCollisionExit Event
		GameObject* go1 = interpenetrationsFixedLastFrame[i].go1;
		GameObject* go2 = interpenetrationsFixedLastFrame[i].go2;
		vec2 normal = interpenetrationsFixedLastFrame[i].collisionNormal;
		// Send GameObjects OnCollisionEnter Event
		Event event1;
		event1.SetType(ET_OnCollisionExit);
		event1.arguments[0] = EventArgument((void*)go1);
		event1.arguments[1] = EventArgument((void*)go2);
		event1.arguments[2] = EventArgument(normal);
		go1->HandleEvent(&event1);

		Event event2;
		event2.SetType(ET_OnCollisionExit);
		event2.arguments[0] = EventArgument((void*)go2);
		event2.arguments[1] = EventArgument((void*)go1);
		event2.arguments[2] = EventArgument(-normal);
		go2->HandleEvent(&event2);
	}

	interpenetrationsFixedLastFrame = interpenetrationsFixed;
}

GameObject* RaycastFirst_Line_2D(vec2 position, vec2 direction, F32 distance)
{
	GameObject* result = NULL;
	F32 resultDistance = distance;

	vec2 rayPerp = Perpendicular_2D(direction);
	for (size_t i = 0; i < GameObject::collisionGameObjects.size(); ++i)
	{
		GameObject* go = GameObject::collisionGameObjects[i];
		vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
		vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
		F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
		F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
		F32 goOriginDOTdirection = dot(go->transform.position - position, direction);
		if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && goOriginDOTdirection < resultDistance && goOriginDOTdirection > 0.0f)
		{
			resultDistance = goOriginDOTdirection;
			result = go;
		}
	}
	for (size_t i = 0; i < GameObject::staticCollisionGameObjects.size(); ++i)
	{
		GameObject* go = GameObject::staticCollisionGameObjects[i];
		vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
		vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
		F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
		F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
		F32 goOriginDOTdirection = dot(go->transform.position - position, direction);
		if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && goOriginDOTdirection < resultDistance && goOriginDOTdirection > 0.0f)
		{
			resultDistance = goOriginDOTdirection;
			result = go;
		}
	}


	return result;
}

vector<GameObject*> RaycastAll_Line_2D(vec2 position, vec2 direction, F32 distance)
{
	vector<GameObject*> result;

	vec2 rayPerp = Perpendicular_2D(direction);
	for (size_t i = 0; i < GameObject::collisionGameObjects.size(); ++i)
	{
		GameObject* go = GameObject::collisionGameObjects[i];
		vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
		vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
		F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
		F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
		if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && dot(go->transform.position - position, direction) < distance && dot(go->transform.position - position, direction) > 0.0f)
		{
			result.push_back(go);
		}
	}
	for (size_t i = 0; i < GameObject::staticCollisionGameObjects.size(); ++i)
	{
		GameObject* go = GameObject::staticCollisionGameObjects[i];
		vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
		vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
		F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
		F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
		if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && dot(go->transform.position - position, direction) < distance && dot(go->transform.position - position, direction) > 0.0f)
		{
			result.push_back(go);
		}
	}

	return result;
}



/*
 * Generate Prefab
 */
GameObject* CreateGameObject(GameObject::Type type)
{
	GameObject* go = new GameObject();
	go->SetType(type);
	GameObject::gameObjects.push_back(go);
	return go;
}

GameObject* CreateHero(vec2 position = vec2(0.0f, 0.0f), bool debugDraw = true)
{
	GameObject* hero = CreateGameObject(GameObject::PlayerCharacter);

	hero->transform.position = position;
	hero->AddTag(GameObject::Hero);
	hero->AddAnimator();
	hero->animator->SetSpriteOffset(vec2(0.0f, 0.4375f)); 
	hero->animator->AddAnimation("up", "link_Up", 2, 0.33f);
	hero->animator->AddAnimation("down", "link_Down", 2, 0.33f);
	hero->animator->AddAnimation("right", "link_Right", 2, 0.33f);
	hero->animator->AddAnimation("left", "link_Left", 2, 0.33f);
	hero->animator->AddAnimation("pushUp", "link_PushUp", 2, 0.33f);
	hero->animator->AddAnimation("pushDown", "link_PushDown", 2, 0.33f);
	hero->animator->AddAnimation("pushRight", "link_PushRight", 2, 0.33f);
	hero->animator->AddAnimation("pushLeft", "link_PushLeft", 2, 0.33f);
	hero->animator->StartAnimation("down");
	hero->animator->PauseAnimation();
	hero->AddRigidbody(1.0f, 0.0f, vec2(0.0f, 0.0f), vec2(0.0f, 0.0f));
	hero->AddCollisionShape(Rectangle_2D(vec2(TileDimensions.x * 0.5f, TileDimensions.y * 0.5625f), vec2(0.0f, 4.5f/16.0f)));

	// State
	hero->facing = vec2(0.0f, -1.0f);
	hero->moving = false;
	hero->pushingForward = false;

	hero->SetDebugState(debugDraw);

	return hero;
}

GameObject* CreateBackground(const char * backgroundName, vec2 position = vec2(0.0f, 0.0f), bool debugDraw = true)
{
	GameObject* bg = CreateGameObject(GameObject::StaticEnvironmentPiece);
	bg->AddTag(GameObject::Background);
	bg->transform.position = position;
	bg->transform.scale = vec2(10.0f, 8.0f);
	bg->AddSprite(backgroundName);

	bg->SetDebugState(debugDraw);

	return bg;
}

GameObject* CreateTree(vec2 position = vec2(0.0f, 0.0f), bool debugDraw = true)
{
	GameObject* tree = CreateGameObject(GameObject::StaticEnvironmentPiece);

	tree->transform.position = position;
	tree->transform.scale = vec2(2.0f, 2.0f);
	tree->AddTag(GameObject::Environment);
	tree->AddSprite("tree_Generic");
	tree->AddCollisionShape(Rectangle_2D(TileDimensions));

	tree->SetDebugState(debugDraw);

	return tree;
}

GameObject* CreateDancingFlowers(vec2 position = vec2(0.0f, 0.0f), bool debugDraw = true)
{
	GameObject* df = CreateGameObject(GameObject::StaticEnvironmentPiece);

	df->transform.position = position;
	df->AddTag(GameObject::Environment);
	df->AddAnimator();
	df->animator->AddAnimation("dance", "dancing_Flower", 4, 1.0f);
	df->animator->StartAnimation("dance");

	df->SetDebugState(debugDraw);

	return df;
}

GameObject* CreateWeed(vec2 position = vec2(0.0f, 0.0f), bool debugDraw = true)
{
	GameObject* weed = CreateGameObject(GameObject::StaticEnvironmentPiece);

	weed->transform.position = position;
	weed->AddTag(GameObject::Environment);
	weed->AddTag(GameObject::Cutable);
	weed->AddTag(GameObject::Burnable);
	weed->AddTag(GameObject::Lightweight);
	weed->AddSprite("weed");
	weed->AddCollisionShape(Rectangle_2D(TileDimensions));

	weed->SetDebugState(debugDraw);

	return weed;
}

GameObject* CreateSpookyTree(vec2 position = vec2(0.0f, 0.0f), bool debugDraw = true)
{
	GameObject* spooky = CreateGameObject(GameObject::StaticEnvironmentPiece);

	spooky->transform.position = position;
	spooky->AddTag(GameObject::Environment);
	spooky->AddSprite("tree_Spooky");
	spooky->AddCollisionShape(Rectangle_2D(TileDimensions));

	spooky->SetDebugState(debugDraw);

	return spooky;
}

GameObject* CreateBlocker(vec2 position = vec2(0.0f, 0.0f), vec2 scale = vec2(1.0f, 1.0f), bool debugDraw = true)
{
	GameObject* blocker = CreateGameObject(GameObject::StaticEnvironmentPiece);
	blocker->transform.position = position;
	blocker->transform.scale = scale;
	blocker->AddTag(GameObject::Environment);
	blocker->AddCollisionShape(Rectangle_2D(TileDimensions));

	blocker->SetDebugState(debugDraw);

	return blocker;
}

GameObject* CreateButton(vec2 position, bool debugDraw = true)
{
	GameObject* button = CreateGameObject(GameObject::Button);

	button->AddTag(GameObject::Environment);
	button->transform.position = position;
	button->AddCollisionShape(Rectangle_2D(TileDimensions, vec2(0.0f, 0.0f), true));

	button->SetDebugState(debugDraw);

	return button;
}

/*
 * Global Game State
 */
bool globalDebugDraw = false;
bool resetDebugStatePerFrame = true;
Camera camera;

#define NUMGAMEOBJECTS 1 // NOTE: LUL
GameObject* flowerGO;
GameObject* treeGO;
GameObject* heroGO;
GameObject* weedGO1;
GameObject* weedGO2;
GameObject* buttonGO;



/*
 * Rendering
 */
void DrawCameraGrid()
{
	// NOTE: Camera space grid, corresponding to tiles
	F32 initialXPos = camera.position.x - 10.0f / 2.0f;
	F32 endXPos = initialXPos + 10.0f;
	F32 posY = camera.position.y + 8.0f / 2.0f;
	F32 negY = camera.position.y - 8.0f / 2.0f;
	vec4 gridLineColor = vec4(.251f, .251f, .251f, 1.0f);
	for (F32 xPos = initialXPos; xPos <= endXPos; ++xPos)
	{
		DrawLine(vec2(xPos, posY), vec2(xPos, negY), gridLineColor, &camera);
	}

	F32 initialYPos = camera.position.y - 8.0f / 2.0f;
	F32 endYPos = initialYPos + 8.0f;
	F32 posX = camera.position.x + 10.0f / 2.0f;
	F32 negX = camera.position.x - 10.0f / 2.0f;
	for (F32 yPos = initialYPos; yPos <= endYPos; ++yPos)
	{
		DrawLine(vec2(posX, yPos), vec2(negX, yPos), gridLineColor, &camera);
	}

	DrawLine(vec2(0.0f, posY), vec2(0.0f, negY), vec4(0.09f, 0.52f, 0.09f, 1.0f), &camera);
	DrawLine(vec2(posX, 0.0f), vec2(negX, 0.0f), vec4(0.52f, 0.09f, 0.09f, 1.0f), &camera);
}

// NOTE: DebugDraw functions are shell functions that draw into the world without needing to supply the rendering camera.
//	the default rendering view is assumed
void DebugDrawPoint(vec2 p, F32 pointSize, vec4 color)
{
	DrawPoint(p, pointSize, color, &camera);
}

void DebugDrawLine(vec2 a, vec2 b, vec4 color)
{
	DrawLine(a, b, color, &camera);
}

void DebugDrawRectangleOutline(vec2 upperLeft, vec2 dimensions, vec4 color, F32 offset)
{
	DrawRectangleOutline(upperLeft, dimensions, color, &camera, offset);
}

void DebugDrawRectangleSolidColor(vec2 halfDim, Transform transform, vec4 color)
{
	DrawRectangle(halfDim, transform, color, &camera);
}

bool CompareYPosition(GameObject* a, GameObject* b)
{
	bool result;
	result = a->transform.position.y >= b->transform.position.y;
	return result;
}

void SortBinByYPosition(GameObject** bin, size_t count)
{
	MergeSort(bin, count, CompareYPosition);
}

const vec4 debugCollisionColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
void DrawGameObject(GameObject* gameObject, F32 dt)
{
	bool hasAnimations = gameObject->animator != NULL && gameObject->animator->NumberOfAnimations();
	bool hasSprite = gameObject->sprite != NULL;

	if (hasAnimations)
	{
		gameObject->animator->StepElapsedAnimationTime(dt);
		TextureHandle animationFrame = gameObject->animator->GetCurrentAnimationFrame();
		DrawSprite(animationFrame, gameObject->animator->GetSpriteOffset(), gameObject->transform, &camera);
	}
	else if (hasSprite)
	{
		DrawSprite(gameObject->sprite->texture, gameObject->sprite->offset, gameObject->transform, &camera);
	}

	if (gameObject->collisionShape != NULL && gameObject->debugDraw)
	{
		const size_t rectangleHash = typeid(Rectangle_2D).hash_code();
		const size_t circleHash = typeid(Circle_2D).hash_code();
		const size_t triangleHash = typeid(Triangle_2D).hash_code();

		size_t gameObjectHash = typeid(*(gameObject->collisionShape)).hash_code();
		if (gameObjectHash == rectangleHash)
		{
			Rectangle_2D* rect = (Rectangle_2D*)gameObject->collisionShape;
			vec2 upperLeft = gameObject->transform.position + (vec2(-rect->halfDim.x, rect->halfDim.y) * gameObject->transform.scale) + gameObject->collisionShape->GetOffset();
			vec2 dimensions = rect->halfDim * 2.0f * gameObject->transform.scale;
			DrawRectangleOutline(upperLeft, dimensions, debugCollisionColor, &camera);
		}
		else if (gameObjectHash == circleHash)
		{
			// NOTE: Draw Circle
		}
		else if (gameObjectHash == triangleHash)
		{
			vec2 trianglePoints[3];
			Triangle_2D* triangle = (Triangle_2D*)gameObject->collisionShape;
			mat3 A = gameObject->transform.LocalToWorldTransform();
			trianglePoints[0] = vec2(A * vec3(triangle->points[0].x, triangle->points[0].y, 1.0f));
			trianglePoints[1] = vec2(A * vec3(triangle->points[1].x, triangle->points[1].y, 1.0f));
			trianglePoints[2] = vec2(A * vec3(triangle->points[2].x, triangle->points[2].y, 1.0f));
			DrawLine(trianglePoints[0], trianglePoints[1], debugCollisionColor, &camera);
			DrawLine(trianglePoints[1], trianglePoints[2], debugCollisionColor, &camera);
			DrawLine(trianglePoints[2], trianglePoints[0], debugCollisionColor, &camera);
		}
	}

	if (gameObject->debugDraw)
	{
		DrawPoint(gameObject->transform.position, 4, vec4(0.0f, 0.0f, 1.0f, 1.0f), &camera);
	}
}

void DrawGameObjects(F32 dt)
{
	// NOTE: Draw the game space.
	SetViewport(vec2(0, 0), vec2(600, 480));

	/*
	 * Sort by tag
	 *		Bins
	 *		1 - Background
	 *		2 - Environment
	 *		3 - Everything else sans effects
	 *		4 - Effects
	 * 
	 * Sort bins by position.y (Lowest -> Highest)
	 *
	 * Draw all objects in each bin starting with Bin 1.
	 */
	vector<GameObject*> bin_Background;
	vector<GameObject*> bin_Environment;
	vector<GameObject*> bin_Characters;
	vector<GameObject*> bin_Effects;
	for (size_t i = 0; i < GameObject::gameObjects.size(); ++i)
	{
		GameObject* go = GameObject::gameObjects[i];
		bool hasTag_Background = go->HasTag(GameObject::Background);
		bool hasTag_Environment = go->HasTag(GameObject::Environment);
		bool hasTag_Effect = go->HasTag(GameObject::Effect);

		if (hasTag_Background)
		{
			bin_Background.push_back(go);
		}
		if (hasTag_Environment)
		{
			bin_Environment.push_back(go);
		}
		if (hasTag_Effect)
		{
			bin_Effects.push_back(go);
		}

		if (!hasTag_Background && !hasTag_Environment && !hasTag_Effect)
		{
			bin_Characters.push_back(go);
		}
	}

	SortBinByYPosition(bin_Background._Myfirst, bin_Background.size());
	SortBinByYPosition(bin_Environment._Myfirst, bin_Environment.size());
	SortBinByYPosition(bin_Characters._Myfirst, bin_Characters.size());
	SortBinByYPosition(bin_Effects._Myfirst, bin_Effects.size());

	// Draw Objects
	for (size_t i = 0; i < bin_Background.size(); ++i)
	{
		GameObject* go = bin_Background[i];
		DrawGameObject(go, dt);
		go->DebugDraw();
	}

	if (globalDebugDraw)
	{
		DrawCameraGrid();
	}

	for (size_t i = 0; i < bin_Environment.size(); ++i)
	{
		GameObject* go = bin_Environment[i];
		DrawGameObject(go, dt);
		go->DebugDraw();
	}

	for (size_t i = 0; i < bin_Characters.size(); ++i)
	{
		GameObject* go = bin_Characters[i];
		DrawGameObject(go, dt);
		go->DebugDraw();
	}

	for (size_t i = 0; i < bin_Effects.size(); ++i)
	{
		GameObject* go = bin_Effects[i];
		DrawGameObject(go, dt);
		go->DebugDraw();
	}


	// Draw chunk outlines
	/*for (size_t x = 0; x < numberOfHorizontalChunks; ++x)
	{
		for (size_t y = 0; y < numberOfVerticalChunks; ++y)
		{
			DrawRectangleOutline(chunks[x][y].position + vec2(0, chunkHeight), vec2(chunkWidth, chunkHeight), vec4(1, 0, 0, 1), &camera);
		}
	}*/
	
	// NOTE: Draw UI
	/*DrawUVRectangleScreenSpace(&guiPanel, vec2(0, 0), vec2(guiPanel.width, guiPanel.height));*/

}



/*
 * Core Game API Functions
 */
bool GameInitialize()
{
	// Initialize game sub systems.
	InitializeRenderer();

	SetWindowTitle("Oracle of Ages Clone");
	SetClientWindowDimensions(vec2(600, 540));
	//SetClearColor(vec4(0.32f, 0.18f, 0.66f, 0.0f));
	SetClearColor(vec4(0.2235f, 0.2235f, 0.2235f, 1.0f));

	spriteAssetFilepathTable.Initialize(60);
	ReadInSpriteAssets(&spriteAssetFilepathTable, "Assets.txt");

	bool debugDraw = globalDebugDraw;
	CreateBackground("background_10-5", vec2(0.0f, 0.0f), debugDraw);
	CreateDancingFlowers(vec2(-0.5f, -1.5f), debugDraw);
	CreateDancingFlowers(vec2(-2.5f, 1.5f), debugDraw);
	CreateWeed(vec2(-2.5f, -0.5f), debugDraw); // Left Group Start
	CreateWeed(vec2(-3.5f, -0.5f), debugDraw);
	CreateWeed(vec2(-3.5f, -1.5f), debugDraw);
	CreateWeed(vec2( 2.5f, -0.5f), debugDraw); // Right Group Start
	CreateWeed(vec2( 1.5f, -1.5f), debugDraw);
	CreateWeed(vec2( 2.5f, -1.5f), debugDraw);
	CreateWeed(vec2( 1.5f, -2.5f), debugDraw);
	CreateWeed(vec2( 2.5f, -2.5f), debugDraw);
	CreateTree(vec2(-3.0f,  4.0f), debugDraw); // Left Group Start
	CreateTree(vec2(-5.0f,  4.0f), debugDraw);
	CreateTree(vec2(-5.0f,  2.0f), debugDraw);
	CreateTree(vec2(-5.0f,  0.0f), debugDraw);
	CreateTree(vec2(-5.0f, -2.0f), debugDraw);
	CreateTree(vec2( 4.0f,  4.0f), debugDraw); // Right Group Start
	CreateTree(vec2( 4.0f,  2.0f), debugDraw);
	CreateTree(vec2( 4.0f,  0.0f), debugDraw);
	CreateTree(vec2( 4.0f, -2.0f), debugDraw);
	CreateBlocker(vec2(0.0f, -3.5f), vec2(10.0f, 1.0f), debugDraw); // Bottom Wall

	heroGO = CreateHero(/*vec2(0.75f, -1.68f)*/vec2(-0.5f, 0.5f), debugDraw);

	buttonGO = CreateButton(vec2(1.5f, 1.5f), debugDraw);

	
	camera.halfDim = vec2(5.0f, 4.0f);
	camera.position = vec2(0.0f, 0.0f);
	camera.rotationAngle = 0.0f;
	camera.scale = 1.0f;

	return true;
}

void UpdateGameObjects_PrePhysics(F32 dt)
{
	for (size_t i = 0; i < GameObject::gameObjects.size(); ++i)
	{
		GameObject::gameObjects[i]->Update_PrePhysics(dt);
	}
}

void UpdateGameObjects_PostPhysics(F32 dt)
{
	for (size_t i = 0; i < GameObject::gameObjects.size(); ++i)
	{
		GameObject::gameObjects[i]->Update_PostPhysics(dt);
	}
}

void DebugDrawGameObjects()
{
	for (size_t i = 0; i < GameObject::gameObjects.size(); ++i)
	{
		GameObject::gameObjects[i]->DebugDraw();
	}
}

F32 time = 0.0f;
F32 skipThreshold = 0.5f;
void GameUpdate(F32 dt)
{
	if (dt > skipThreshold)
	{
		return;
	}

	if (GetKeyDown(KeyCode_1))
	{
		globalDebugDraw = !globalDebugDraw;
		for (size_t i = 0; i < GameObject::gameObjects.size(); ++i)
		{
			GameObject* go = GameObject::gameObjects[i];
			go->SetDebugState(globalDebugDraw);
		}
	}

	if (GetKeyDown(KeyCode_2))
	{
		resetDebugStatePerFrame = !resetDebugStatePerFrame;
	}

	//dt = 0.0166f;// *2.0f;
	//if (heroGO->transform.position.y < 3.0f)
	//{
	//	time += dt;
	//}
	//else
	//{
	//	F32 i = 0;
	//}

	// NOTE: Get rid of this just for testing IP
	//time += dt;
	//if (heroGO->transform.position.y >= 2.0f)
	//{
	//	I32 g = 0;
	//}


	//DebugPrintf(512, "delta time = %f\n", dt);
	//heroGO->rigidbody->ApplyImpulse(vec2(1.0f, 0.0f), 1.5f);
 	Clear();
	UpdateGameObjects_PrePhysics(dt);
	IntegratePhysicsObjects(dt);
	FixAllInterpenetrations();

	/*
	 * Tests
	 */
	//Transform biasedTransform = heroGO->transform;
	//biasedTransform.scale *= 1.2f;
	//CollisionInfo_2D ci = DetectCollision_2D(heroGO->collisionShape, biasedTransform/*heroGO->transform*/, treeGO->collisionShape, treeGO->transform);
	//vec2 perp = Perpendicular_2D(vec2(0.0f, 1.0f));


	


	UpdateGameObjects_PostPhysics(dt);
	DrawGameObjects(dt);


	// NOTE: For ray testing
	if (resetDebugStatePerFrame)
	{
		for (size_t i = 0; i < GameObject::gameObjects.size(); ++i)
		{
			GameObject* go = GameObject::gameObjects[i];
			go->SetDebugState(false);
		}
	}
}

bool GameShutdown()
{
	spriteAssetFilepathTable.Destroy();
	ShutdownRenderer();
	return true;
}