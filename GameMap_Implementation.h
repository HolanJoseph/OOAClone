#pragma once

#include "GameMap.h"

#include "Prefabs.h"



void GameMap::Initialize(const char* name, vec2 size, vec2 numberOfScreens, vec2 originOffset, vec2 screenCountOffset)
{
	this->name = Copy(name);
	this->dimensions = size;
	this->numberOfScreens = numberOfScreens;
	this->originOffset = originOffset;
	this->screenCountOffset = screenCountOffset;

	this->collisionWorld = CollisionWorld_2D();
	this->collisionWorld.Initialize(size, numberOfScreens, originOffset);

	this->screenDimensions = VVD(size, numberOfScreens);

	// Generate Screens
	for (size_t x = (size_t)screenCountOffset.x; x < screenCountOffset.x + numberOfScreens.x; ++x)
	{
		for (size_t y = (size_t)screenCountOffset.y; y < screenCountOffset.y + numberOfScreens.y; ++y)
		{
			vec2 backgroundPosition = vec2((screenDimensions.x * (x - screenCountOffset.x)) + screenDimensions.x / 2.0f, (screenDimensions.y * (y - screenCountOffset.y)) + screenDimensions.y / 2.0f) + originOffset;
			char* assetName = Concat(Concat(Concat(Concat(Concat("background_", name), "_"), ToString((U32)x)), "-"), ToString((U32)y));
			GameObject* bgGO = CreateBackground(assetName, backgroundPosition, screenDimensions);
			this->AddGameObject(bgGO);

			GameObject* leftBarGO = CreateVerticalTransitionBar(backgroundPosition - vec2(screenDimensions.x / 2.0f, 0.0f), screenDimensions.y);
			this->AddGameObject(leftBarGO);

			GameObject* bottomBarGO = CreateHorizontalTransitionBar(backgroundPosition - vec2(0.0f, screenDimensions.y / 2.0f), screenDimensions.x);
			this->AddGameObject(bottomBarGO);

			if (x == (screenCountOffset.x + numberOfScreens.x) - 1)
			{
				GameObject* rightBarGO = CreateVerticalTransitionBar(backgroundPosition + vec2(screenDimensions.x / 2.0f, 0.0f), screenDimensions.y);
				this->AddGameObject(rightBarGO);
			}

			if (y == (screenCountOffset.y + numberOfScreens.y) - 1)
			{
				GameObject* topBarGO = CreateHorizontalTransitionBar(backgroundPosition + vec2(0.0f, screenDimensions.y / 2.0f), screenDimensions.x);
				this->AddGameObject(topBarGO);
			}
		}
	}



}


void GameMap::AddGameObject(GameObject* gameObject)
{
	this->gameObjects.push_back(gameObject);
	this->collisionWorld.Add(gameObject);
}

// NOTE: Queues GameObject for destruction at the end of frame
void GameMap::RemoveGameObject(GameObject* gameObject)
{
	this->gameObjectDestructionQueue.push_back(gameObject);
}

void GameMap::DestroyQueuedGameObject(GameObject* gameObject)
{
	this->collisionWorld.Remove(gameObject);

	// NOTE: This should be the GameObjects responsibility
	if (gameObject->sprite != NULL)
	{
		gameObject->RemoveSprite();
	}
	if (gameObject->animator != NULL)
	{
		gameObject->RemoveAnimator();
	}
	if (gameObject->rigidbody != NULL)
	{
		gameObject->RemoveRigidbody();
	}
	if (gameObject->collisionShape != NULL)
	{
		gameObject->RemoveCollisionShape();
	}
	if (gameObject->camera != NULL)
	{
		gameObject->RemoveCamera();
	}

	for (size_t i = 0; i < this->gameObjects.size(); ++i)
	{
		GameObject* go = this->gameObjects[i];
		if (gameObject == go)
		{
			this->gameObjects.erase(this->gameObjects.begin() + i);
			break;
		}
	}

	gameObject->SetType(GameObjectType::Null);
	gameObject->ClearAllTags();

	delete gameObject;
}

void GameMap::ClearDestructionQueue()
{
	for (size_t i = 0; i < this->gameObjectDestructionQueue.size(); ++i)
	{
		GameObject* go = this->gameObjectDestructionQueue[i];
		DestroyQueuedGameObject(go);
	}

	this->gameObjectDestructionQueue.clear();
}


vector<GameObject*> GameMap::FindGameObjectByType(GameObjectType type)
{
	vector<GameObject*> result;

	for (size_t i = 0; i < this->gameObjects.size(); ++i)
	{
		GameObject* go = this->gameObjects[i];
		GameObjectType goType = go->GetType();
		if (goType == type)
		{
			result.push_back(go);
		}
	}

	return result;
}

vector<GameObject*> GameMap::FindGameObjectByTag(GameObjectTags tag)
{
	vector<GameObject*> result;

	for (size_t i = 0; i < this->gameObjects.size(); ++i)
	{
		GameObject* go = this->gameObjects[i];
		bool hasTag = go->HasTag(tag);
		if (hasTag)
		{
			result.push_back(go);
		}
	}

	return result;
}