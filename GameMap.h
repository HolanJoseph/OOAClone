#pragma once

#include "Types.h"
#include "Math.h"

#include "GameObject.h"
#include "CollisionWorld2D.h"

#include <vector>
using std::vector;


struct GameMap
{
	const char * name;

	vec2 dimensions;
	vec2 screenDimensions;
	vec2 numberOfScreens;
	vec2 originOffset;
	vec2 screenCountOffset;

	vector<GameObject*> gameObjects;
	vector<GameObject*> gameObjectDestructionQueue;

	CollisionWorld_2D collisionWorld;

	void Initialize(const char* name, vec2 size, vec2 numberOfScreens, vec2 originOffset = vec2(0.0f, 0.0f), vec2 screenCountOffset = vec2(0.0f, 0.0f));


	void AddGameObject(GameObject* gameObject);

	// NOTE: Queues GameObject for destruction at the end of frame
	void RemoveGameObject(GameObject* gameObject);

	void DestroyQueuedGameObject(GameObject* gameObject);

	void ClearDestructionQueue();


	vector<GameObject*> FindGameObjectByType(GameObjectType type);

	vector<GameObject*> FindGameObjectByTag(GameObjectTags tag);

};