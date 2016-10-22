#pragma once

#include "Types.h"
#include "Math.h"

#include "_SystemAPI.h"
#include "_GameplayAPI.h"

#include "Transform.h"
#include "CollisionDetection2DObj.h"
#include "GameObject.h"

#include <vector>
using std::vector;

struct PenetrationInfo_2D
{
	GameObject* go1;
	GameObject* go2;
	vec2 normal;
	F32 distance;

	PenetrationInfo_2D(GameObject* go1, GameObject* go2, vec2 normal, F32 distance)
	{
		this->go1 = go1;
		this->go2 = go2;
		this->normal = normal;
		this->distance = distance;
	}
};

struct CollisionChunk_2D
{
	Rectangle_2D dimensions;
	Transform transform;

	vector<GameObject*> phantoms;
	vector<GameObject*> activeColliders;
	vector<GameObject*> staticColliders;

	CollisionChunk_2D(Rectangle_2D dimensions, Transform transform)
	{
		this->dimensions = dimensions;
		this->transform = transform;
	}

	void Add(vector<GameObject*>& bin, GameObject* go)
	{
		bin.push_back(go);
	}

	void AddPhantom(GameObject* go)
	{
		this->Add(this->phantoms, go);
	}

	void AddActive(GameObject* go)
	{
		this->Add(this->activeColliders, go);
	}

	void AddStatic(GameObject* go)
	{
		this->Add(this->staticColliders, go);
	}



	void Remove(vector<GameObject*>& bin, GameObject* go)
	{
		for (size_t i = 0; i < bin.size(); ++i)
		{
			GameObject* curr = bin[i];
			if (go == curr)
			{
				bin.erase(bin.begin() + i);
				break;
			}
		}
	}

	void RemovePhantom(GameObject* go)
	{
		this->Remove(this->phantoms, go);
	}

	void RemoveActive(GameObject* go)
	{
		this->Remove(this->activeColliders, go);
	}

	void RemoveStatic(GameObject* go)
	{
		this->Remove(this->staticColliders, go);
	}
};

struct CollisionChunkRectangle
{
	Rectangle_2D dimensions;
	Transform transform;

	CollisionChunkRectangle(Rectangle_2D dimensions, Transform transform)
	{
		this->dimensions = dimensions;
		this->transform = transform;
	}
};

struct Range_size_t
{
	size_t min;
	size_t max;

	Range_size_t()
	{
		this->min = 0;
		this->max = 0;
	}

	Range_size_t(size_t min, size_t max)
	{
		this->min = min;
		this->max = max;
	}
};

struct Range_2D_size_t
{
	Range_size_t x;
	Range_size_t y;
};

struct ActiveColliderInfo
{
	GameObject* go;
	Transform prevTransform;
	AxisAlignedBoundingBox prevAABB;
	Range_2D_size_t prevRange;

	ActiveColliderInfo(GameObject* go, Transform prevTransform, AxisAlignedBoundingBox prevAABB, Range_2D_size_t prevRange)
	{
		this->go = go;
		this->prevTransform = prevTransform;
		this->prevAABB = prevAABB;
		this->prevRange = prevRange;
	}
};

struct Displacements
{
	vec2 displacement1;
	vec2 displacement2;

	Displacements()
	{
		this->displacement1 = vec2(0.0f, 0.0f);
		this->displacement2 = vec2(0.0f, 0.0f);
	}
};

struct CollisionWorld_2D
{
	vec2 size;
	vec2 numberOfScreens;
	vec2 originOffset;
	vector<CollisionChunk_2D> chunks;

	vector<ActiveColliderInfo> actives; // Necessary because the same object can be in multiple chunks, which would cause multiple updating. 

	void Initialize(vec2 size, vec2 numberOfScreens, vec2 originOffset = vec2(0.0f, 0.0f));
	void Destroy();

	vector<CollisionChunkRectangle> Add(GameObject* go);
	void Remove(GameObject* go);

	void FixupActives();
	vector<PenetrationInfo_2D> ResolveInterpenetrations(vec2 position, vec2 halfDimensions);
	vector<PenetrationInfo_2D> ResolveInterpenetrations(size_t chunkIndex);
	vector<PenetrationInfo_2D> GenerateCollisions(vector<GameObject*>& collidables);

	GameObject* RaycastFirst(vec2 position, vec2 direction);
	GameObject* RaycastFirst(vec2 position, vec2 direction, GameObjectType type);
	GameObject* RaycastFirst(vec2 position, vec2 direction, GameObjectTags tag);
	GameObject* RaycastFirst(vec2 position, vec2 direction, GameObjectType type, GameObjectTags tag);

	vector<GameObject*> RaycastAll(vec2 position, vec2 direction);
	vector<GameObject*> RaycastAll(vec2 position, vec2 direction, GameObjectType type);
	vector<GameObject*> RaycastAll(vec2 position, vec2 direction, GameObjectTags tag);
	vector<GameObject*> RaycastAll(vec2 position, vec2 direction, GameObjectType type, GameObjectTags tag);

	vector<GameObject*> Shapecast_Rectangle(vec2 position, vec2 halfdim, F32 rotationAngle);
	vector<GameObject*> Shapecast_Rectangle(vec2 position, vec2 halfdim, F32 rotationAngle, GameObjectType type);
	vector<GameObject*> Shapecast_Rectangle(vec2 position, vec2 halfdim, F32 rotationAngle, GameObjectTags tag);
	vector<GameObject*> Shapecast_Rectangle(vec2 position, vec2 halfdim, F32 rotationAngle, GameObjectType type, GameObjectTags tag);

	vector<CollisionChunkRectangle> Debug_GetChunkOutlines();

	// NOTE: Internal usage
	size_t GetIndex(vec2 xy);
	size_t GetIndex(size_t x, size_t y);
	vec2 GetChunkContainingPosition(vec2 position);
	Range_2D_size_t GetRange(vec2 chunkNumber, AxisAlignedBoundingBox goAABB);
	vector<size_t> GetIndicesFromRange_2D(Range_2D_size_t range);
};