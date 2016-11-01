// NOTE: Do these functions need to be inlined?
#pragma once

#include "GameObject.h"
#include "CollisionWorld2D.h"



void GameObject::ForceUpdates(bool b)
{
	this->forceUpdates = b;
}

bool GameObject::ForcesUpdates()
{
	bool result;

	result = this->forceUpdates;

	return result;
}



void GameObject::SetType(GameObjectType type)
{
	this->type = type;
}

GameObjectType GameObject::GetType()
{
	GameObjectType result;

	result = this->type;

	return result;
}



void GameObject::AddTag(GameObjectTags tag)
{
	SetBit(&this->tags, (U8)tag, 1);
}

void GameObject::RemoveTag(GameObjectTags tag)
{
	SetBit(&tags, (U8)tag, 0);
}

void GameObject::ClearAllTags()
{
	this->tags = 0;
}

bool GameObject::HasTag(GameObjectTags tag)
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

	CollisionWorld_2D* collisionWorld = GetCollisionWorld();
	collisionWorld->Add(this);
}

void GameObject::AddCamera(vec2 halfDim)
{
	this->camera = new Camera();
	this->camera->position = this->transform.position;
	this->camera->scale = 1.0f;
	this->camera->rotationAngle = this->transform.rotationAngle;
	this->camera->ResizeViewArea(halfDim);
	SetRenderCamera(this->camera);
}



void GameObject::RemoveSprite()
{
	delete this->sprite;
	this->sprite = NULL;
}

void GameObject::RemoveAnimator()
{
	delete this->animator;
	this->animator = NULL;
}

void GameObject::RemoveRigidbody()
{
	for (size_t i = 0; i < physicsGameObjects.size(); ++i)
	{
		GameObject* go = physicsGameObjects[i];
		if (this == go)
		{
			physicsGameObjects.erase(physicsGameObjects.begin() + i);
			break;
		}
	}
	delete this->rigidbody;
	this->rigidbody = NULL;
}

void GameObject::RemoveCollisionShape()
{
	CollisionWorld_2D* collisionWorld = GetCollisionWorld();
	collisionWorld->Remove(this);

	delete this->collisionShape;
	this->collisionShape = NULL;
}

void GameObject::RemoveCamera()
{
	delete this->camera;
	this->camera = NULL;
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