#pragma once

#include "Types.h"
#include "Math.h"

#include "AssetNameToFilepathTable.h"
#include "Renderer.h"
#include "CollisionDetection2DObj.h"
#include "Event.h"
#include "BitManip.h"

#include "_GameplayAPI.h"

extern const vec2 TileDimensions;
extern const vec2 ScreenDimensions;

#include <vector>
using std::vector;

extern StringStringHashTable spriteAssetFilepathTable;

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
		const char* filePath = spriteAssetFilepathTable.GetValue(assetName);
		if (filePath == NULL)
		{
			filePath = spriteAssetFilepathTable.GetValue("missing_Texture"); //AssetNameToFilepath("missing_Texture");
		}
		this->texture = AddToTexturePool(filePath);
		this->offset = offset;
	}

	void Load()
	{
		bool isLoaded = IsTextureLoaded(this->texture);
		if (!isLoaded)
		{
			LoadTexture(this->texture);
		}
	}

	void Unload()
	{
		bool isLoaded = IsTextureLoaded(this->texture);
		if (isLoaded)
		{
			UnloadTexture(this->texture);
		}
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

	void Load()
	{
		for (size_t i = 0; i < this->numberOfFrames; ++i)
		{
			bool isLoaded = IsTextureLoaded(this->frames[i]);
			if (!isLoaded)
			{
				LoadTexture(this->frames[i]);
			}
		}
	}

	void Unload()
	{
		for (size_t i = 0; i < this->numberOfFrames; ++i)
		{
			bool isLoaded = IsTextureLoaded(this->frames[i]);
			if (isLoaded)
			{
				UnloadTexture(this->frames[i]);
			}
		}
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

	void Load()
	{
		for (size_t i = 0; i < this->numberOfIndices; ++i)
		{
			AnimationHashLink* row = this->table[i];
			U32 rowLength = this->subListLength[i];
			for (size_t j = 0; j < rowLength; ++j)
			{
				AnimationHashLink* entry = &(row[j]);

				entry->v->Load();
			}
		}
	}

	void Unload()
	{
		for (size_t i = 0; i < this->numberOfIndices; ++i)
		{
			AnimationHashLink* row = this->table[i];
			U32 rowLength = this->subListLength[i];
			for (size_t j = 0; j < rowLength; ++j)
			{
				AnimationHashLink* entry = &(row[j]);

				entry->v->Unload();
			}
		}
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

	void Load()
	{
		this->animations.Load();
	}

	void Unload()
	{
		this->animations.Unload();
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
 * GameObject
 */
enum GameObjectType
{
	Null,

	PlayerCharacter,
	StaticEnvironmentPiece,
	PlayerCamera,
	CameraTetherPoint,
	TransitionBar,
	Fire,


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

enum GameObjectTags
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

struct GameObject
{
	// Debug Variables
	bool forceUpdates;
	bool debugDraw;

	GameObjectType type;
	U64  tags;

	Transform transform;
	Sprite*   sprite;
	AnimationController* animator;
	RigidBody* rigidbody;
	Shape_2D* collisionShape;
	Camera* camera;

	// State
	// NOTE: This needs to be dealt with.
	// Player
	vec2 facing;
	bool moving;
	F32 movementSpeed;
	F32 slowPercentage;
	bool isSlowed;
	bool pushingForward;
	bool frozen;
	bool showRay;

	// Fire
	F32 lifetime;

	// Camera
	GameObject* tetherPoint;
	bool bound;
	GameObject* target;


	GameObject()
	{
		this->forceUpdates = false;
		this->type = Null;
		this->tags = 0;
		this->transform = Transform();
		this->sprite = NULL;
		this->animator = NULL;
		this->rigidbody = NULL;
		this->collisionShape = NULL;
		this->camera = NULL;
		this->Update_Pre = NULL;
		this->Update_Post = NULL;
		this->DoEvent = NULL;
		this->Debug = NULL;
	}

	void ForceUpdates(bool b);
	bool ForcesUpdates();

	void SetType(GameObjectType type);
	GameObjectType GetType();

	void AddTag(GameObjectTags tag);
	void RemoveTag(GameObjectTags tag);
	void ClearAllTags();
	bool HasTag(GameObjectTags tag);

	void AddSprite(const char * sprite);
	void AddAnimator();
	void AddRigidbody(F32 invmass = 0.0f, F32 dampingfactor = 0.0f, vec2 velocity = vec2(0.0f, 0.0f), vec2 force = vec2(0.0f)); // NOTE: Rigidbody should be added BEFORE collision shapes
	template <typename S>
	void AddCollisionShape(S shape);
	void AddCamera(vec2 halfDim);

	void RemoveSprite();
	void RemoveAnimator();
	void RemoveRigidbody();
	void RemoveCollisionShape(); // NOTE: Rigidbody should be removed AFTER collision shapes
	void RemoveCamera();

	//void Update_PrePhysics(F32 dt);
	//void Update_PostPhysics(F32 dt);
	//void HandleEvent(Event* e);
	//void DebugDraw();

	void (*Update_Pre)(GameObject* go, F32 dt);
	void (*Update_Post)(GameObject* go, F32 dt);
	void (*DoEvent)(GameObject* go, Event* e);
	void (*Debug)(GameObject* go);

	bool GetDebugState();
	void SetDebugState(bool debugState);
	void ToggleDebugState();

	static vector<GameObject*> gameObjects;
	static vector<GameObject*> gameObjectDestructionQueue;

	static vector<GameObject*> physicsGameObjects;
	static vector<GameObject*> collisionGameObjects;
	static vector<GameObject*> staticCollisionGameObjects;
	static vector<GameObject*> phantomCollisionGameObjects;

};

void SetRenderCamera(Camera* camera);
void GameObject::AddCamera(vec2 halfDim)
{
	this->camera = new Camera();
	this->camera->position = this->transform.position;
	this->camera->scale = 1.0f;
	this->camera->rotationAngle = this->transform.rotationAngle;
	this->camera->ResizeViewArea(halfDim);
	SetRenderCamera(this->camera);
}