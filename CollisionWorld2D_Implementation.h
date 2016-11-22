#pragma once

#include "CollisionWorld2D.h"



void CollisionWorld_2D::Initialize(vec2 size, vec2 numberOfScreens, vec2 originOffset)
{
	this->size = size;
	this->numberOfScreens = numberOfScreens;
	this->originOffset = originOffset;

	vec2 dimensions = VVD(size, numberOfScreens);
	vec2 halfDimensions = dimensions / 2.0f;
	size_t totalNumberOfScreens = (size_t)numberOfScreens.x * (size_t)numberOfScreens.y;

	this->chunks.reserve(totalNumberOfScreens);
	for (size_t y = 0; y < numberOfScreens.y; ++y)
	{
		for (size_t x = 0; x < numberOfScreens.x; ++x)
		{
			CollisionChunk_2D chunk = CollisionChunk_2D(halfDimensions, Transform());
			chunk.transform.position = VVM(vec2(x, y), dimensions) + halfDimensions + originOffset;
			this->chunks.push_back(chunk);
		}
	}
}

void CollisionWorld_2D::Destroy()
{
	this->size = vec2(0.0f, 0.0f);
	this->numberOfScreens = vec2(0.0f, 0.0f);
	this->chunks.clear();
}



bool IsActive(GameObject* go)
{
	bool result = false;

	if (go->rigidbody != NULL)
	{
		result = true;
	}

	return result;
}

// NOTE: Return value only for debug visualization purposes.
vector<CollisionChunkRectangle> CollisionWorld_2D::Add(GameObject* go)
{
	vector<CollisionChunkRectangle> result;

	// If the GameObject has none of the necessary components exit.
	if (go->collisionShape == NULL && go->rigidbody == NULL)
	{
		return result;
	}

	vec2 prospectiveChunk = this->GetChunkContainingPosition(go->transform.position);
	AxisAlignedBoundingBox goAABB = go->collisionShape->GetAxisAlignedBoundingBox(go->transform);

	Range_2D_size_t range = this->GetRange(prospectiveChunk, goAABB);
	for (size_t x = range.x.min; x <= range.x.max; ++x)
	{
		for (size_t y = range.y.min; y <= range.y.max; ++y)
		{
			size_t i = this->GetIndex(x, y);
			bool isPhantom = go->collisionShape->IsPhantom();
			bool isActive = IsActive(go);

			if (isPhantom)
			{
				this->chunks[i].AddPhantom(go);
			}
			else if (isActive)
			{
				this->chunks[i].AddActive(go);
			}
			else if (!isActive)
			{
				this->chunks[i].AddStatic(go);
			}

			result.push_back(CollisionChunkRectangle(this->chunks[i].dimensions, this->chunks[i].transform));
		}
	}

	// NOTE: Add GameObject to actives list.
	if (IsActive(go))
	{
		this->actives.push_back(ActiveColliderInfo(go, go->transform, goAABB, range));
	}

	return result;
}

void CollisionWorld_2D::Remove(GameObject* go)
{
	for (size_t i = 0; i < this->actives.size(); ++i)
	{
		GameObject* active = this->actives[i].go;
		if (go == active)
		{
			this->actives.erase(this->actives.begin() + i);
			break;
		}
	}

	vec2 prospectiveChunk = this->GetChunkContainingPosition(go->transform.position);
	size_t chunkIndex = this->GetIndex(prospectiveChunk);

	AxisAlignedBoundingBox goAABB = go->collisionShape->GetAxisAlignedBoundingBox(go->transform);

	Range_2D_size_t range = this->GetRange(prospectiveChunk, goAABB);

	bool isPhantom = go->collisionShape->IsPhantom();
	bool isActive = (go->rigidbody != NULL ? true : false);

	for (size_t y = range.y.min; y <= range.y.max; ++y)
	{
		for (size_t x = range.x.min; x <= range.x.max; ++x)
		{
			size_t i = this->GetIndex(x, y);
			CollisionChunk_2D* chunk = &(this->chunks[i]);
			
			if (isPhantom)
			{
				chunk->RemovePhantom(go);
			}
			else if (isActive)
			{
				chunk->RemoveActive(go);
			}
			else if (!isActive)
			{
				chunk->RemoveStatic(go);
			}

		}
	}
}



bool IsIndexPresent(vector<size_t>& indices, size_t index)
{
	bool result = false;

	for (size_t i = 0; i < indices.size(); ++i)
	{
		size_t testIndex = indices[i];
		if (index == testIndex)
		{
			result = true;
			break;
		}
	}

	return result;
}

void CollisionWorld_2D::FixupActives()
{
	for (size_t i = 0; i < this->actives.size(); ++i)
	{
		ActiveColliderInfo* activesInfo = &(this->actives[i]);
		GameObject* go = activesInfo->go;
		Transform prevTransform = activesInfo->prevTransform;
		AxisAlignedBoundingBox prevAABB = activesInfo->prevAABB;
		Range_2D_size_t prevRange = activesInfo->prevRange;

		if (go->transform != prevTransform)
		{
			// NOTE: Get new AABB.
			AxisAlignedBoundingBox newAABB;
			if ((go->transform.rotationAngle == prevTransform.rotationAngle) &&  (go->transform.scale == prevTransform.scale))
			{
				vec2 translation = go->transform.position - prevTransform.position;
				newAABB = prevAABB;
				newAABB.upperLeft += translation;
				newAABB.bottomRight += translation;
			}
			else
			{
				newAABB = go->collisionShape->GetAxisAlignedBoundingBox(go->transform);
			}

			// Compare ranges
			vec2 initialChunk = this->GetChunkContainingPosition(go->transform.position);
			Range_2D_size_t newRange = this->GetRange(initialChunk, newAABB);

			vector<size_t> prevIndices = this->GetIndicesFromRange_2D(prevRange);
			vector<size_t> newIndices = this->GetIndicesFromRange_2D(newRange);

			vector<size_t> addIndices;
			vector<size_t> removeIndices;

			for (size_t i = 0; i < newIndices.size(); ++i)
			{
				size_t index = newIndices[i];
				bool previouslyPresent = IsIndexPresent(prevIndices, index);
				if (!previouslyPresent)
				{
					addIndices.push_back(index);
				}
			}

			for (size_t i = 0; i < prevIndices.size(); ++i)
			{
				size_t index = prevIndices[i];
				bool stillPresent = IsIndexPresent(newIndices, index);
				if (!stillPresent)
				{
					removeIndices.push_back(index);
				}
			}

			size_t ais = addIndices.size();
			for (size_t i = 0; i < addIndices.size(); ++i)
			{
				// Add the go to the chunk.
				size_t index = addIndices[i];
				CollisionChunk_2D* chunk = &(this->chunks[index]);
				chunk->AddActive(go);
			}

			size_t ris = removeIndices.size();
			for (size_t i = 0; i < removeIndices.size(); ++i)
			{
				// Remove the go from the chunk.
				size_t index = removeIndices[i];
				CollisionChunk_2D* chunk = &(this->chunks[index]);
				chunk->RemoveActive(go);
			}

			activesInfo->prevTransform = go->transform;
			activesInfo->prevRange = newRange;
			activesInfo->prevAABB = newAABB;
		}
	}
}

AxisAlignedBoundingBox AxisAlignedBoundingBoxFromCenterAndHalfDim(vec2 center, vec2 halfDim)
{
	AxisAlignedBoundingBox result;

	vec2 upperLeft = center + vec2(-halfDim.x, halfDim.y);
	vec2 bottomRight = center + vec2(halfDim.x, -halfDim.y);
	result = AxisAlignedBoundingBox(upperLeft, bottomRight);

	return result;
}

vector<PenetrationInfo_2D> CollisionWorld_2D::ResolveInterpenetrations(vec2 position, vec2 halfDimensions)
{
	vector<PenetrationInfo_2D> result;

	vec2 centerChunk = this->GetChunkContainingPosition(position);
	AxisAlignedBoundingBox aabb = AxisAlignedBoundingBoxFromCenterAndHalfDim(position, halfDimensions);
	Range_2D_size_t chunksToResolve = this->GetRange(centerChunk, aabb);
	vector<size_t> indices = this->GetIndicesFromRange_2D(chunksToResolve);
	for (size_t i = 0; i < indices.size(); ++i)
	{
		size_t index = indices[i];
		
		vector<PenetrationInfo_2D> resolved = this->ResolveInterpenetrations(index);
		for (size_t j = 0; j < resolved.size(); ++j)
		{
			result.push_back(resolved[j]);
		}
	}

	return result;
}

Displacements GetDisplacements(PenetrationInfo_2D info)
{
	Displacements result;

	// NOTE: go1 is always active due to the way we are generating the interpenetrations.
	bool go2IsActive = IsActive(info.go2);
	vec2 displacement = info.normal * info.distance;
	if (go2IsActive)
	{
		// case: go1 active, go2 active
		F32 go1Mass = 1.0f;
		F32 go2Mass = 1.0f;

		F32 go1ScaleFactor = (go2Mass / (go1Mass + go2Mass));
		F32 go2ScaleFactor = (go1Mass / (go1Mass + go2Mass));

		vec2 pm1Displacement = go1ScaleFactor * displacement;
		vec2 pm2Displacement = go2ScaleFactor * -displacement;

		result.displacement1 = -pm1Displacement;
		result.displacement2 = -pm2Displacement;
	}
	else
	{
		// case: go1 active, go2 static
		result.displacement1 = -displacement;
		result.displacement2 = vec2(0.0f, 0.0f);
	}

	return result;
}

vector<PenetrationInfo_2D> CollisionWorld_2D::ResolveInterpenetrations(size_t chunkIndex)
{
	vector<PenetrationInfo_2D> result;

	// Generate Contacts
	vector<PenetrationInfo_2D> interpenetrations;
	CollisionChunk_2D* chunk = &(this->chunks[chunkIndex]);
	vector<GameObject*>& actives = chunk->activeColliders;
	vector<GameObject*>& statics = chunk->staticColliders;
	for (size_t i = 0; i < actives.size(); ++i)
	{
		GameObject* go1 = actives[i];

		// Active Colliders
		for (size_t j = i + 1; j < actives.size(); ++j)
		{
			GameObject* go2 = actives[j];
			CollisionInfo_2D ci = DetectCollision_2D(go1->collisionShape, go1->transform, go2->collisionShape, go2->transform);
			if (ci.collided)
			{
				interpenetrations.push_back(PenetrationInfo_2D(go1, go2, ci.normal, ci.distance));
			}
		}

		for (size_t j = 0; j < statics.size(); ++j)
		{
			GameObject* go2 = statics[j];
			CollisionInfo_2D ci = DetectCollision_2D(go1->collisionShape, go1->transform, go2->collisionShape, go2->transform);
			if (ci.collided)
			{
				interpenetrations.push_back(PenetrationInfo_2D(go1, go2, ci.normal, ci.distance));
			}
		}
	}


	// Fix contacts
	size_t MAX_NUMBER_OF_ITERATIONS = interpenetrations.size() * 10;
	size_t i = 0;
	while (i < MAX_NUMBER_OF_ITERATIONS)
	{
		I32 index = -1;
		F32 penetrationDepth = -1.0f;
		for (size_t j = 0; j < interpenetrations.size(); ++j)
		{
			if (interpenetrations[j].distance > penetrationDepth)
			{
				index = j;
				penetrationDepth = interpenetrations[j].distance;
			}
		}

		if (index == -1 || penetrationDepth <= 0.0f)
		{
			// We have fixed all interpenetrations.
			break;
		}

		Displacements d = GetDisplacements(interpenetrations[index]);

		interpenetrations[index].go1->transform.position += d.displacement1;
		interpenetrations[index].go2->transform.position += d.displacement2;
		interpenetrations[index].distance = 0.0f;

		for (size_t k = 0; k < interpenetrations.size(); ++k) // Apply displacements
		{
			if (k == index)
			{
				continue;
			}

			GameObject* go1 = interpenetrations[k].go1;
			GameObject* go2 = interpenetrations[k].go2;

			bool t1 = (interpenetrations[index].go1 == go1);
			bool t2 = (interpenetrations[index].go1 == go2);
			if (t1)
			{
				interpenetrations[k].distance += dot(interpenetrations[k].normal, d.displacement1);
			}
			if (t2)
			{
				interpenetrations[k].distance -= dot(interpenetrations[k].normal, d.displacement1);
			}

			bool t3 = (interpenetrations[index].go2 == go1);
			bool t4 = (interpenetrations[index].go2 == go2);
			if (t3)
			{
				interpenetrations[k].distance += dot(interpenetrations[k].normal, d.displacement2);
			}
			if (t4)
			{
				interpenetrations[k].distance -= dot(interpenetrations[k].normal, d.displacement2);
			}
		}

		++i;
	}


	//return result;
	return interpenetrations;
}

vector<PenetrationInfo_2D> CollisionWorld_2D::GetPhantomInterpenetrations(vec2 position, vec2 halfDimensions)
{
	vector<PenetrationInfo_2D> result;

	vec2 centerChunk = this->GetChunkContainingPosition(position);
	AxisAlignedBoundingBox aabb = AxisAlignedBoundingBoxFromCenterAndHalfDim(position, halfDimensions);
	Range_2D_size_t chunksToResolve = this->GetRange(centerChunk, aabb);
	vector<size_t> indices = this->GetIndicesFromRange_2D(chunksToResolve);
	for (size_t i = 0; i < indices.size(); ++i)
	{
		size_t index = indices[i];

		vector<PenetrationInfo_2D> resolved = this->GetPhantomInterpenetrations(index);
		for (size_t j = 0; j < resolved.size(); ++j)
		{
			result.push_back(resolved[j]);
		}
	}

	return result;
}

vector<PenetrationInfo_2D> CollisionWorld_2D::GetPhantomInterpenetrations(size_t chunkIndex)
{
	vector<PenetrationInfo_2D> result;

	// Generate Contacts
	vector<PenetrationInfo_2D> interpenetrations;
	CollisionChunk_2D* chunk = &(this->chunks[chunkIndex]);
	vector<GameObject*>& actives = chunk->activeColliders;
	vector<GameObject*>& phantoms = chunk->phantoms;
	for (size_t i = 0; i < actives.size(); ++i)
	{
		GameObject* go1 = actives[i];

		for (size_t j = 0; j < phantoms.size(); ++j)
		{
			GameObject* go2 = phantoms[j];
			CollisionInfo_2D ci = DetectCollision_2D(go1->collisionShape, go1->transform, go2->collisionShape, go2->transform);
			if (ci.collided)
			{
				result.push_back(PenetrationInfo_2D(go1, go2, ci.normal, ci.distance));
			}
		}
	}

	return result;
}




GameObject* CollisionWorld_2D::RaycastFirst(vec2 position, vec2 direction)
{
	GameObject* result;

	F32 resultDistance = 500.0f;
	vec2 rayPerp = Perpendicular_2D(direction);

	vec2 chunkNumber = this->GetChunkContainingPosition(position);
	size_t chunkIndex = this->GetIndex(chunkNumber);
	CollisionChunk_2D* chunk = &(this->chunks[chunkIndex]);

	// Check each bucket.
	for (size_t i = 0; i < chunk->activeColliders.size(); ++i)
	{
		GameObject* go = chunk->activeColliders[i];
		vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
		vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
		F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
		F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
		F32 goOriginDOTdirection = dot(go->transform.position - position, direction);
		if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && goOriginDOTdirection < resultDistance && goOriginDOTdirection > 0.0f)
		{
			resultDistance = goOriginDOTdirection;
			result = go;
		}
	}
	for (size_t i = 0; i < chunk->staticColliders.size(); ++i)
	{
		GameObject* go = chunk->staticColliders[i];
		vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
		vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
		F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
		F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
		F32 goOriginDOTdirection = dot(go->transform.position - position, direction);
		if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && goOriginDOTdirection < resultDistance && goOriginDOTdirection > 0.0f)
		{
			resultDistance = goOriginDOTdirection;
			result = go;
		}
	}
	for (size_t i = 0; i < chunk->phantoms.size(); ++i)
	{
		GameObject* go = chunk->phantoms[i];
		vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
		vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
		F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
		F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
		F32 goOriginDOTdirection = dot(go->transform.position - position, direction);
		if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && goOriginDOTdirection < resultDistance && goOriginDOTdirection > 0.0f)
		{
			resultDistance = goOriginDOTdirection;
			result = go;
		}
	}

	return result;
}

GameObject* CollisionWorld_2D::RaycastFirst(vec2 position, vec2 direction, GameObjectType type)
{
	GameObject* result;

	F32 resultDistance = 500.0f;
	vec2 rayPerp = Perpendicular_2D(direction);

	vec2 chunkNumber = this->GetChunkContainingPosition(position);
	size_t chunkIndex = this->GetIndex(chunkNumber);
	CollisionChunk_2D* chunk = &(this->chunks[chunkIndex]);

	// Check each bucket.
	for (size_t i = 0; i < chunk->activeColliders.size(); ++i)
	{
		GameObject* go = chunk->activeColliders[i];
		GameObjectType goType = go->GetType();

		if (goType == type)
		{
			vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
			vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
			F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
			F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
			F32 goOriginDOTdirection = dot(go->transform.position - position, direction);
			if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && goOriginDOTdirection < resultDistance && goOriginDOTdirection > 0.0f)
			{
				resultDistance = goOriginDOTdirection;
				result = go;
			}
		}
	}
	for (size_t i = 0; i < chunk->staticColliders.size(); ++i)
	{
		GameObject* go = chunk->staticColliders[i];
		GameObjectType goType = go->GetType();

		if (goType == type)
		{
			vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
			vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
			F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
			F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
			F32 goOriginDOTdirection = dot(go->transform.position - position, direction);
			if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && goOriginDOTdirection < resultDistance && goOriginDOTdirection > 0.0f)
			{
				resultDistance = goOriginDOTdirection;
				result = go;
			}
		}
	}
	for (size_t i = 0; i < chunk->phantoms.size(); ++i)
	{
		GameObject* go = chunk->phantoms[i];
		GameObjectType goType = go->GetType();

		if (goType == type)
		{
			vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
			vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
			F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
			F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
			F32 goOriginDOTdirection = dot(go->transform.position - position, direction);
			if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && goOriginDOTdirection < resultDistance && goOriginDOTdirection > 0.0f)
			{
				resultDistance = goOriginDOTdirection;
				result = go;
			}
		}
	}

	return result;
}

GameObject* CollisionWorld_2D::RaycastFirst(vec2 position, vec2 direction, GameObjectTags tag)
{
	GameObject* result;

	F32 resultDistance = 500.0f;
	vec2 rayPerp = Perpendicular_2D(direction);

	vec2 chunkNumber = this->GetChunkContainingPosition(position);
	size_t chunkIndex = this->GetIndex(chunkNumber);
	CollisionChunk_2D* chunk = &(this->chunks[chunkIndex]);

	// Check each bucket.
	for (size_t i = 0; i < chunk->activeColliders.size(); ++i)
	{
		GameObject* go = chunk->activeColliders[i];
		bool hasTag = go->HasTag(tag);

		if (hasTag)
		{
			vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
			vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
			F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
			F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
			F32 goOriginDOTdirection = dot(go->transform.position - position, direction);
			if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && goOriginDOTdirection < resultDistance && goOriginDOTdirection > 0.0f)
			{
				resultDistance = goOriginDOTdirection;
				result = go;
			}
		}
	}
	for (size_t i = 0; i < chunk->staticColliders.size(); ++i)
	{
		GameObject* go = chunk->staticColliders[i];
		bool hasTag = go->HasTag(tag);

		if (hasTag)
		{
			vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
			vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
			F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
			F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
			F32 goOriginDOTdirection = dot(go->transform.position - position, direction);
			if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && goOriginDOTdirection < resultDistance && goOriginDOTdirection > 0.0f)
			{
				resultDistance = goOriginDOTdirection;
				result = go;
			}
		}
	}
	for (size_t i = 0; i < chunk->phantoms.size(); ++i)
	{
		GameObject* go = chunk->phantoms[i];
		bool hasTag = go->HasTag(tag);

		if (hasTag)
		{
			vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
			vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
			F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
			F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
			F32 goOriginDOTdirection = dot(go->transform.position - position, direction);
			if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && goOriginDOTdirection < resultDistance && goOriginDOTdirection > 0.0f)
			{
				resultDistance = goOriginDOTdirection;
				result = go;
			}
		}
	}

	return result;
}

GameObject* CollisionWorld_2D::RaycastFirst(vec2 position, vec2 direction, GameObjectType type, GameObjectTags tag)
{
	GameObject* result;

	F32 resultDistance = 500.0f;
	vec2 rayPerp = Perpendicular_2D(direction);

	vec2 chunkNumber = this->GetChunkContainingPosition(position);
	size_t chunkIndex = this->GetIndex(chunkNumber);
	CollisionChunk_2D* chunk = &(this->chunks[chunkIndex]);

	// Check each bucket.
	for (size_t i = 0; i < chunk->activeColliders.size(); ++i)
	{
		GameObject* go = chunk->activeColliders[i];
		GameObjectType goType = go->GetType();
		bool hasTag = go->HasTag(tag);

		if (goType == type && hasTag)
		{
			vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
			vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
			F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
			F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
			F32 goOriginDOTdirection = dot(go->transform.position - position, direction);
			if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && goOriginDOTdirection < resultDistance && goOriginDOTdirection > 0.0f)
			{
				resultDistance = goOriginDOTdirection;
				result = go;
			}
		}
	}
	for (size_t i = 0; i < chunk->staticColliders.size(); ++i)
	{
		GameObject* go = chunk->staticColliders[i];
		GameObjectType goType = go->GetType();
		bool hasTag = go->HasTag(tag);

		if (goType == type && hasTag)
		{
			vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
			vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
			F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
			F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
			F32 goOriginDOTdirection = dot(go->transform.position - position, direction);
			if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && goOriginDOTdirection < resultDistance && goOriginDOTdirection > 0.0f)
			{
				resultDistance = goOriginDOTdirection;
				result = go;
			}
		}
	}
	for (size_t i = 0; i < chunk->phantoms.size(); ++i)
	{
		GameObject* go = chunk->phantoms[i];
		GameObjectType goType = go->GetType();
		bool hasTag = go->HasTag(tag);

		if (goType == type && hasTag)
		{
			vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
			vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
			F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
			F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
			F32 goOriginDOTdirection = dot(go->transform.position - position, direction);
			if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && goOriginDOTdirection < resultDistance && goOriginDOTdirection > 0.0f)
			{
				resultDistance = goOriginDOTdirection;
				result = go;
			}
		}
	}

	return result;
}



vector<GameObject*> CollisionWorld_2D::RaycastAll(vec2 position, vec2 direction)
{
	vector<GameObject*> result;

	F32 distance = 500.0f;
	vec2 chunkNumber = this->GetChunkContainingPosition(position);
	size_t chunkIndex = this->GetIndex(chunkNumber);
	CollisionChunk_2D* chunk = &(this->chunks[chunkIndex]);

	vec2 rayPerp = Perpendicular_2D(direction);
	for (size_t i = 0; i < chunk->activeColliders.size(); ++i)
	{
		GameObject* go = chunk->activeColliders[i];
		vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
		vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
		F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
		F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
		if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && dot(go->transform.position - position, direction) < distance && dot(go->transform.position - position, direction) > 0.0f)
		{
			result.push_back(go);
		}
	}
	for (size_t i = 0; i < chunk->staticColliders.size(); ++i)
	{
		GameObject* go = chunk->staticColliders[i];
		vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
		vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
		F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
		F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
		if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && dot(go->transform.position - position, direction) < distance && dot(go->transform.position - position, direction) > 0.0f)
		{
			result.push_back(go);
		}
	}
	for (size_t i = 0; i < chunk->phantoms.size(); ++i)
	{
		GameObject* go = chunk->phantoms[i];
		vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
		vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
		F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
		F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
		if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && dot(go->transform.position - position, direction) < distance && dot(go->transform.position - position, direction) > 0.0f)
		{
			result.push_back(go);
		}
	}

	return result;
}

vector<GameObject*> CollisionWorld_2D::RaycastAll(vec2 position, vec2 direction, GameObjectType type)
{
	vector<GameObject*> result;

	F32 distance = 500.0f;
	vec2 chunkNumber = this->GetChunkContainingPosition(position);
	size_t chunkIndex = this->GetIndex(chunkNumber);
	CollisionChunk_2D* chunk = &(this->chunks[chunkIndex]);

	vec2 rayPerp = Perpendicular_2D(direction);
	for (size_t i = 0; i < chunk->activeColliders.size(); ++i)
	{
		GameObject* go = chunk->activeColliders[i];
		GameObjectType goType = go->GetType();

		if (goType == type)
		{
			vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
			vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
			F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
			F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
			if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && dot(go->transform.position - position, direction) < distance && dot(go->transform.position - position, direction) > 0.0f)
			{
				result.push_back(go);
			}
		}
	}
	for (size_t i = 0; i < chunk->staticColliders.size(); ++i)
	{
		GameObject* go = chunk->staticColliders[i];
		GameObjectType goType = go->GetType();

		if (goType == type)
		{
			vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
			vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
			F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
			F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
			if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && dot(go->transform.position - position, direction) < distance && dot(go->transform.position - position, direction) > 0.0f)
			{
				result.push_back(go);
			}
		}
	}
	for (size_t i = 0; i < chunk->phantoms.size(); ++i)
	{
		GameObject* go = chunk->phantoms[i];
		GameObjectType goType = go->GetType();

		if (goType == type)
		{
			vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
			vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
			F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
			F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
			if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && dot(go->transform.position - position, direction) < distance && dot(go->transform.position - position, direction) > 0.0f)
			{
				result.push_back(go);
			}
		}
	}

	return result;
}

vector<GameObject*> CollisionWorld_2D::RaycastAll(vec2 position, vec2 direction, GameObjectTags tag)
{
	vector<GameObject*> result;

	F32 distance = 500.0f;
	vec2 chunkNumber = this->GetChunkContainingPosition(position);
	size_t chunkIndex = this->GetIndex(chunkNumber);
	CollisionChunk_2D* chunk = &(this->chunks[chunkIndex]);

	vec2 rayPerp = Perpendicular_2D(direction);
	for (size_t i = 0; i < chunk->activeColliders.size(); ++i)
	{
		GameObject* go = chunk->activeColliders[i];
		bool hasTag = go->HasTag(tag);

		if (hasTag)
		{
			vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
			vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
			F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
			F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
			if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && dot(go->transform.position - position, direction) < distance && dot(go->transform.position - position, direction) > 0.0f)
			{
				result.push_back(go);
			}
		}
	}
	for (size_t i = 0; i < chunk->staticColliders.size(); ++i)
	{
		GameObject* go = chunk->staticColliders[i];
		bool hasTag = go->HasTag(tag);

		if (hasTag)
		{
			vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
			vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
			F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
			F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
			if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && dot(go->transform.position - position, direction) < distance && dot(go->transform.position - position, direction) > 0.0f)
			{
				result.push_back(go);
			}
		}
	}
	for (size_t i = 0; i < chunk->phantoms.size(); ++i)
	{
		GameObject* go = chunk->phantoms[i];
		bool hasTag = go->HasTag(tag);

		if (hasTag)
		{
			vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
			vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
			F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
			F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
			if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && dot(go->transform.position - position, direction) < distance && dot(go->transform.position - position, direction) > 0.0f)
			{
				result.push_back(go);
			}
		}
	}

	return result;
}

vector<GameObject*> CollisionWorld_2D::RaycastAll(vec2 position, vec2 direction, GameObjectType type, GameObjectTags tag)
{
	vector<GameObject*> result;

	F32 distance = 500.0f;
	vec2 chunkNumber = this->GetChunkContainingPosition(position);
	size_t chunkIndex = this->GetIndex(chunkNumber);
	CollisionChunk_2D* chunk = &(this->chunks[chunkIndex]);

	vec2 rayPerp = Perpendicular_2D(direction);
	for (size_t i = 0; i < chunk->activeColliders.size(); ++i)
	{
		GameObject* go = chunk->activeColliders[i];
		GameObjectType goType = go->GetType();
		bool hasTag = go->HasTag(tag);

		if (goType == type && hasTag)
		{
			vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
			vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
			F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
			F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
			if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && dot(go->transform.position - position, direction) < distance && dot(go->transform.position - position, direction) > 0.0f)
			{
				result.push_back(go);
			}
		}
	}
	for (size_t i = 0; i < chunk->staticColliders.size(); ++i)
	{
		GameObject* go = chunk->staticColliders[i];
		GameObjectType goType = go->GetType();
		bool hasTag = go->HasTag(tag);

		if (goType == type && hasTag)
		{
			vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
			vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
			F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
			F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
			if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && dot(go->transform.position - position, direction) < distance && dot(go->transform.position - position, direction) > 0.0f)
			{
				result.push_back(go);
			}
		}
	}
	for (size_t i = 0; i < chunk->phantoms.size(); ++i)
	{
		GameObject* go = chunk->phantoms[i];
		GameObjectType goType = go->GetType();
		bool hasTag = go->HasTag(tag);

		if (goType == type && hasTag)
		{
			vec2 farthestRight = go->collisionShape->Support(go->transform, rayPerp) - position;
			vec2 farthestLeft = go->collisionShape->Support(go->transform, -rayPerp) - position;
			F32 farthestRightDOTrayPerp = dot(rayPerp, farthestRight);
			F32 farthestLeftDOTraPerp = dot(rayPerp, farthestLeft);
			if (farthestRightDOTrayPerp > 0.0f && farthestLeftDOTraPerp <= 0.0f && dot(go->transform.position - position, direction) < distance && dot(go->transform.position - position, direction) > 0.0f)
			{
				result.push_back(go);
			}
		}
	}

	return result;
}



vector<GameObject*> CollisionWorld_2D::Shapecast_Rectangle(vec2 position, vec2 halfdim, F32 rotationAngle)
{
	vector<GameObject*> result;

	Rectangle_2D collisionRect = Rectangle_2D(halfdim);
	
	Transform collisionRectTransform;
	collisionRectTransform.position = position;
	collisionRectTransform.rotationAngle = rotationAngle;
	
	vec2 chunkNumber = this->GetChunkContainingPosition(position);
	size_t chunkIndex = this->GetIndex(chunkNumber);
	CollisionChunk_2D* chunk = &(this->chunks[chunkIndex]);

	for (size_t i = 0; i < chunk->activeColliders.size(); ++i)
	{
		GameObject* go = chunk->activeColliders[i];
		CollisionInfo_2D ci = DetectCollision_2D(&collisionRect, collisionRectTransform, go->collisionShape, go->transform);
		if (ci.collided)
		{
			result.push_back(go);
		}
	}
	for (size_t i = 0; i < chunk->staticColliders.size(); ++i)
	{
		GameObject* go = chunk->staticColliders[i];
		CollisionInfo_2D ci = DetectCollision_2D(&collisionRect, collisionRectTransform, go->collisionShape, go->transform);
		if (ci.collided)
		{
			result.push_back(go);
		}
	}
	for (size_t i = 0; i < chunk->phantoms.size(); ++i)
	{
		GameObject* go = chunk->phantoms[i];
		CollisionInfo_2D ci = DetectCollision_2D(&collisionRect, collisionRectTransform, go->collisionShape, go->transform);
		if (ci.collided)
		{
			result.push_back(go);
		}
	}

	return result;
}

vector<GameObject*> CollisionWorld_2D::Shapecast_Rectangle(vec2 position, vec2 halfdim, F32 rotationAngle, GameObjectType type)
{
	vector<GameObject*> result;

	Rectangle_2D collisionRect = Rectangle_2D(halfdim);

	Transform collisionRectTransform;
	collisionRectTransform.position = position;
	collisionRectTransform.rotationAngle = rotationAngle;

	vec2 chunkNumber = this->GetChunkContainingPosition(position);
	size_t chunkIndex = this->GetIndex(chunkNumber);
	CollisionChunk_2D* chunk = &(this->chunks[chunkIndex]);

	for (size_t i = 0; i < chunk->activeColliders.size(); ++i)
	{
		GameObject* go = chunk->activeColliders[i];
		GameObjectType goType = go->GetType();

		if (goType == type)
		{
			CollisionInfo_2D ci = DetectCollision_2D(&collisionRect, collisionRectTransform, go->collisionShape, go->transform);
			if (ci.collided)
			{
				result.push_back(go);
			}
		}
	}
	for (size_t i = 0; i < chunk->staticColliders.size(); ++i)
	{
		GameObject* go = chunk->staticColliders[i];
		GameObjectType goType = go->GetType();

		if (goType == type)
		{
			CollisionInfo_2D ci = DetectCollision_2D(&collisionRect, collisionRectTransform, go->collisionShape, go->transform);
			if (ci.collided)
			{
				result.push_back(go);
			}
		}
	}
	for (size_t i = 0; i < chunk->phantoms.size(); ++i)
	{
		GameObject* go = chunk->phantoms[i];
		GameObjectType goType = go->GetType();

		if (goType == type)
		{
			CollisionInfo_2D ci = DetectCollision_2D(&collisionRect, collisionRectTransform, go->collisionShape, go->transform);
			if (ci.collided)
			{
				result.push_back(go);
			}
		}
	}

	return result;
}

vector<GameObject*> CollisionWorld_2D::Shapecast_Rectangle(vec2 position, vec2 halfdim, F32 rotationAngle, GameObjectTags tag)
{
	vector<GameObject*> result;

	Rectangle_2D collisionRect = Rectangle_2D(halfdim);

	Transform collisionRectTransform;
	collisionRectTransform.position = position;
	collisionRectTransform.rotationAngle = rotationAngle;

	vec2 chunkNumber = this->GetChunkContainingPosition(position);
	size_t chunkIndex = this->GetIndex(chunkNumber);
	CollisionChunk_2D* chunk = &(this->chunks[chunkIndex]);

	for (size_t i = 0; i < chunk->activeColliders.size(); ++i)
	{
		GameObject* go = chunk->activeColliders[i];
		bool hasTag = go->HasTag(tag);

		if (hasTag)
		{
			CollisionInfo_2D ci = DetectCollision_2D(&collisionRect, collisionRectTransform, go->collisionShape, go->transform);
			if (ci.collided)
			{
				result.push_back(go);
			}
		}
	}
	for (size_t i = 0; i < chunk->staticColliders.size(); ++i)
	{
		GameObject* go = chunk->staticColliders[i];
		bool hasTag = go->HasTag(tag);

		if (hasTag)
		{
			CollisionInfo_2D ci = DetectCollision_2D(&collisionRect, collisionRectTransform, go->collisionShape, go->transform);
			if (ci.collided)
			{
				result.push_back(go);
			}
		}
	}
	for (size_t i = 0; i < chunk->phantoms.size(); ++i)
	{
		GameObject* go = chunk->phantoms[i];
		bool hasTag = go->HasTag(tag);

		if (hasTag)
		{
			CollisionInfo_2D ci = DetectCollision_2D(&collisionRect, collisionRectTransform, go->collisionShape, go->transform);
			if (ci.collided)
			{
				result.push_back(go);
			}
		}
	}

	return result;
}

vector<GameObject*> CollisionWorld_2D::Shapecast_Rectangle(vec2 position, vec2 halfdim, F32 rotationAngle, GameObjectType type, GameObjectTags tag)
{
	vector<GameObject*> result;

	Rectangle_2D collisionRect = Rectangle_2D(halfdim);

	Transform collisionRectTransform;
	collisionRectTransform.position = position;
	collisionRectTransform.rotationAngle = rotationAngle;

	vec2 chunkNumber = this->GetChunkContainingPosition(position);
	size_t chunkIndex = this->GetIndex(chunkNumber);
	CollisionChunk_2D* chunk = &(this->chunks[chunkIndex]);

	for (size_t i = 0; i < chunk->activeColliders.size(); ++i)
	{
		GameObject* go = chunk->activeColliders[i];
		GameObjectType goType = go->GetType();
		bool hasTag = go->HasTag(tag);

		if (goType == type && hasTag)
		{
			CollisionInfo_2D ci = DetectCollision_2D(&collisionRect, collisionRectTransform, go->collisionShape, go->transform);
			if (ci.collided)
			{
				result.push_back(go);
			}
		}
	}
	for (size_t i = 0; i < chunk->staticColliders.size(); ++i)
	{
		GameObject* go = chunk->staticColliders[i];
		GameObjectType goType = go->GetType();
		bool hasTag = go->HasTag(tag);

		if (goType == type && hasTag)
		{
			CollisionInfo_2D ci = DetectCollision_2D(&collisionRect, collisionRectTransform, go->collisionShape, go->transform);
			if (ci.collided)
			{
				result.push_back(go);
			}
		}
	}
	for (size_t i = 0; i < chunk->phantoms.size(); ++i)
	{
		GameObject* go = chunk->phantoms[i];
		GameObjectType goType = go->GetType();
		bool hasTag = go->HasTag(tag);

		if (goType == type && hasTag)
		{
			CollisionInfo_2D ci = DetectCollision_2D(&collisionRect, collisionRectTransform, go->collisionShape, go->transform);
			if (ci.collided)
			{
				result.push_back(go);
			}
		}
	}

	return result;
}



vector<CollisionChunkRectangle> CollisionWorld_2D::Debug_GetChunkOutlines()
{
	vector<CollisionChunkRectangle> result;
	result.reserve(this->chunks.size());

	for (size_t i = 0; i < this->chunks.size(); ++i)
	{
		CollisionChunk_2D* chunk = &(this->chunks[i]);
		result.push_back(CollisionChunkRectangle(chunk->dimensions, chunk->transform));
	}

	return result;
}

size_t CollisionWorld_2D::GetIndex(vec2 xy)
{
	size_t result;

	result = (size_t)xy.x + ((size_t)xy.y * (size_t)this->numberOfScreens.x);

	return result;
}

size_t CollisionWorld_2D::GetIndex(size_t x, size_t y)
{
	size_t result;

	result = x + (y * (size_t)this->numberOfScreens.x);

	return result;
}

vec2 CollisionWorld_2D::GetChunkContainingPosition(vec2 position)
{
	vec2 result;

	vec2 screenSize = VVD(this->size, this->numberOfScreens);
	vec2 prospectiveChunk = VVD((position - this->originOffset), screenSize);
	prospectiveChunk.x = (F32)((I32)prospectiveChunk.x);
	prospectiveChunk.y = (F32)((I32)prospectiveChunk.y);
	
	prospectiveChunk.x = ClampRange_F32(prospectiveChunk.x, 0, this->numberOfScreens.x - 1);
	prospectiveChunk.y = ClampRange_F32(prospectiveChunk.y, 0, this->numberOfScreens.y - 1);

	return result = prospectiveChunk;
}

Range_2D_size_t CollisionWorld_2D::GetRange(vec2 chunkNumber, AxisAlignedBoundingBox goAABB)
{
	Range_2D_size_t result;

	size_t chunkIndex = this->GetIndex(chunkNumber);
	CollisionChunk_2D* chunk = &(this->chunks[chunkIndex]);
	vec2 chunkPosition = chunk->transform.position;
	vec2 chunkHalfDim = chunk->dimensions.halfDim;
	vec2 chunkUpperLeft = chunkPosition + vec2(-chunkHalfDim.x, chunkHalfDim.y);
	vec2 chunkBottomRight = chunkPosition + vec2(chunkHalfDim.x, -chunkHalfDim.y);

	size_t startX = (size_t)chunkNumber.x;
	size_t endX = (size_t)chunkNumber.x;
	size_t startY = (size_t)chunkNumber.y;
	size_t endY = (size_t)chunkNumber.y;
	if (goAABB.upperLeft.x < chunkUpperLeft.x && startX > 0)
	{
		--startX;
	}
	if (goAABB.bottomRight.x > chunkBottomRight.x && endX < this->numberOfScreens.x - 1)
	{
		++endX;
	}
	if (goAABB.bottomRight.y < chunkBottomRight.y && startY > 0)
	{
		--startY;
	}
	if (goAABB.upperLeft.y > chunkUpperLeft.y && endY < this->numberOfScreens.y - 1)
	{
		++endY;
	}

	result.x = Range_size_t(startX, endX);
	result.y = Range_size_t(startY, endY);

	return result;
}

vector<size_t> CollisionWorld_2D::GetIndicesFromRange_2D(Range_2D_size_t range)
{
	vector<size_t> result;

	for (size_t y = range.y.min; y <= range.y.max; ++y)
	{
		for (size_t x = range.x.min; x <= range.x.max; ++x)
		{
			size_t index = this->GetIndex(x, y);
			result.push_back(index);
		}
	}

	return result;
}