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

	bool StartAnimation(const char* referenceName, bool loopAnimation = true, bool playInReverse = false, F32 startTime = 0.0f)
	{
		AnimationHash::GetKeyIndexResult ki = this->animations.GetKeyIndex(referenceName);
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

	RigidBody()
	{
		this->position = vec2(0.0f, 0.0f);
		this->velocity = vec2(0.0f, 0.0f);
		this->forceAccumulator = vec2(0.0f, 0.0f);
		this->inverseMass = 0.0f;
		this->dampingFactor = DEFAULT_DAMPING_FACTOR;
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
	};

	void Destroy()
	{
		this->position = vec2(0.0f, 0.0f);
		this->velocity = vec2(0.0f, 0.0f);
		this->forceAccumulator = vec2(0.0f, 0.0f);

		this->inverseMass = 0.0f;
		this->dampingFactor = DEFAULT_DAMPING_FACTOR;
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
		this->position = this->position + (this->velocity * deltaTime);
		this->velocity = (this->velocity * pow(this->dampingFactor, deltaTime)) + (acceleration * deltaTime);
		this->forceAccumulator = vec2(0.0f, 0.0f);
	}

	void ApplyForce(vec2 direction, F32 power)
	{
		vec2 scaledForce = direction * power;
		this->forceAccumulator += scaledForce;
	}

	void ApplyImpulse(vec2 direction, F32 power)
	{
		vec2 scaledVelocity = direction * power;
		this->velocity += scaledVelocity;
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
	bool drawCollisionShapeOutline;

	Type type;
	U64  tags;

	Transform transform;
	Sprite*   sprite;
	AnimationController* animator;
	RigidBody* rigidbody;
	Shape_2D* collisionShape;

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

	static vector<GameObject*> gameObjects;
	static vector<GameObject*> physicsGameObjects;
	static vector<GameObject*> collisionGameObjects;

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
	collisionGameObjects.push_back(this);
}

void GameObject::Update_PrePhysics(F32 dt)
{
	switch (this->type)
	{
	case PlayerCharacter:
	{
							if (GetKey(KeyCode_A))
							{
								this->rigidbody->ApplyImpulse(vec2(-1.0f, 0.0f), 100.0f * dt);
							}
							if (GetKey(KeyCode_D))
							{
								this->rigidbody->ApplyImpulse(vec2(1.0f, 0.0f), 100.0f * dt);
							}
							if (GetKey(KeyCode_W))
							{
								this->rigidbody->ApplyImpulse(vec2(0.0f, 1.0f), 100.0f * dt);
							}
							if (GetKey(KeyCode_S))
							{
								this->rigidbody->ApplyImpulse(vec2(0.0f, -1.0f), 100.0f * dt);
							}
	}
	break;

	default:
	break;
	}
}

void GameObject::Update_PostPhysics(F32 dt)
{

}

vector<GameObject*> GameObject::gameObjects;
vector<GameObject*> GameObject::physicsGameObjects;
vector<GameObject*> GameObject::collisionGameObjects;



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

			CollisionInfo_2D ci = DetectCollision_2D(go1->collisionShape, go1->transform, go2->collisionShape, go2->transform);
			if (ci.collided)
			{
				// Resolve Interpenetration
				//FixInterpenetration(go1, go2, ci);
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

void FixInterpenetration(GameObject* go1, GameObject* go2, CollisionInfo_2D collisionInfo)
{
	// NOTE: inverseMass == 0 == infinite mass
	if (go1->HasTag(GameObject::Environment))
	{
		vec2 displacement = collisionInfo.normal * collisionInfo.distance;
		go2->transform.position += displacement;
	}
	else if (go2->HasTag(GameObject::Environment))
	{
		vec2 displacement = collisionInfo.normal * collisionInfo.distance;
		go1->transform.position -= displacement;
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
	}
}

void FixAllInterpenetrations()
{
	vector<CollisionPair> collisions = GenerateContacts();
	size_t maxNumberOfIterations = collisions.size() * 5;
	for (size_t i = 0; i < maxNumberOfIterations; ++i)
	{
		MergeSort(collisions._Myfirst, collisions.size(), ComparePenetrationDepth);
		if (collisions.size() > 1)
		{
			Assert(collisions[0].info.distance >= collisions[1].info.distance);
		}

		if (collisions.size() == 0 || collisions[0].info.distance <= 0.0f)
		{
			break;
		}

		FixInterpenetration(collisions[0].go1, collisions[0].go2, collisions[0].info);

		collisions.clear();
		collisions = GenerateContacts();
	}
}



/*
 * Generate Prefab
 */
GameObject* CreateGameObject()
{
	GameObject* go = new GameObject();
	GameObject::gameObjects.push_back(go);
	return go;
}

GameObject* CreateHero(vec2 position = vec2(0.0f, 0.0f), bool debugDraw = true)
{
	GameObject* hero = CreateGameObject();

	hero->transform.position = position;
	hero->SetType(GameObject::PlayerCharacter);
	hero->AddTag(GameObject::Hero);
	hero->AddAnimator();
	hero->animator->AddAnimation("up", "link_Up", 2, 0.33f);
	hero->animator->AddAnimation("down", "link_Down", 2, 0.33f);
	hero->animator->AddAnimation("right", "link_Right", 2, 0.33f);
	hero->animator->AddAnimation("left", "link_Left", 2, 0.33f);
	hero->animator->StartAnimation("right");
	hero->animator->PauseAnimation();
	hero->AddRigidbody(1.0f, 0.0f, vec2(0.0f, 0.0f), vec2(0.0f, 0.0f));
	hero->AddCollisionShape(Rectangle_2D(TileDimensions));

	hero->drawCollisionShapeOutline = debugDraw;

	return hero;
}

GameObject* CreateTree(vec2 position = vec2(0.0f, 0.0f), bool debugDraw = true)
{
	GameObject* tree = CreateGameObject();

	tree->transform.position = position;
	tree->transform.scale = vec2(2.0f, 2.0f);
	tree->SetType(GameObject::StaticEnvironmentPiece);
	tree->AddTag(GameObject::Environment);
	tree->AddSprite("tree_Generic");
	tree->AddCollisionShape(Rectangle_2D(TileDimensions));

	tree->drawCollisionShapeOutline = debugDraw;

	return tree;
}

GameObject* CreateDancingFlowers(vec2 position = vec2(0.0f, 0.0f), bool debugDraw = true)
{
	GameObject* df = CreateGameObject();

	df->transform.position = position;
	df->SetType(GameObject::StaticEnvironmentPiece);
	df->AddTag(GameObject::Environment);
	df->AddAnimator();
	df->animator->AddAnimation("dance", "dancing_Flower", 4, 1.0f);
	df->animator->StartAnimation("dance");

	df->drawCollisionShapeOutline = debugDraw;

	return df;
}

GameObject* CreateWeed(vec2 position = vec2(0.0f, 0.0f), bool debugDraw = true)
{
	GameObject* weed = CreateGameObject();

	weed->transform.position = position;
	weed->SetType(GameObject::PlayerCamera);
	//weed->AddTag(GameObject::Environment);
	weed->AddTag(GameObject::Cutable);
	weed->AddTag(GameObject::Burnable);
	weed->AddTag(GameObject::Lightweight);
	weed->AddSprite("weed");
	weed->AddCollisionShape(Rectangle_2D(TileDimensions));

	weed->drawCollisionShapeOutline = debugDraw;

	return weed;
}

GameObject* CreateSpookyTree(vec2 position = vec2(0.0f, 0.0f), bool debugDraw = true)
{
	GameObject* spooky = CreateGameObject();

	spooky->transform.position = position;
	spooky->SetType(GameObject::StaticEnvironmentPiece);
	spooky->AddTag(GameObject::Environment);
	spooky->AddSprite("tree_Spooky");
	spooky->AddCollisionShape(Rectangle_2D(TileDimensions));

	spooky->drawCollisionShapeOutline = debugDraw;

	return spooky;
}



/*
 * Global Game State
 */
#define NUMGAMEOBJECTS 1
GameObject* flowerGO;
GameObject* treeGO;
GameObject* heroGO;
Camera camera;

bool GameInitialize()
{
	// Initialize game sub systems.
	InitializeRenderer();

	SetWindowTitle("Oracle of Ages Clone");
	SetClientWindowDimensions(vec2(600, 540));
	SetClearColor(vec4(0.32f, 0.18f, 0.66f, 0.0f));

	spriteAssetFilepathTable.Initialize(60);
	ReadInSpriteAssets(&spriteAssetFilepathTable, "Assets.txt");

	flowerGO = CreateDancingFlowers(vec2(-2.0f, 1.0f));
	//treeGO = CreateTree(vec2(2.0f, -1.0f), false);
	heroGO = CreateHero(vec2(-2.0f, 0.0f), true);
	CreateTree(vec2(3.0f, 0.0f), true);
	//CreateTree(vec2(0.0f, 1.0f), true);
	CreateWeed(vec2(1.0f, 0.0f), true);
	//CreateWeed(vec2(-3.0f, 1.0f));

	camera.halfDim = vec2(5.0f, 4.0f);
	camera.position = vec2(0.0f, 0.0f);
	camera.rotationAngle = 0.0f;
	camera.scale = 1.0f;

	return true;
}



/*
 * Rendering
 */
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
		DrawSprite(animationFrame, vec2(0.0f, 0.0f), gameObject->transform, &camera);
	}
	else if (hasSprite)
	{
		DrawSprite(gameObject->sprite->texture, gameObject->sprite->offset, gameObject->transform, &camera);
	}

	if (gameObject->collisionShape != NULL && gameObject->drawCollisionShapeOutline)
	{
		const size_t rectangleHash = typeid(Rectangle_2D).hash_code();
		const size_t circleHash = typeid(Circle_2D).hash_code();
		const size_t triangleHash = typeid(Triangle_2D).hash_code();

		size_t gameObjectHash = typeid(*(gameObject->collisionShape)).hash_code();
		if (gameObjectHash == rectangleHash)
		{
			Rectangle_2D* rect = (Rectangle_2D*)gameObject->collisionShape;
			vec2 upperLeft = gameObject->transform.position + (vec2(-rect->halfDim.x, rect->halfDim.y) * gameObject->transform.scale);
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
	}

	for (size_t i = 0; i < bin_Environment.size(); ++i)
	{
		GameObject* go = bin_Environment[i];
		DrawGameObject(go, dt);
	}

	for (size_t i = 0; i < bin_Characters.size(); ++i)
	{
		GameObject* go = bin_Characters[i];
		DrawGameObject(go, dt);
	}

	for (size_t i = 0; i < bin_Effects.size(); ++i)
	{
		GameObject* go = bin_Effects[i];
		DrawGameObject(go, dt);
	}


	// Draw chunk outlines
	/*for (size_t x = 0; x < numberOfHorizontalChunks; ++x)
	{
		for (size_t y = 0; y < numberOfVerticalChunks; ++y)
		{
			DrawRectangleOutline(chunks[x][y].position + vec2(0, chunkHeight), vec2(chunkWidth, chunkHeight), vec4(1, 0, 0, 1), &camera);
		}
	}*/

	// NOTE: Camera space grid, corresponding to tiles
	/*F32 initialXPos = camera.position.x - chunkWidth / 2.0f;
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
	}*/
	
	// NOTE: Draw UI
	/*DrawUVRectangleScreenSpace(&guiPanel, vec2(0, 0), vec2(guiPanel.width, guiPanel.height));*/

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

void GameUpdate(F32 dt)
{
	vec2 deltaMovement = heroGO->transform.position;

	Clear();
	UpdateGameObjects_PrePhysics(dt);
	IntegratePhysicsObjects(dt);
	FixAllInterpenetrations();

	// NOTE: Test for collision detection
	/*CollisionInfo_2D ci = DetectCollision_2D(heroGO->collisionShape, heroGO->transform, treeGO->collisionShape, treeGO->transform);
	if (ci.collided)
	{
		FixInterpenetration(heroGO, treeGO, ci);
	}*/

	UpdateGameObjects_PostPhysics(dt);
	DrawGameObjects(dt);

	deltaMovement = heroGO->transform.position - deltaMovement;
	//DebugPrintf(1024, "hero movement distance = %f\n", length(deltaMovement));
	//DebugPrintf(1024, "hero movement = (%f, %f)\n", deltaMovement.x, deltaMovement.y);
	//DebugPrintf(1024, "delta time = %f\n", dt);
	F32 fps = 1.0f / dt;
	vec2 normalizedMovement = deltaMovement * fps;
	DebugPrintf(1024, "normalized hero movement = (%f, %f)\n", normalizedMovement.x, normalizedMovement.y);
}

bool GameShutdown()
{
	spriteAssetFilepathTable.Destroy();
	ShutdownRenderer();
	return true;
}