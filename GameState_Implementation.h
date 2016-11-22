#pragma once

#include "GameState.h"

GameState::GameState()
{
	// Maps
	this->currentMap = NULL;

	// Simulation
	this->simulationSpaceTarget = NULL;

	// Loading

	// Assets

	// Interpenetration

	// Update Loop Control
	this->pauseAnimations = false;

	this->freezeNextFrame = false;
	this->gameFrozen = false;
	this->freezeTimer = 0.0f;
	this->freezeLength = 0;

	// Debug
	this->showCollisionVolumes = false;
	this->fillPhantoms = false;
	this->showSpriteSize = false;
	this->heroGO = NULL;
	this->cameraGO = NULL;

	// Rendering
	renderCamera = NULL;
}

GameMap* GameState::GetCurrentGameMap()
{
	GameMap* result;

	result = this->currentMap;

	return result;
}

GameMap* GameState::GetGameMap(const char * mapName)
{
	GameMap* result = NULL;

	for (size_t i = 0; i < this->maps.size(); ++i)
	{
		const char * testMapName = this->maps[i].name;
		bool found = Compare(mapName, testMapName);
		if (found)
		{
			result = &(this->maps[i]);
			break;
		}
	}

	return result;
}

void GameState::SetCurrentGameMap(GameMap* map)
{
	this->currentMap = map;
}

void GameState::SetCurrentGameMap(const char * mapName)
{
	GameMap* map = this->GetGameMap(mapName);
	if (map != NULL)
	{
		this->SetCurrentGameMap(map);
	}
}

GameMap* GameState::AddGameMap(const char* name, vec2 size, vec2 numberOfScreens, vec2 originOffset = vec2(0.0f, 0.0f), vec2 screenCountOffset = vec2(0.0f, 0.0f))
{
	this->maps.push_back(GameMap());
	size_t index = this->maps.size() - 1;
	GameMap* map = &(this->maps[index]);
	map->Initialize(name, size, numberOfScreens, originOffset, screenCountOffset);

	return map;
}

void GameState::SetSimulationSpaceTarget(GameObject* target)
{
	this->simulationSpaceTarget = target;
}

GameObject* GameState::GetSimulationSpaceTarget()
{
	GameObject* result;

	result = this->simulationSpaceTarget;

	return result;
}

bool GameState::AreAllAnimationsPaused()
{
	bool result;

	result = this->pauseAnimations;

	return result;
}

void GameState::PauseAllAnimations()
{
	this->pauseAnimations = true;
}

void GameState::UnpauseAllAnimations()
{
	this->pauseAnimations = false;
}

bool GameState::GameFrozen()
{
	bool result;

	result = this->gameFrozen;

	return result;
}

void GameState::StartFreeze()
{
	this->freezeNextFrame = false;
	this->gameFrozen = true;
	this->freezeTimer = 0.0f;
}

bool GameState::WantsToBeFrozen()
{
	bool result;
	result = this->freezeNextFrame;
	return result;
}

void GameState::EndFreeze()
{
	this->freezeNextFrame = false;
	this->gameFrozen = false;
	this->freezeTimer = 0.0f;
	this->freezeLength = 0.0f;
}

void GameState::StepGameFreeze(F32 dt)
{
	if (this->GameFrozen())
	{
		this->freezeTimer += dt;
		if (this->freezeTimer >= this->freezeLength)
		{
			this->EndFreeze();
		}
	}
}

void GameState::FreezeGame(U32 milliseconds)
{
	if (!(this->GameFrozen() || this->WantsToBeFrozen()))
	{
		this->freezeNextFrame = true;
		this->freezeLength = (F32)((F64)milliseconds / (F64)1000);
	}
}

void GameState::SendEvent(GameObject* gameObject, Event* e)
{
	if (gameObject->DoEvent != NULL)
	{
		gameObject->DoEvent(gameObject, e);
	}
}

void GameState::QueueEvent(GameObject* gameObject, Event* e, U32 numberOfFramesToWait)
{
	this->queuedEventsFromThisFrame.push_back(QueuedEvent(gameObject, e, numberOfFramesToWait));
}

void GameState::AddThisFramesQueuedEventsToProcessingQueue()
{
	for (size_t i = 0; i < this->queuedEventsFromThisFrame.size(); ++i)
	{
		this->queuedEventsToProcess.push_back(this->queuedEventsFromThisFrame[i]);
	}
	this->queuedEventsFromThisFrame.clear();
}

void GameState::SendQueuedEvents()
{
	for (size_t i = 0; i < this->queuedEventsToProcess.size(); ++i)
	{
		QueuedEvent* qe = &(this->queuedEventsToProcess[i]);
		--qe->numberOfFramesToWait;
		if (qe->numberOfFramesToWait <= 0)
		{
			this->SendEvent(qe->recipient, qe->e);
		}
		size_t qs = this->queuedEventsToProcess.size();
		size_t tlsz = 1;
	}

	for (I32 i = this->queuedEventsToProcess.size() - 1; i >= 0; --i)
	{
		QueuedEvent* qe = &(this->queuedEventsToProcess[i]);
		if (qe->numberOfFramesToWait <= 0)
		{
			this->queuedEventsToProcess.erase(this->queuedEventsToProcess.begin() + i);
		}
	}
}

void GameState::SetGameCamera(Camera* camera)
{
	this->renderCamera = camera;
}

Camera* GameState::GetGameCamera()
{
	Camera* result;
	result = this->renderCamera;
	return result;
}

