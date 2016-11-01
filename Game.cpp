#include "Types.h"
#include "Math.h"

#include "_GameAPI.h"
#include "_SystemAPI.h"
#include "_GameplayAPI.h"

#include "BitManip.h"
#include "Util.h"
#include "RandomNumberGenerator.h"
#include "String.h"
#include "MergeSort.h"

#include "AssetLoading.h"
#include "AssetNameToFilepathTable.h"

#include "Renderer.h"
#include "CollisionDetection2DObj.h"
#include "GameObject.h"
#include "CollisionWorld2D.h"
#include "Event.h"

#include "StringAPI_Tests.h"

#include "Prefabs.h"

#include "GameObject_Implementation.h"
#include "CollisionWorld2D_Implementation.h"

#include <vector>
using std::vector;
#include <typeinfo>


#define GAME 1


// NOTE: These are half dimensions
const vec2 TileDimensions = vec2(0.5f, 0.5f);
const vec2 ScreenDimensions = vec2(5.0f, 4.0f);

StringStringHashTable spriteAssetFilepathTable;
inline void ReadInSpriteAssets(StringStringHashTable* hash, char* filename)
{
	GetFileSizeReturnType assetFileSize = GetFileSize(filename);
	if (assetFileSize.fileExists)
	{
		char* fullFile = (char*)malloc(sizeof(char)* (assetFileSize.fileSize + 1));
		ReadFileReturnType readStats = ReadFile(filename, fullFile, assetFileSize.fileSize);
		fullFile[assetFileSize.fileSize] = '\0';
		SplitResult lines = Split(fullFile, assetFileSize.fileSize, "\r\n", 3);
		free(fullFile);

		for (size_t i = 0; i < lines.numberOfComponents; ++i)
		{
			SplitResult lineParts = Split(lines.components[i], lines.componentLengths[i], '=');
			Assert(lineParts.numberOfComponents == 2);

			const char* assetName = lineParts.components[0];
			const char* assetPath = lineParts.components[1];
			hash->AddKVPair(assetName, assetPath);

			lineParts.Destroy();
		}
		lines.Destroy();
	}
};





// Initializations for GameObject
vector<GameObject*> GameObject::gameObjects;
vector<GameObject*> GameObject::gameObjectDestructionQueue;
vector<GameObject*> GameObject::physicsGameObjects;





/*
 * Gameplay GameObject API Implementation
 */
GameObject* CreateGameObject(GameObjectType type)
{
	GameObject* go = new GameObject();
	go->SetType(type);
	GameObject::gameObjects.push_back(go);
	return go;
}

void DestroyQueuedGameObject(GameObject* gameObject)
{
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

	for (size_t i = 0; i < GameObject::gameObjects.size(); ++i)
	{
		GameObject* go = GameObject::gameObjects[i];
		if (gameObject == go)
		{
			GameObject::gameObjects.erase(GameObject::gameObjects.begin() + i);
			break;
		}
	}

	gameObject->SetType(GameObjectType::Null);
	gameObject->ClearAllTags();

	delete gameObject;
}

void ClearDestructionQueue()
{
	for (size_t i = 0; i < GameObject::gameObjectDestructionQueue.size(); ++i)
	{
		GameObject* go = GameObject::gameObjectDestructionQueue[i];
		DestroyQueuedGameObject(go);
	}

	GameObject::gameObjectDestructionQueue.clear();
}

// This only queues the GameObject for destruction.
void DestroyGameObject(GameObject* gameObject)
{
	GameObject::gameObjectDestructionQueue.push_back(gameObject);
}


vector<GameObject*> FindGameObjectByType(GameObjectType type)
{
	vector<GameObject*> result;

	for (size_t i = 0; i < GameObject::gameObjects.size(); ++i)
	{
		GameObject* go = GameObject::gameObjects[i];
		GameObjectType goType = go->GetType();
		if (goType == type)
		{
			result.push_back(go);
		}
	}

	return result;
}

vector<GameObject*> FindGameObjectByTag(GameObjectTags tag)
{
	vector<GameObject*> result;

	for (size_t i = 0; i < GameObject::gameObjects.size(); ++i)
	{
		GameObject* go = GameObject::gameObjects[i];
		bool hasTag = go->HasTag(tag);
		if (hasTag)
		{
			result.push_back(go);
		}
	}

	return result;
}





/*
 * Gameplay World Manipulation API Implementation
 */
void CreateMap(const char* name, vec2 size, vec2 numberOfScreens, vec2 originOffset = vec2(0.0f, 0.0f), vec2 screenCountOffset = vec2(0.0f, 0.0f))
{
	vec2 screenDimensions = VVD(size, numberOfScreens);

	// Generate Screens
	for (size_t x = (size_t)screenCountOffset.x; x < screenCountOffset.x + numberOfScreens.x; ++x)
	{
		for (size_t y = (size_t)screenCountOffset.y; y < screenCountOffset.y + numberOfScreens.y; ++y)
		{
			vec2 backgroundPosition = vec2((screenDimensions.x * (x - screenCountOffset.x)) + screenDimensions.x / 2.0f, (screenDimensions.y * (y - screenCountOffset.y)) + screenDimensions.y / 2.0f) + originOffset;
			char* assetName = Concat(Concat(Concat(Concat(Concat("background_", name), "_"), ToString((U32)x)), "-"), ToString((U32)y));
			CreateBackground(assetName, backgroundPosition, screenDimensions, true);

			CreateVerticalTransitionBar(backgroundPosition - vec2(screenDimensions.x / 2.0f, 0.0f), screenDimensions.y, true);
			CreateHorizontalTransitionBar(backgroundPosition - vec2(0.0f, screenDimensions.y / 2.0f), screenDimensions.x, true);

			if (x == (screenCountOffset.x + numberOfScreens.x) - 1)
			{
				CreateVerticalTransitionBar(backgroundPosition + vec2(screenDimensions.x / 2.0f, 0.0f), screenDimensions.y, true);
			}

			if (y == (screenCountOffset.y + numberOfScreens.y) - 1)
			{
				CreateHorizontalTransitionBar(backgroundPosition + vec2(0.0f, screenDimensions.y / 2.0f), screenDimensions.x, true);
			}
		}
	}
}





/*
 * Gameplay Event API Implementation
 */
void SendEvent(GameObject* gameObject, Event* e)
{
	//gameObject->HandleEvent(e);
	gameObject->DoEvent(gameObject, e);
}

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
vector<QueuedEvent> queuedEventsToProcess;
vector<QueuedEvent> queuedEventsFromThisFrame;
void QueueEvent(GameObject* gameObject, Event* e, U32 numberOfFramesToWait)
{
	queuedEventsFromThisFrame.push_back(QueuedEvent(gameObject, e, numberOfFramesToWait));
}

void AddThisFramesQueuedEventsToProcessingQueue()
{
	for (size_t i = 0; i < queuedEventsFromThisFrame.size(); ++i)
	{
		queuedEventsToProcess.push_back(queuedEventsFromThisFrame[i]);
	}
	queuedEventsFromThisFrame.clear();
}

void SendQueuedEvents()
{
	for (size_t i = 0; i < queuedEventsToProcess.size(); ++i)
	{
		QueuedEvent* qe = &(queuedEventsToProcess[i]);
		--qe->numberOfFramesToWait;
		if (qe->numberOfFramesToWait <= 0)
		{
			SendEvent(qe->recipient, qe->e);
		}
		size_t qs = queuedEventsToProcess.size();
		size_t tlsz = 1;
	}

	for (I32 i = queuedEventsToProcess.size() - 1; i >= 0; --i)
	{
		QueuedEvent* qe = &(queuedEventsToProcess[i]);
		if (qe->numberOfFramesToWait <= 0)
		{
			queuedEventsToProcess.erase(queuedEventsToProcess.begin() + i);
		}
	}
}





/*
 * Gameplay Collision API Implementation
 */
CollisionWorld_2D collisionWorld;

CollisionWorld_2D*  GetCollisionWorld()
{
	CollisionWorld_2D* result;

	result = &collisionWorld;

	return result;
}

GameObject* RaycastFirst(vec2 position, vec2 direction, F32 distance)
{
	CollisionWorld_2D* collisionWorld = GetCollisionWorld();
	return collisionWorld->RaycastFirst(position, direction);
}

vector<GameObject*> RaycastAll(vec2 position, vec2 direction, F32 distance)
{
	CollisionWorld_2D* collisionWorld = GetCollisionWorld();
	return collisionWorld->RaycastAll(position, direction);
}

vector<GameObject*> RaycastAll(GameObjectType type,vec2 position, vec2 direction, F32 distance)
{
	CollisionWorld_2D* collisionWorld = GetCollisionWorld();
	return collisionWorld->RaycastAll(position, direction, type);
}

vector<GameObject*> Shapecast_Rectangle(vec2 position, vec2 halfDim, F32 rotationAngle)
{
	CollisionWorld_2D* collisionWorld = GetCollisionWorld();
	return collisionWorld->Shapecast_Rectangle(position, halfDim, rotationAngle);
}

vector<GameObject*> Shapecast_Rectangle(GameObjectType type, vec2 position, vec2 halfDim, F32 rotationAngle)
{
	CollisionWorld_2D* collisionWorld = GetCollisionWorld();
	return collisionWorld->Shapecast_Rectangle(position, halfDim, rotationAngle, type);
}





/*
 * Gameplay Control API Implementation
 */
bool pauseAnimations = false;
bool AreAllAnimationsPaused()
{
	bool result;

	result = pauseAnimations;

	return result;
}

void PauseAllAnimations()
{
	pauseAnimations = true;
}

void UnpauseAllAnimations()
{
	pauseAnimations = false;
}



bool freezeNextFrame = false;
bool gameFrozen = false;
F32 freezeTimer = 0.0f;
F32 freezeLength = 0;

bool GameFrozen()
{
	bool result;

	result = gameFrozen;

	return result;
}

void StartFreeze()
{
	freezeNextFrame = false;
	gameFrozen = true;
	freezeTimer = 0.0f;
}

bool WantsToBeFrozen()
{
	bool result;
	result = freezeNextFrame;
	return result;
}

void EndFreeze()
{
	freezeNextFrame = false;
	gameFrozen = false;
	freezeTimer = 0.0f;
	freezeLength = 0.0f;
}

void StepGameFreeze(F32 dt)
{
	if (GameFrozen())
	{
		freezeTimer += dt;
		if (freezeTimer >= freezeLength)
		{
			EndFreeze();
		}
	}
}

// Game freezes take effect starting the NEXT FRAME
void FreezeGame(U32 milliseconds)
{
	if (!(GameFrozen() || WantsToBeFrozen()))
	{
		freezeNextFrame = true;
		freezeLength = (F32)((F64)milliseconds / (F64)1000);
	}
}



Rectangle_2D simSpace_Rect;
Transform simSpace_Transform;

const vec2 simSpace_MINDIMENSIONS = vec2(10.0f, 8.0f);
const vec2 simSpace_Rect_MAXDIMENSIONS = vec2(15.0f, 11.0f);

const F32 loadSpace_SIZEMULTIPLIER = 3.125f;

vec2 GetSimulationSpacePosition()
{
	vec2 result;

	result = simSpace_Transform.position;

	return result;
}

vec2 GetSimulationSpaceHalfDimensions()
{
	vec2 result;

	result = simSpace_Rect.halfDim;

	return result;
}

vec2 GetSimulationSpaceDimensions()
{
	vec2 result;

	result = simSpace_Rect.halfDim * 2.0f;

	return result;
}

GameObject* FindWithSmallestX(GameObject** bin, size_t binCount)
{
	GameObject* result = NULL;

	F32 smallestX = 100000.0f;
	GameObject* smallestX_go = NULL;

	for (size_t i = 0; i < binCount; ++i)
	{
		GameObject* go = bin[i];
		if (go->transform.position.x < smallestX)
		{
			smallestX = go->transform.position.x;
			smallestX_go = go;
		}
	}
	result = smallestX_go;

	return result;
}

GameObject* FindWithLargestX(GameObject** bin, size_t binCount)
{
	GameObject* result = NULL;

	F32 largestX = -100000.0f;
	GameObject* largestX_go = NULL;

	for (size_t i = 0; i < binCount; ++i)
	{
		GameObject* go = bin[i];
		if (go->transform.position.x > largestX)
		{
			largestX = go->transform.position.x;
			largestX_go = go;
		}
	}
	result = largestX_go;

	return result;
}

GameObject* FindWithSmallestY(GameObject** bin, size_t binCount)
{
	GameObject* result = NULL;

	F32 smallestY = 100000.0f;
	GameObject* smallestY_go = NULL;

	for (size_t i = 0; i < binCount; ++i)
	{
		GameObject* go = bin[i];
		if (go->transform.position.x < smallestY)
		{
			smallestY = go->transform.position.x;
			smallestY_go = go;
		}
	}
	result = smallestY_go;

	return result;
}

GameObject* FindWithLargestY(GameObject** bin, size_t binCount)
{
	GameObject* result = NULL;

	F32 largestY = -100000.0f;
	GameObject* largestY_go = NULL;

	for (size_t i = 0; i < binCount; ++i)
	{
		GameObject* go = bin[i];
		if (go->transform.position.y > largestY)
		{
			largestY = go->transform.position.y;
			largestY_go = go;
		}
	}
	result = largestY_go;

	return result;
}

void SetSimulationSpace(vec2 center = vec2(0.0f, 0.0f), vec2 dimensions = simSpace_MINDIMENSIONS)
{
	dimensions.x = ClampRange_F32(dimensions.x, simSpace_MINDIMENSIONS.x, simSpace_Rect_MAXDIMENSIONS.x);
	dimensions.y = ClampRange_F32(dimensions.y, simSpace_MINDIMENSIONS.y, simSpace_Rect_MAXDIMENSIONS.y);

	vec2 halfDim = dimensions / 2.0f;
	simSpace_Rect = Rectangle_2D(halfDim, vec2(0.0f, 0.0f), true);
	simSpace_Transform.position = center;
}

void SetSimulationSpace(GameObject* target)
{
	// Change the simulation space to be centered around the new screen.
	vec2 rayOrigin = target->transform.position;
	vector<GameObject*> rayResults_xPos = RaycastAll(GameObjectType::TransitionBar, rayOrigin, vec2(1.0f, 0.0f), 20.0f);
	vector<GameObject*> rayResults_xNeg = RaycastAll(GameObjectType::TransitionBar, rayOrigin, vec2(-1.0f, 0.0f), 20.0f);
	vector<GameObject*> rayResults_yPos = RaycastAll(GameObjectType::TransitionBar, rayOrigin, vec2(0.0f, 1.0f), 20.0f);
	vector<GameObject*> rayResults_yNeg = RaycastAll(GameObjectType::TransitionBar, rayOrigin, vec2(0.0f, -1.0f), 20.0f);

	GameObject* transitionBar_xPos = FindWithSmallestX(rayResults_xPos._Myfirst, rayResults_xPos.size());
	GameObject* transitionBar_xNeg = FindWithLargestX(rayResults_xNeg._Myfirst, rayResults_xNeg.size());
	GameObject* transitionBar_yPos = FindWithSmallestY(rayResults_yPos._Myfirst, rayResults_yPos.size());
	GameObject* transitionBar_yNeg = FindWithLargestY(rayResults_yNeg._Myfirst, rayResults_yNeg.size());

	vec2 simulationSpaceDimensions;
	simulationSpaceDimensions.x = transitionBar_xPos->transform.position.x - transitionBar_xNeg->transform.position.x;
	simulationSpaceDimensions.y = transitionBar_yPos->transform.position.y - transitionBar_yNeg->transform.position.y;

	vec2 simulationSpaceCenter;
	simulationSpaceCenter.x = transitionBar_xPos->transform.position.x - (simulationSpaceDimensions.x / 2.0f);
	simulationSpaceCenter.y = transitionBar_yPos->transform.position.y - (simulationSpaceDimensions.y / 2.0f);

	SetSimulationSpace(simulationSpaceCenter, simulationSpaceDimensions);
}



vec2 GetLoadSpacePosition()
{
	vec2 result;

	result = GetSimulationSpacePosition();

	return result;
}

vec2 GetLoadSpaceDimensions()
{
	vec2 result;

	result = GetSimulationSpaceDimensions() * loadSpace_SIZEMULTIPLIER;

	return result;
}

vec2 GetLoadSpaceHalfDimensions()
{
	vec2 result;

	result = GetLoadSpaceDimensions() / 2.0f;

	return result;
}





/*
 * Gameplay Rendering API Implementation
 */
Camera* renderCamera;
void SetRenderCamera(Camera* camera)
{
	renderCamera = camera;
}

Camera* GetRenderCamera()
{
	Camera* result;
	result = renderCamera;
	return result;
}





/*
* Gameplay Debug API Implementation
*/
// NOTE: DebugDraw functions are shell functions that draw into the world without needing to supply the rendering camera.
//	the default rendering view is assumed
void DebugDrawPoint(vec2 p, F32 pointSize, vec4 color)
{
	Camera* renderCamera = GetRenderCamera();
	DrawPoint(p, pointSize, color, renderCamera);
}

void DebugDrawLine(vec2 a, vec2 b, vec4 color)
{
	Camera* renderCamera = GetRenderCamera();
	DrawLine(a, b, color, renderCamera);
}

void DebugDrawRectangleOutline(vec2 upperLeft, vec2 dimensions, vec4 color, F32 offset)
{
	Camera* renderCamera = GetRenderCamera();
	DrawRectangleOutline(upperLeft, dimensions, color, renderCamera, offset);
}

void DebugDrawRectangleSolidColor(vec2 halfDim, Transform transform, vec4 color)
{
	Camera* renderCamera = GetRenderCamera();
	DrawRectangle(halfDim, transform, color, renderCamera);
}















/*
* Physics Updating
*/
/* NOTE: Probably include this in GameObject. */
// NOTE: Should be relative to world
void IntegratePhysicsObjects(F32 dt)
{
	for (size_t i = 0; i < GameObject::physicsGameObjects.size(); ++i)
	{
		GameObject* go = GameObject::physicsGameObjects[i];
		go->rigidbody->SetPosition(go->transform.position);
		go->rigidbody->Integrate(dt);
		go->transform.position = go->rigidbody->position;
	}
}







/*
 * Global Game State
 */
bool globalDebugDraw = true;
bool resetDebugStatePerFrame = false;


GameObject* heroGO;
GameObject* cameraGO;

GameObject* flowerGO;
GameObject* treeGO;
GameObject* weedGO1;
GameObject* weedGO2;
GameObject* buttonGO;
GameObject* spooks;

vector<CollisionChunkRectangle> debug_collisionWorldChunks;
vector<CollisionChunkRectangle> debug_cwgoAddChunks;




/*
 * Rendering
 */

void DrawCameraGrid()
{
	// NOTE: Camera space grid, corresponding to tiles
	F32 initialXPos = renderCamera->position.x - 10.0f / 2.0f;
	F32 endXPos = initialXPos + 10.0f;
	F32 posY = renderCamera->position.y + 8.0f / 2.0f;
	F32 negY = renderCamera->position.y - 8.0f / 2.0f;
	vec4 gridLineColor = vec4(.251f, .251f, .251f, 1.0f);
	for (F32 xPos = initialXPos; xPos <= endXPos; ++xPos)
	{
		DrawLine(vec2(xPos, posY), vec2(xPos, negY), gridLineColor, renderCamera);
	}

	F32 initialYPos = renderCamera->position.y - 8.0f / 2.0f;
	F32 endYPos = initialYPos + 8.0f;
	F32 posX = renderCamera->position.x + 10.0f / 2.0f;
	F32 negX = renderCamera->position.x - 10.0f / 2.0f;
	for (F32 yPos = initialYPos; yPos <= endYPos; ++yPos)
	{
		DrawLine(vec2(posX, yPos), vec2(negX, yPos), gridLineColor, renderCamera);
	}

	DrawLine(vec2(0.0f, posY), vec2(0.0f, negY), vec4(0.09f, 0.52f, 0.09f, 1.0f), renderCamera);
	DrawLine(vec2(posX, 0.0f), vec2(negX, 0.0f), vec4(0.52f, 0.09f, 0.09f, 1.0f), renderCamera);
}

bool CompareYPosition(GameObject* a, GameObject* b)
{
	bool result;
	result = a->transform.position.y >= b->transform.position.y;
	return result;
}

void SortBinByYPosition(GameObject** bin, size_t count)
{
	MergeSort(bin, count, CompareYPosition);
}

//NOTE:  Draws do not need dt anymore it was for stepping animations internally which has been removed. remove this.
const vec4 debugCollisionColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
void DrawGameObject(GameObject* gameObject, F32 dt)
{
	bool hasAnimations = gameObject->animator != NULL && gameObject->animator->NumberOfAnimations();
	bool hasSprite = gameObject->sprite != NULL;

	if (hasAnimations)
	{
		TextureHandle animationFrame = gameObject->animator->GetCurrentAnimationFrame();
		vec2 spriteOffset = gameObject->animator->GetSpriteOffset();
		DrawSprite(animationFrame, spriteOffset, gameObject->transform, renderCamera);
	}
	else if (hasSprite)
	{
		DrawSprite(gameObject->sprite->texture, gameObject->sprite->offset, gameObject->transform, renderCamera);
	}

	if (gameObject->collisionShape != NULL && gameObject->debugDraw)
	{
		const size_t rectangleHash = typeid(Rectangle_2D).hash_code();
		const size_t circleHash = typeid(Circle_2D).hash_code();
		const size_t triangleHash = typeid(Triangle_2D).hash_code();

		size_t gameObjectHash = typeid(*(gameObject->collisionShape)).hash_code();
		if (gameObjectHash == rectangleHash)
		{
			Rectangle_2D* rect = (Rectangle_2D*)gameObject->collisionShape;
			if (rect->IsPhantom())
			{
				Transform t = gameObject->transform;
				t.position += rect->GetOffset();
				//DrawRectangle(rect->halfDim, t, vec4(0.0f, 0.0f, 1.0f, 0.3f), renderCamera);
			}
				//vec2 upperLeft = gameObject->transform.position + (vec2(-rect->halfDim.x, rect->halfDim.y) * gameObject->transform.scale) + gameObject->collisionShape->GetOffset();
				//vec2 dimensions = rect->halfDim * 2.0f * gameObject->transform.scale;
				//DrawRectangleOutline(upperLeft, dimensions, debugCollisionColor, renderCamera);
		}
		else if (gameObjectHash == circleHash)
		{
			// NOTE: Draw Circle
		}
		else if (gameObjectHash == triangleHash)
		{
			vec2 trianglePoints[3];
			Triangle_2D* triangle = (Triangle_2D*)gameObject->collisionShape;
			mat3 A = gameObject->transform.LocalToWorldTransform();
			trianglePoints[0] = vec2(A * vec3(triangle->points[0].x, triangle->points[0].y, 1.0f));
			trianglePoints[1] = vec2(A * vec3(triangle->points[1].x, triangle->points[1].y, 1.0f));
			trianglePoints[2] = vec2(A * vec3(triangle->points[2].x, triangle->points[2].y, 1.0f));
			DrawLine(trianglePoints[0], trianglePoints[1], debugCollisionColor, renderCamera);
			DrawLine(trianglePoints[1], trianglePoints[2], debugCollisionColor, renderCamera);
			DrawLine(trianglePoints[2], trianglePoints[0], debugCollisionColor, renderCamera);
		}
	}

	// Draw Position
	if (gameObject->debugDraw)
	{
		//DrawPoint(gameObject->transform.position, 4, vec4(0.0f, 0.0f, 1.0f, 1.0f), renderCamera);
	}
}

void DrawGameObjects(F32 dt)
{
	// NOTE: Draw the game space.
	SetViewport(vec2(0, 0), vec2(600, 480));

	/*
	 * Sort by tag
	 *		Bins
	 *		1 - Background
	 *		2 - Environment
	 *		3 - Everything else sans effects
	 *		4 - Effects
	 * 
	 * Sort bins by position.y (Lowest -> Highest)
	 *
	 * Draw all objects in each bin starting with Bin 1.
	 */
	vector<GameObject*> bin_Background;
	vector<GameObject*> bin_Environment;
	vector<GameObject*> bin_Characters;
	vector<GameObject*> bin_Effects;
	for (size_t i = 0; i < GameObject::gameObjects.size(); ++i)
	{
		GameObject* go = GameObject::gameObjects[i];
		bool hasTag_Background = go->HasTag(GameObjectTags::Background);
		bool hasTag_Environment = go->HasTag(GameObjectTags::Environment);
		bool hasTag_Effect = go->HasTag(GameObjectTags::Effect);

		if (hasTag_Background)
		{
			bin_Background.push_back(go);
		}
		if (hasTag_Environment)
		{
			bin_Environment.push_back(go);
		}
		if (hasTag_Effect)
		{
			bin_Effects.push_back(go);
		}

		if (!hasTag_Background && !hasTag_Environment && !hasTag_Effect)
		{
			bin_Characters.push_back(go);
		}
	}

	SortBinByYPosition(bin_Background._Myfirst, bin_Background.size());
	SortBinByYPosition(bin_Environment._Myfirst, bin_Environment.size());
	SortBinByYPosition(bin_Characters._Myfirst, bin_Characters.size());
	SortBinByYPosition(bin_Effects._Myfirst, bin_Effects.size());

	// Draw Objects
	for (size_t i = 0; i < bin_Background.size(); ++i)
	{
		GameObject* go = bin_Background[i];
		DrawGameObject(go, dt);
		if (go->Debug != NULL)
		{
			go->Debug(go);
		}
	}

	if (globalDebugDraw)
	{
		DrawCameraGrid();
	}


	for (size_t i = 0; i < bin_Environment.size(); ++i)
	{
		GameObject* go = bin_Environment[i];
		DrawGameObject(go, dt);
		if (go->Debug != NULL)
		{
			go->Debug(go);
		}
	}

	for (size_t i = 0; i < bin_Characters.size(); ++i)
	{
		GameObject* go = bin_Characters[i];
		DrawGameObject(go, dt);
		if (go->Debug != NULL)
		{
			go->Debug(go);
		}
	}

	for (size_t i = 0; i < bin_Effects.size(); ++i)
	{
		GameObject* go = bin_Effects[i];
		DrawGameObject(go, dt);
		if (go->Debug != NULL)
		{
			go->Debug(go);
		}
	}

	// Draw Simulation Space Outline.
	vec2 ssDimensions = GetSimulationSpaceDimensions();
	vec2 ssHalfDim = ssDimensions / 2.0f;
	vec2 ssPosition = GetSimulationSpacePosition();
	vec4 ssColor = vec4(0.0f, 0.65f, 1.0f, 1.0f);
	DebugDrawRectangleOutline(ssPosition + vec2(-ssHalfDim.x, ssHalfDim.y), ssDimensions, ssColor);


	// Draw Load Space Outline
	vec2 lsDimensions = GetLoadSpaceDimensions();
	vec2 lsHalfDim = lsDimensions / 2.0f;
	vec2 lsPosition = GetLoadSpacePosition();
	vec4 lsColor = vec4(0.57f, 0.0f, 1.0f, 1.0f);
	DebugDrawRectangleOutline(lsPosition + vec2(-lsHalfDim.x, lsHalfDim.y), lsDimensions, lsColor);



	// NOTE: Draw UI
	/*DrawUVRectangleScreenSpace(&guiPanel, vec2(0, 0), vec2(guiPanel.width, guiPanel.height));*/

}











/*
 * Core Game API Functions
 */
bool GameInitialize()
{
	// NOTE: Initialize game sub systems.
	InitializeRenderer();

	SetWindowTitle("Oracle of Ages Clone");
	SetClientWindowDimensions(vec2(600, 540));
	SetClearColor(vec4(0.2235f, 0.2235f, 0.2235f, 1.0f));

	spriteAssetFilepathTable.Initialize(60);
	ReadInSpriteAssets(&spriteAssetFilepathTable, "Assets.txt");



	bool debugDraw = globalDebugDraw;
	
	collisionWorld = CollisionWorld_2D();
	collisionWorld.Initialize(vec2(100, 72), vec2(10, 9), vec2(-65.0f, -20.0f));

	debug_collisionWorldChunks = collisionWorld.Debug_GetChunkOutlines();

	CreateDancingFlowers(vec2(-0.5f, -1.5f), debugDraw);
	CreateDancingFlowers(vec2(-2.5f, 1.5f), debugDraw);
	CreateWeed(vec2(-2.5f, -0.5f), debugDraw); // Left Group Start
	CreateWeed(vec2(-3.5f, -0.5f), debugDraw);
	CreateWeed(vec2(-3.5f, -1.5f), debugDraw);
	CreateWeed(vec2( 2.5f, -0.5f), debugDraw); // Right Group Start
	CreateWeed(vec2( 1.5f, -1.5f), debugDraw);
	CreateWeed(vec2( 2.5f, -1.5f), debugDraw);
	CreateWeed(vec2( 1.5f, -2.5f), debugDraw);
	CreateWeed(vec2( 2.5f, -2.5f), debugDraw);
	CreateTree(vec2(-3.0f,  4.0f), debugDraw); // Left Group Start
	CreateTree(vec2(-5.0f,  4.0f), debugDraw);
	CreateTree(vec2(-5.0f,  2.0f), debugDraw);
	CreateTree(vec2(-5.0f,  0.0f), debugDraw);
	CreateTree(vec2(-5.0f, -2.0f), debugDraw);
	CreateTree(vec2( 4.0f,  4.0f), debugDraw); // Right Group Start
	CreateTree(vec2( 4.0f,  2.0f), debugDraw);
	CreateTree(vec2( 4.0f,  0.0f), debugDraw);
	CreateTree(vec2( 4.0f, -2.0f), debugDraw);
	CreateBlocker(vec2(0.0f, -3.5f), vec2(10.0f, 1.0f), debugDraw); // Bottom Wall


	
	CreateDancingFlowers(vec2(-0.5f, 6.5f), debugDraw);
	CreateDancingFlowers(vec2(1.5f, 10.5f), debugDraw);
	CreateWeed(vec2(-2.5f, 5.5f), debugDraw); // Left Group Start
	CreateWeed(vec2(-2.5f, 6.5f), debugDraw);
	CreateWeed(vec2(-2.5f, 7.5f), debugDraw);
	CreateTree(vec2(-4.0f, 8.0f), debugDraw); // Left Group Start
	CreateTree(vec2(-4.0f, 10.0f), debugDraw);
	CreateTree(vec2(-3.0f, 12.0f), debugDraw);
	CreateTree(vec2(-5.0f, 12.0f), debugDraw);
	CreateTree(vec2(-4.0f, 8.0f), debugDraw);
	CreateTree(vec2(4.0f, 6.0f), debugDraw); // Right Group Start
	CreateTree(vec2(4.0f, 8.0f), debugDraw);
	CreateSpookyTree(vec2(4.0f, 10.0f), debugDraw); // Right Group Start
	CreateSpookyTree(vec2(4.0f, 12.0f), debugDraw);

	heroGO = CreateHero(vec2(-0.5f,  3.0f)/*vec2(-0.5f, 0.5f)*/, debugDraw);
	cameraGO = CreatePlayerCamera(vec2(0.0f, 0.0f), debugDraw);



	// NOTE: Add all maps.
	CreateMap("present_worldMap", vec2(100, 72), vec2(10, 9), vec2(-65.0f, -20.0f), vec2(4, 3));
	//CreateMap("past_blackTower", vec2(45, 22), vec2(3, 2), vec2(-22.5f, -11.5f));
	//CreateMap("past_makuPath", vec2(45, 44), vec2(3, 4), vec2(-22.5f, -22.0f));
	//CreateMap("past_townHouses", vec2(60, 8), vec2(6, 1), vec2(-30.0f, 0.0f));
	//CreateMap("past_worldMap", vec2(60, 64), vec2(6, 8), vec2(-30.0f, -32.0f));
	//CreateMap("present_poeGrave", vec2(10, 8), vec2(1, 1), vec2(-5.0f, 0.0f));
	//CreateMap("present_skullCave", vec2(15, 11), vec2(1, 1), vec2(-7.5f, 0.0f));
	//CreateMap("present_makuPath", vec2(15, 44), vec2(1, 4), vec2(-7.5f, -22.0f));
	//CreateMap("present_townHouses", vec2(130, 8), vec2(13, 1), vec2(-65.0f, 0.0f));
	//CreateMap("present_spiritsGrave", vec2(75, 77), vec2(5, 7), vec2(-37.5f, -38.5f));



	SetSimulationSpace(heroGO); // NOTE: This could probably be moved into the update loop and out of transition bars/ portals
	return true;
}

void UpdateGameObjects_PrePhysics(vector<GameObject*> &gameObjects, F32 dt)
{
	for (size_t i = 0; i < gameObjects.size(); ++i)
	{
		GameObject* go = gameObjects[i];
		if (go->Update_Pre != NULL)
		{
			go->Update_Pre(go, dt);
		}
	}
}

void UpdateGameObjects_PostPhysics(vector<GameObject*> &gameObjects, F32 dt)
{
	for (size_t i = 0; i < gameObjects.size(); ++i)
	{
		GameObject* go = gameObjects[i];
		if (go->Update_Post != NULL)
		{
			go->Update_Post(go, dt);
		}
	}
}

void AdvanceAnimations(F32 dt)
{
	for (size_t i = 0; i < GameObject::gameObjects.size(); ++i)
	{
		GameObject* go = GameObject::gameObjects[i];
		bool hasAnimations = go->animator != NULL && go->animator->NumberOfAnimations();
		if (hasAnimations)
		{
			go->animator->StepElapsedAnimationTime(dt);
		}
	}
}

void ReconcileCameras()
{
	for (size_t i = 0; i < GameObject::gameObjects.size(); ++i)
	{
		GameObject* go = GameObject::gameObjects[i];
		GameObjectType goType = go->GetType();
		if (goType == GameObjectType::PlayerCamera)
		{
			go->camera->position = go->transform.position;
			go->camera->rotationAngle = go->transform.rotationAngle;
		}
	}
}

void DebugDrawGameObjects()
{
	for (size_t i = 0; i < GameObject::gameObjects.size(); ++i)
	{
		GameObject::gameObjects[i]->Debug(GameObject::gameObjects[i]);
	}
}



vector<GameObject*> GenerateUpdateBin()
{
	vector<GameObject*> result;

	for (size_t i = 0; i < GameObject::gameObjects.size(); ++i)
	{
		GameObject* go = GameObject::gameObjects[i];
		if (go->ForcesUpdates())
		{
			result.push_back(go);
			continue;
		}
		
		vec2 goPosition = go->transform.position;

		vec2 simulationSpacePosition = GetSimulationSpacePosition();
		vec2 simulationSpaceHalfDimensions = GetSimulationSpaceHalfDimensions();
		vec2 simulationSpaceUpperLeft = simulationSpacePosition + vec2(-simulationSpaceHalfDimensions.x, simulationSpaceHalfDimensions.y);
		vec2 simulationSpaceLowerRight = simulationSpacePosition + vec2(simulationSpaceHalfDimensions.x, -simulationSpaceHalfDimensions.y);

		if (goPosition.x >= simulationSpaceUpperLeft.x && goPosition.x <= simulationSpaceLowerRight.x &&
			goPosition.y >= simulationSpaceLowerRight.y && goPosition.y <= simulationSpaceUpperLeft.y)
		{
			result.push_back(go);
		}
	}

	return result;
}

void DebugDrawUpdateBin(vector<GameObject*> *bin)
{
	vec4 updateBinColor = vec4(0.0f, 1.0f, 1.0f, 1.0f);
	for (size_t i = 0; i < bin->size(); ++i)
	{
		GameObject* go = (*bin)[i];

		Transform unscaledTransform = go->transform;
		unscaledTransform.scale = vec2(1.0f, 1.0f);
		DebugDrawRectangleSolidColor((TileDimensions * 0.2f), unscaledTransform, updateBinColor);
	}
}



vector<GameObject*> GenerateLoadBin()
{
	vector<GameObject*> result;

	for (size_t i = 0; i < GameObject::gameObjects.size(); ++i)
	{
		GameObject* go = GameObject::gameObjects[i];
		vec2 goPosition = go->transform.position;

		vec2 simulationSpacePosition = GetLoadSpacePosition();
		vec2 simulationSpaceHalfDimensions = GetLoadSpaceHalfDimensions();
		vec2 simulationSpaceUpperLeft = simulationSpacePosition + vec2(-simulationSpaceHalfDimensions.x, simulationSpaceHalfDimensions.y);
		vec2 simulationSpaceLowerRight = simulationSpacePosition + vec2(simulationSpaceHalfDimensions.x, -simulationSpaceHalfDimensions.y);

		if (goPosition.x >= simulationSpaceUpperLeft.x && goPosition.x <= simulationSpaceLowerRight.x &&
			goPosition.y >= simulationSpaceLowerRight.y && goPosition.y <= simulationSpaceUpperLeft.y)
		{
			result.push_back(go);
		}
	}

	return result;
}

void DebugDrawLoadBin(vector<GameObject*> *bin)
{
	vec4 updateBinColor = vec4(1.0f, 0.5f, 0.0f, 1.0f);
	for (size_t i = 0; i < bin->size(); ++i)
	{
		GameObject* go = (*bin)[i];

		Transform unscaledTransform = go->transform;
		unscaledTransform.scale = vec2(1.0f, 1.0f);
		DebugDrawRectangleSolidColor((TileDimensions * 0.2f), unscaledTransform, updateBinColor);
	}
}



void DrawCollisionChunkRectangles(vector<CollisionChunkRectangle>& chunks, vec4 color)
{
	for (size_t i = 0; i < chunks.size(); ++i)
	{
		vec2 halfDim = chunks[i].dimensions.halfDim;
		vec2 dimensions = 2.0f * halfDim;
		vec2 upperLeft = chunks[i].transform.position + vec2(-halfDim.x, halfDim.y);
		DebugDrawRectangleOutline(upperLeft, dimensions, color);
		//DebugDrawPoint(chunks[i].transform.position, 2.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f));
	}
}


vector<GameObject*> gameObjectsToUpdate;
vector<GameObject*> gameObjectsToLoad;

vector<PenetrationInfo_2D> interpenetrationsFixedLastFrame;
vector<PenetrationInfo_2D> phantomInterpenetrationsFromLastFrame;
bool SameGameObjectsInvolved(const PenetrationInfo_2D& lhs, const PenetrationInfo_2D& rhs)
{
	bool result = true;

	if (lhs.go1 != rhs.go1)
	{
		result = false;
	}
	if (lhs.go2 != rhs.go2)
	{
		result = false;
	}

	return result;
}
void GenerateCollisionEvents(vector<Event>& events_OnCollisionEnter, vector<Event>& events_OnCollision, vector<Event>& events_OnCollisionExit, vector<PenetrationInfo_2D>& interpenetrationsFixed, vector<PenetrationInfo_2D>& interpenetrationsFixedLastFrame)
{
	/* Generate OnCollisionEnter and OnCollsion events */
	for (size_t i = 0; i < interpenetrationsFixed.size(); ++i)
	{
		bool processedLastFrame = false;
		for (size_t j = 0; j < interpenetrationsFixedLastFrame.size(); ++j)
		{
			bool collisionIsTheSame = SameGameObjectsInvolved(interpenetrationsFixed[i], interpenetrationsFixedLastFrame[j]);
			if (collisionIsTheSame)
			{
				processedLastFrame = true;
				interpenetrationsFixedLastFrame.erase(interpenetrationsFixedLastFrame.begin() + j);
				break;
			}
		}
		if (!processedLastFrame)
		{
			// Generate OnCollisionEnter events for this collision.
			GameObject* go1 = interpenetrationsFixed[i].go1;
			GameObject* go2 = interpenetrationsFixed[i].go2;
			vec2 normal = interpenetrationsFixed[i].normal;
	
			Event event1;
			event1.SetType(ET_OnCollisionEnter);
			event1.arguments[0] = EventArgument((void*)go1);
			event1.arguments[1] = EventArgument((void*)go2);
			event1.arguments[2] = EventArgument(normal);
			events_OnCollisionEnter.push_back(event1);
	
			Event event2;
			event2.SetType(ET_OnCollisionEnter);
			event2.arguments[0] = EventArgument((void*)go2);
			event2.arguments[1] = EventArgument((void*)go1);
			event2.arguments[2] = EventArgument(-normal);
			events_OnCollisionEnter.push_back(event2);
		}
	
		// Generate OnCollision event for this collision.
		GameObject* go1 = interpenetrationsFixed[i].go1;
		GameObject* go2 = interpenetrationsFixed[i].go2;
		vec2 normal = interpenetrationsFixed[i].normal;
	
		Event event1;
		event1.SetType(ET_OnCollision);
		event1.arguments[0] = EventArgument((void*)go1);
		event1.arguments[1] = EventArgument((void*)go2);
		event1.arguments[2] = EventArgument(normal);
		events_OnCollision.push_back(event1);
	
		Event event2;
		event2.SetType(ET_OnCollision);
		event2.arguments[0] = EventArgument((void*)go2);
		event2.arguments[1] = EventArgument((void*)go1);
		event2.arguments[2] = EventArgument(-normal);
		events_OnCollision.push_back(event2);
	}
	
	/* Generate OnCollisionExit events */
	for (size_t i = 0; i < interpenetrationsFixedLastFrame.size(); ++i)
	{
		// Because we removed all of the collisions that were fixed this frame too, this list only
		//	contains collisions that didn't happen this frame.
		// NOTE: Events could also be sent directly from this loop AFTER the OnCollisionEnter, and OnCollision
		//	Events are sent because it contains only OnCollisionExit events.
	
		// Generate OnCollisionExit event
		GameObject* go1 = interpenetrationsFixedLastFrame[i].go1;
		GameObject* go2 = interpenetrationsFixedLastFrame[i].go2;
		vec2 normal = interpenetrationsFixedLastFrame[i].normal;
	
		Event event1;
		event1.SetType(ET_OnCollisionExit);
		event1.arguments[0] = EventArgument((void*)go1);
		event1.arguments[1] = EventArgument((void*)go2);
		event1.arguments[2] = EventArgument(normal);
		events_OnCollisionExit.push_back(event1);
	
		Event event2;
		event2.SetType(ET_OnCollisionExit);
		event2.arguments[0] = EventArgument((void*)go2);
		event2.arguments[1] = EventArgument((void*)go1);
		event2.arguments[2] = EventArgument(-normal);
		events_OnCollisionExit.push_back(event2);
	}

}
void SendCollisionEvents(vector<Event>& bin)
{
	for (size_t i = 0; i < bin.size(); ++i)
	{
		GameObject* recipient = (GameObject*)bin[i].arguments[0].AsPointer();
		if (recipient->DoEvent != NULL)
		{
			recipient->DoEvent(recipient, &(bin[i]));
		}
	}
}

F32 time = 0.0f;
F32 skipThreshold = 1.0f;
void GameUpdate(F32 dt)
{
	HighResolutionTimer frameTimer = HighResolutionTimer("Frame Time");
	frameTimer.Start();

	// Handle really long frame times. Mainly for debugging/ moving the window around
	//	we should probably pause the game when the player moves the window around instead of just throwing the frame out 
	//  this is just a temp measure.
	if (dt > skipThreshold)
	{
		return;
	}

	// Handle freezing the game.
	if (WantsToBeFrozen())
	{
		StartFreeze();
	}
	if (GameFrozen())
	{
		StepGameFreeze(dt);
	}

 	Clear();
	if (!GameFrozen())
	{
		gameObjectsToLoad.clear();
		gameObjectsToLoad = GenerateLoadBin();
		for (size_t i = 0; i < gameObjectsToLoad.size(); ++i)
		{
			GameObject* go = gameObjectsToLoad[i];
		
			if (go->sprite)
			{
				go->sprite->Load();
			}
		
			if (go->animator)
			{
				go->animator->Load();
			}
		}

		gameObjectsToUpdate.clear();
		gameObjectsToUpdate = GenerateUpdateBin();
		UpdateGameObjects_PrePhysics(gameObjectsToUpdate, dt);


		HighResolutionTimer cwt = HighResolutionTimer("Collision World Things");
		cwt.Start();
		IntegratePhysicsObjects(dt);
		collisionWorld.FixupActives();
		vector<PenetrationInfo_2D> interpenetrationsFixed = collisionWorld.ResolveInterpenetrations(GetSimulationSpacePosition(), GetSimulationSpaceHalfDimensions());
		vector<PenetrationInfo_2D> phantomInterpenetrations = collisionWorld.GetPhantomInterpenetrations(GetSimulationSpacePosition(), GetSimulationSpaceHalfDimensions());

		vector<Event> events_OnCollisionEnter;
		vector<Event> events_OnCollision;
		vector<Event> events_OnCollisionExit;

		GenerateCollisionEvents(events_OnCollisionEnter, events_OnCollision, events_OnCollisionExit, interpenetrationsFixed, interpenetrationsFixedLastFrame);
		GenerateCollisionEvents(events_OnCollisionEnter, events_OnCollision, events_OnCollisionExit, phantomInterpenetrations, phantomInterpenetrationsFromLastFrame);
		interpenetrationsFixedLastFrame = interpenetrationsFixed;
		phantomInterpenetrationsFromLastFrame = phantomInterpenetrations;
		DebugPrintf(512, "Interpenetrations Resolved = %u\n", interpenetrationsFixed.size());
		DebugPrintf(512, "Phantom Collisions = %u\n", phantomInterpenetrations.size());




		// Send Collision events.
		SendCollisionEvents(events_OnCollisionEnter);
		SendCollisionEvents(events_OnCollision);
		SendCollisionEvents(events_OnCollisionExit);
		cwt.End();
		cwt.Report();

		SendQueuedEvents();

		UpdateGameObjects_PostPhysics(gameObjectsToUpdate, dt);

		AreAllAnimationsPaused() ? NULL : AdvanceAnimations(dt);

		ReconcileCameras();
	}


	DrawGameObjects(dt);
	//DrawCollisionChunkRectangles(debug_collisionWorldChunks, vec4(1.0f, 1.0f, 1.0f, 1.0f));
	//DrawCollisionChunkRectangles(debug_cwgoAddChunks, vec4(1.0f, 0.5f, 0.0f, 1.0f));
	//glFinish();


	if (!GameFrozen())
	{
		AddThisFramesQueuedEventsToProcessingQueue();

		ClearDestructionQueue(); // NOTE: Maybe this should be done before we draw?
	}


	frameTimer.End();
	frameTimer.Report();
	DebugPrintf(64, "\n\n\n");
}

bool GameShutdown()
{
	spriteAssetFilepathTable.Destroy();
	ShutdownRenderer();
	return true;
}