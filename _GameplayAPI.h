#pragma once

#include "Types.h"
#include "Math.h"

#include "String.h"
#include "Transform.h"

struct Camera;

enum GameObjectType;
enum GameObjectTags;
struct GameObject;
struct Event;
struct CollisionWorld_2D;

#include <vector>
using std::vector;



/*
 * GameObjects
 */
GameObject* CreateGameObject(GameObjectType type);
void		DestroyGameObject(GameObject* gameObject);

vector<GameObject*> FindGameObjectByType(GameObjectType type);
vector<GameObject*> FindGameObjectByTag(GameObjectTags tag);



/*
 * World Manipulation
 */
void CreateMap(const char* name, vec2 size, vec2 numberOfScreens, vec2 originOffset, vec2 screenCountOffset);



/*
 * Events
 */
void SendEvent(GameObject* gameObject, Event* e);
void QueueEvent(GameObject* gameObject, Event* e, U32 numberOfFramesToWait);



/*
 * Collision
 */
CollisionWorld_2D*  GetCollisionWorld();
GameObject*			RaycastFirst(vec2 position, vec2 direction, F32 distance);
vector<GameObject*> RaycastAll(vec2 position, vec2 direction, F32 distance);
vector<GameObject*> RaycastAll(GameObjectType type, vec2 position, vec2 direction, F32 distance);
vector<GameObject*> Shapecast_Rectangle(vec2 position, vec2 halfDim, F32 rotationAngle);
vector<GameObject*> Shapecast_Rectangle(GameObjectType type, vec2 position, vec2 halfDim, F32 rotationAngle);



/*
 * Control
 */
bool AreAllAnimationsPaused();
void PauseAllAnimations();
void UnpauseAllAnimations();

bool GameFrozen();
void FreezeGame(U32 milliseconds);

vec2 GetSimulationSpacePosition();
vec2 GetSimulationSpaceDimensions();
vec2 GetSimulationSpaceHalfDimensions();
void SetSimulationSpace(vec2 center, vec2 dimensions);
void SetSimulationSpace(GameObject* target);

vec2 GetLoadSpacePosition();
vec2 GetLoadSpaceDimensions();
vec2 GetLoadSpaceHalfDimensions();



/*
 * Rendering
 */
void SetRenderCamera(Camera* camera);
Camera* GetRenderCamera();



/*
 * Debug
 */
void DebugDrawPoint(vec2 p, F32 pointSize, vec4 color);
void DebugDrawLine(vec2 a, vec2 b, vec4 color);
void DebugDrawRectangleOutline(vec2 upperLeft, vec2 dimensions, vec4 color, F32 offset = 0.0f);
void DebugDrawRectangleSolidColor(vec2 halfDim, Transform transform, vec4 color);

struct Timer
{
	const char* name;
	SystemTime startTime;
	SystemTime endTime;

	Timer(const char* name)
	{
		this->name = Copy(name);
	}

	~Timer()
	{
		free((char*)this->name);
	}

	void Start()
	{
		this->startTime = GetTimeSinceStartup();
	}

	void End()
	{
		this->endTime = GetTimeSinceStartup();
	}

	SystemTime Get()
	{
		SystemTime result;

		result = this->endTime - this->startTime;

		return result;
	}

	void Report()
	{
		DebugPrintf(1024, "Timer %s: %ums\n", this->name, this->Get().milliseconds);
	}
};