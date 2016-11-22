#pragma once

#include "Types.h"
#include "Math.h"

#include "AssetNameToFilepathTable.h"
#include "Renderer.h"
#include "CollisionDetection2DObj.h"
#include "Event.h"
#include "BitManip.h"

#include "_GameplayAPI.h"


#include <vector>
using std::vector;


/*
 * GameObject Components
 */
struct Sprite
{
	TextureHandle texture;
	vec2 offset;

	Sprite();

	void Initialize(const char* assetName, vec2 offset);
	void Load();
	void Unload();
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

	void Initialize(const char* animationFolderPath, U32 numberOfFrames, F32 animationTime);
	void Destroy();
	void Load();
	void Unload();
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

	void Initialize(U32 numberOfIndices);

	struct GetKeyIndexResult
	{
		bool present;
		U32 i;
		U32 j;

		GetKeyIndexResult() : present(false), i(0), j(0) {};
	};
	GetKeyIndexResult GetKeyIndex(const char* key);
	const char* GetKey(size_t i, size_t j);
	Animation* GetValue(const char* key);
	void AddKVPair(const char* key, Animation* val);
	void RemoveKVPair(const char* key);
	size_t Length();
	void Destroy();
	void Load();
	void Unload();
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

	AnimationController();

	void Initialize();
	/* NOTE: Should this chain be returning const char *???*/
	const char * ActiveAnimation();
	void AddAnimation(const char* referenceName, const char* assetName, U32 numberOfFrames, F32 animationTime);
	void RemoveAnimation(const char* referenceName);
	void StartAnimation();
	bool StartAnimation(const char* referenceName, bool restartAnimation = true, bool loopAnimation = true, bool playInReverse = false, F32 startTime = 0.0f);
	bool IsPaused();
	void PauseAnimation();
	void StopAnimation();
	// ?? Useful?
	void ReverseAnimation(bool reverse = true);
	void StepElapsedAnimationTime(F32 time);
	void SetElapsedAnimationTime(F32 time);
	// 0 == 0%, 100 == 100%
	void SetElapsedAnimationTimeAsPercent(F32 percent);
	size_t NumberOfAnimations();
	TextureHandle GetCurrentAnimationFrame();
	void SetSpriteOffset(vec2 spriteOffset);
	vec2 GetSpriteOffset();
	void Load();
	void Unload();
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

	RigidBody();

	void Initialize(vec2 position, vec2 velocity, vec2 force, F32 inverseMass, F32 dampingFactor);
	void Destroy();
	F32 GetMass();
	vec2 GetVelocityForFrame();
	vec2 GetAcceleration();
	void SetPosition(vec2 position);
	void Integrate(F32 deltaTime);
	void ApplyForce(vec2 direction, F32 power);
	// directions should be normalized
	void ApplyImpulse(vec2 direction, F32 power);
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

	// NOTE: These are still here because I'm not sure if this shell is a better idea or 
	//	if this should be registered and batched to kill a bunch of ifs
	//void Update_PrePhysics(F32 dt);
	//void Update_PostPhysics(F32 dt);
	//void HandleEvent(Event* e);
	//void DebugDraw();

	void (*Update_Pre)(GameObject* go, F32 dt);
	void (*Update_Post)(GameObject* go, F32 dt);
	void (*DoEvent)(GameObject* go, Event* e);
	void (*Debug)(GameObject* go);

	static vector<GameObject*> physicsGameObjects; // NOTE: still not sure if this should be per world or global, updating all. For now its global

	static void IntegratePhysicsObjects(F32 dt);
};