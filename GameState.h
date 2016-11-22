#pragma once

#include "Types.h"
#include "Math.h"
#include "Util.h"

#include "_SystemAPI.h"
#include "_GameplayAPI.h"

#include "AssetNameToFilepathTable.h"
#include "Event.h"
#include "CollisionDetection2DObj.h"
#include "CollisionWorld2D.h"
#include "GameMap.h"

#include <vector>
using std::vector;




struct Space
{
	Transform transform;
	Rectangle_2D collisionRectangle;

	vec2 collisionRectangle_MINDIMENSIONS;
	vec2 collisionRectangle_MAXDIMENSIONS;

	void SetDimensionRange(vec2 min, vec2 max)
	{
		this->collisionRectangle_MINDIMENSIONS = min;
		this->collisionRectangle_MAXDIMENSIONS = max;
	}
	vec2 GetMinDimension()
	{
		vec2 result;

		result = this->collisionRectangle_MINDIMENSIONS;

		return result;
	}
	vec2 GetMaxDimension()
	{
		vec2 result;

		result = this->collisionRectangle_MAXDIMENSIONS;

		return result;
	}

	void SetPosition(vec2 position)
	{
		this->transform.position = position;
	}
	vec2 GetPosition()
	{
		vec2 result;

		result = this->transform.position;

		return result;
	}

	void SetHalfDimensions(vec2 halfDimensions)
	{
		halfDimensions.x = ClampRange_F32(halfDimensions.x, collisionRectangle_MINDIMENSIONS.x / 2.0f, collisionRectangle_MAXDIMENSIONS.x / 2.0f);
		halfDimensions.y = ClampRange_F32(halfDimensions.y, collisionRectangle_MINDIMENSIONS.y / 2.0f, collisionRectangle_MAXDIMENSIONS.y / 2.0f);

		this->collisionRectangle = Rectangle_2D(halfDimensions, vec2(0.0f, 0.0f), true);
	}
	vec2 GetHalfDimensions()
	{
		vec2 result;

		result = this->collisionRectangle.halfDim;

		return result;
	}

	void SetDimensions(vec2 dimensions)
	{
		dimensions.x = ClampRange_F32(dimensions.x, collisionRectangle_MINDIMENSIONS.x, collisionRectangle_MAXDIMENSIONS.x);
		dimensions.y = ClampRange_F32(dimensions.y, collisionRectangle_MINDIMENSIONS.y, collisionRectangle_MAXDIMENSIONS.y);

		vec2 halfDim = dimensions / 2.0f;
		this->collisionRectangle = Rectangle_2D(halfDim, vec2(0.0f, 0.0f), true);
	}
	vec2 GetDimensions()
	{
		vec2 result;

		result = this->collisionRectangle.halfDim * 2.0f;

		return result;
	}

	void Set(vec2 position, vec2 dimensions)
	{
		dimensions.x = ClampRange_F32(dimensions.x, collisionRectangle_MINDIMENSIONS.x, collisionRectangle_MAXDIMENSIONS.x);
		dimensions.y = ClampRange_F32(dimensions.y, collisionRectangle_MINDIMENSIONS.y, collisionRectangle_MAXDIMENSIONS.y);

		vec2 halfDim = dimensions / 2.0f;
		this->collisionRectangle = Rectangle_2D(halfDim, vec2(0.0f, 0.0f), true);
		this->transform.position = position;
	}
};

struct QueuedEvent
{
	GameObject* recipient;
	Event* e;
	U32 numberOfFramesToWait;

	QueuedEvent(GameObject* recipient, Event* e, U32 numberOfFramesToWait)
	{
		this->recipient = recipient;
		this->e = new Event(e);
		this->numberOfFramesToWait = numberOfFramesToWait;
	}
};

struct GameState
{
	// Maps
	GameMap* currentMap;
	vector<GameMap> maps;
	const vec2 TileDimensions = vec2(0.5f, 0.5f);
	const vec2 ScreenSize = vec2(10.0f, 8.0f);
	const vec2 XLScreenSize = vec2(15.0f, 11.0f);

	// Simulation
	Space simulationSpace;
	GameObject* simulationSpaceTarget;

	// Loading
	Space loadSpace;
	const F32 loadSpace_SIZEMULTIPLIER = 3.125f;

	// Assets
	StringStringHashTable spriteAssetFilepathTable;

	// Interpenetration
	vector<PenetrationInfo_2D> interpenetrationsFixedLastFrame;
	vector<PenetrationInfo_2D> phantomInterpenetrationsFromLastFrame;

	// Update Loop Control
	bool pauseAnimations = false;

	bool freezeNextFrame = false;
	bool gameFrozen = false;
	F32 freezeTimer = 0.0f;
	F32 freezeLength = 0;

	// Events
	// NOTE: This should be folded into its own postmaster class
	vector<QueuedEvent> queuedEventsToProcess;
	vector<QueuedEvent> queuedEventsFromThisFrame;

	// Debug
	const F32 skipThreshold = 1.0f;
	bool showCollisionVolumes;
	bool fillPhantoms;
	bool showSpriteSize;
	const vec4 collisionShapeOutlineColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	const vec4 spriteOutlineColor = vec4(0.5f, 1.0f, 1.0f, 1.0f);
	const vec4 phantomFillColor = vec4(0.0f, 0.0f, 1.0f, 0.7f);
	const vec4 simulationSpaceOutlineColor = vec4(0.0f, 0.65f, 1.0f, 1.0f);
	const vec4 loadSpaceOutlineColor = vec4(0.57f, 0.0f, 1.0f, 1.0f);
	const vec4 collisionWorldChunkOutlineColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	GameObject* heroGO;  // NOTE: Only so we can quickly grab the hero/ camera for test purposes
	GameObject* cameraGO;

	// Rendering
	const vec4 clearColor = vec4(0.2235f, 0.2235f, 0.2235f, 1.0f);
	Camera* renderCamera;

	GameState();
	void Initialize();
	void Destroy();
	GameMap* GetCurrentGameMap();
	GameMap* GetGameMap(const char * mapName);
	void SetCurrentGameMap(GameMap* map);
	void SetCurrentGameMap(const char * mapName);
	GameMap* AddGameMap(const char* name, vec2 size, vec2 numberOfScreens, vec2 originOffset, vec2 screenCountOffset);
	void SetSimulationSpaceTarget(GameObject* target);
	GameObject* GetSimulationSpaceTarget();
	bool AreAllAnimationsPaused();
	void PauseAllAnimations();
	void UnpauseAllAnimations();
	bool GameFrozen();
	void StartFreeze();
	bool WantsToBeFrozen();
	void EndFreeze();
	void StepGameFreeze(F32 dt);
	void FreezeGame(U32 milliseconds); // Game freezes take effect starting the NEXT FRAME
	void SendEvent(GameObject* gameObject, Event* e);
	void QueueEvent(GameObject* gameObject, Event* e, U32 numberOfFramesToWait);
	void AddThisFramesQueuedEventsToProcessingQueue();
	void SendQueuedEvents();
	void SetGameCamera(Camera* camera);
	Camera* GetGameCamera();
};