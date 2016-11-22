// NOTE: Do these functions need to be inlined?
#pragma once

#include "GameObject.h"
#include "CollisionWorld2D.h"
#include "GameState.h"

extern GameState global;





/*
 * Sprite Implementation
 */
inline Sprite::Sprite()
{
	this->texture = TextureHandle();
	this->offset = vec2();
}

inline void Sprite::Initialize(const char* assetName, vec2 offset = vec2(0.0f, 0.0f))
{
	const char* filePath = global.spriteAssetFilepathTable.GetValue(assetName);
	if (filePath == NULL)
	{
		filePath = global.spriteAssetFilepathTable.GetValue("missing_Texture"); //AssetNameToFilepath("missing_Texture");
	}
	this->texture = AddToTexturePool(filePath);
	this->offset = offset;
}

inline void Sprite::Load()
{
	bool isLoaded = IsTextureLoaded(this->texture);
	if (!isLoaded)
	{
		LoadTexture(this->texture);
	}
}

inline void Sprite::Unload()
{
	bool isLoaded = IsTextureLoaded(this->texture);
	if (isLoaded)
	{
		UnloadTexture(this->texture);
	}
}




/*
 * Animation Implementation
 */
void Animation::Initialize(const char* animationFolderPath, U32 numberOfFrames, F32 animationTime)
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

void Animation::Destroy()
{
	for (size_t i = 0; i < this->numberOfFrames; ++i)
	{
		//RemoveFromTexturePool(animation->frames[i]);
	}
	free(this->frames);

	this->numberOfFrames = 0;
	this->animationTime = 0;
}

void Animation::Load()
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

void Animation::Unload()
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





/*
 * AnimationHash Implementation
 */
void AnimationHash::Initialize(U32 numberOfIndices)
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


AnimationHash::GetKeyIndexResult AnimationHash::GetKeyIndex(const char* key)
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

const char* AnimationHash::GetKey(size_t i, size_t j)
{
	const char* result = NULL;

	if (i < this->numberOfIndices && j < this->subListLength[i])
	{
		result = table[i][j].k;
	}

	return result;
}

Animation* AnimationHash::GetValue(const char* key)
{
	Animation* result = NULL;

	GetKeyIndexResult keyIndex = GetKeyIndex(key);
	if (keyIndex.present)
	{
		result = this->table[keyIndex.i][keyIndex.j].v;
	}

	return result;
}

void AnimationHash::AddKVPair(const char* key, Animation* val)
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

void AnimationHash::RemoveKVPair(const char* key)
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

size_t AnimationHash::Length()
{
	size_t result;

	result = this->length;

	return result;
}

void AnimationHash::Destroy()
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

void AnimationHash::Load()
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

void AnimationHash::Unload()
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





/*
 * AnimationController Implementation
 */
AnimationController::AnimationController()
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

void AnimationController::Initialize()
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
const char * AnimationController::ActiveAnimation()
{
	const char * result = this->animations.GetKey(this->activeAnimationKI_i, this->activeAnimationKI_j);
	return result;
}

void AnimationController::AddAnimation(const char* referenceName, const char* assetName, U32 numberOfFrames, F32 animationTime)
{
	const char* filePath = global.spriteAssetFilepathTable.GetValue(assetName);
	Animation* animation = new Animation();
	animation->Initialize(filePath, numberOfFrames, animationTime);
	this->animations.AddKVPair(referenceName, animation);
	this->isAnimated = true;
}

void AnimationController::RemoveAnimation(const char* referenceName)
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

void AnimationController::StartAnimation()
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

bool AnimationController::StartAnimation(const char* referenceName, bool restartAnimation, bool loopAnimation, bool playInReverse, F32 startTime)
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

bool AnimationController::IsPaused()
{
	bool result = !this->isAnimationPlaying;
	return result;
}

void AnimationController::PauseAnimation()
{
	this->isAnimationPlaying = false;
}

void AnimationController::StopAnimation()
{
	this->isAnimationPlaying = false;
	// NOTE: Maybe leave these in? Only time will tell.
	//entity->isAnimationLooped = false;
	//entity->isAnimationReversed = false;
	this->elapsedTime = 0.0f;
}

// ?? Useful?
void AnimationController::ReverseAnimation(bool reverse)
{
	this->isAnimationReversed = reverse;
}

void AnimationController::StepElapsedAnimationTime(F32 time)
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

void AnimationController::SetElapsedAnimationTime(F32 time)
{
	F32 animTime = this->animations.table[this->activeAnimationKI_i][this->activeAnimationKI_j].v->animationTime;
	F32 newTime = time - ((I32)(time / animTime) * animTime);
	this->elapsedTime = newTime;
}

// 0 == 0%, 100 == 100%
void AnimationController::SetElapsedAnimationTimeAsPercent(F32 percent)
{
	F32 multiplier = (percent / 100.0f) - (I32)(percent / 100.0f);
	F32 animationLength = this->animations.table[this->activeAnimationKI_i][this->activeAnimationKI_j].v->animationTime;
	F32 newTime = multiplier * animationLength;
	this->elapsedTime = newTime;
}

size_t AnimationController::NumberOfAnimations()
{
	size_t result;
	result = this->animations.Length();
	return result;
}

TextureHandle AnimationController::GetCurrentAnimationFrame()
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

void AnimationController::SetSpriteOffset(vec2 spriteOffset)
{
	this->spriteOffset = spriteOffset;
}

vec2 AnimationController::GetSpriteOffset()
{
	vec2 result;
	result = this->spriteOffset;
	return result;
}

void AnimationController::Load()
{
	this->animations.Load();
}

void AnimationController::Unload()
{
	this->animations.Unload();
}






/*
 * RigidBody Implementation
 */
RigidBody::RigidBody()
{
	this->position = vec2(0.0f, 0.0f);
	this->velocity = vec2(0.0f, 0.0f);
	this->forceAccumulator = vec2(0.0f, 0.0f);
	this->inverseMass = 0.0f;
	this->dampingFactor = DEFAULT_DAMPING_FACTOR;

	this->frameVelocity = vec2(0.0f, 0.0f);
}

void RigidBody::Initialize(
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

void RigidBody::Destroy()
{
	this->position = vec2(0.0f, 0.0f);
	this->velocity = vec2(0.0f, 0.0f);
	this->forceAccumulator = vec2(0.0f, 0.0f);

	this->inverseMass = 0.0f;
	this->dampingFactor = DEFAULT_DAMPING_FACTOR;

	this->frameVelocity = vec2(0.0f, 0.0f);
};

F32 RigidBody::GetMass()
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

vec2 RigidBody::GetVelocityForFrame()
{
	vec2 result;
	result = this->frameVelocity;
	return result;
}

vec2 RigidBody::GetAcceleration()
{
	vec2 result = this->inverseMass * this->forceAccumulator;
	return result;
}

void RigidBody::SetPosition(vec2 position)
{
	this->position = position;
}

void RigidBody::Integrate(F32 deltaTime)
{
	vec2 acceleration = this->GetAcceleration();
	this->frameVelocity = this->velocity * deltaTime; // NOTE: Honestly not sure if this is accurate. Though it should be because of the next line.
	this->position = this->position + this->frameVelocity; /*(this->velocity * deltaTime);*/
	this->velocity = (this->velocity * pow(this->dampingFactor, deltaTime)) + (acceleration * deltaTime);
	this->forceAccumulator = vec2(0.0f, 0.0f);
}

void RigidBody::ApplyForce(vec2 direction, F32 power)
{
	vec2 scaledForce = direction * power;
	this->forceAccumulator += scaledForce;
}

// directions should be normalized
void RigidBody::ApplyImpulse(vec2 direction, F32 power)
{
	vec2 scaledVelocity = direction * power;
	this->velocity += scaledVelocity;
}





/*
 * GameObject Implementation
 */
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

	//CollisionWorld_2D* collisionWorld = &(global.GetCurrentGameMap()->collisionWorld);
	//collisionWorld->Add(this);
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
	//CollisionWorld_2D* collisionWorld = &(global.GetCurrentGameMap()->collisionWorld);
	//collisionWorld->Remove(this);

	delete this->collisionShape;
	this->collisionShape = NULL;
}

void GameObject::RemoveCamera()
{
	delete this->camera;
	this->camera = NULL;
}



void GameObject::IntegratePhysicsObjects(F32 dt)
{
	for (size_t i = 0; i < physicsGameObjects.size(); ++i)
	{
		GameObject* go = physicsGameObjects[i];
		go->rigidbody->SetPosition(go->transform.position);
		go->rigidbody->Integrate(dt);
		go->transform.position = go->rigidbody->position;
	}
}