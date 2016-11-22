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

#include "Event.h"
#include "CollisionDetection2DObj.h"
#include "GameObject.h"
#include "CollisionWorld2D.h"
#include "GameMap.h"
#include "GameState.h"

#include "StringAPI_Tests.h"

#include "Prefabs.h"

#include "GameObject_Implementation.h"
#include "CollisionWorld2D_Implementation.h"
#include "GameMap_Implementation.h"
#include "GameState_Implementation.h"

#include <vector>
using std::vector;
#include <typeinfo>








// GameObject Declarations
vector<GameObject*> GameObject::physicsGameObjects;










/*
 * General Game Implementation Utility
 */
GameObject* CreateGameObject(GameObjectType type)
{
	GameObject* go = new GameObject();
	go->SetType(type);
	return go;
}

GameState global;

// NOTE: The following values will eventually be derived from a config file
const char * windowTitle = "Oracle of Ages Clone";
vec2 windowDimensions = vec2(600, 540);





/*
 * GameInitialize and Utility
 */
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

void GameState::Initialize()
{

}

bool GameInitialize()
{
	// NOTE: Initialize game sub systems.
	InitializeRenderer();

	SetWindowTitle(windowTitle);
	SetClientWindowDimensions(windowDimensions);
	SetClearColor(global.clearColor);

	global.spriteAssetFilepathTable.Initialize(60);
	ReadInSpriteAssets(&global.spriteAssetFilepathTable, "Assets.txt");

	// NOTE: Add all maps.
	GameMap* present_worldMap = global.AddGameMap("present_worldMap", vec2(100, 72), vec2(10, 9), vec2(-65.0f, -20.0f), vec2(4, 3));
	//GameMap* past_blackTower = global.AddGameMap("past_blackTower", vec2(45, 22), vec2(3, 2), vec2(-22.5f, -11.5f));
	//GameMap* past_makuPath = global.AddGameMap("past_makuPath", vec2(45, 44), vec2(3, 4), vec2(-22.5f, -22.0f));
	//GameMap* past_townHouses = global.AddGameMap("past_townHouses", vec2(60, 8), vec2(6, 1), vec2(-30.0f, 0.0f));
	//GameMap* past_worldMap = global.AddGameMap("past_worldMap", vec2(60, 64), vec2(6, 8), vec2(-30.0f, -32.0f));
	//GameMap* present_poeGrave = global.AddGameMap("present_poeGrave", vec2(10, 8), vec2(1, 1), vec2(-5.0f, 0.0f));
	//GameMap* present_skullCave = global.AddGameMap("present_skullCave", vec2(15, 11), vec2(1, 1), vec2(-7.5f, 0.0f));
	//GameMap* present_makuPath = global.AddGameMap("present_makuPath", vec2(15, 44), vec2(1, 4), vec2(-7.5f, -22.0f));
	//GameMap* present_townHouses = global.AddGameMap("present_townHouses", vec2(130, 8), vec2(13, 1), vec2(-65.0f, 0.0f));
	//GameMap* present_spiritsGrave = global.AddGameMap("present_spiritsGrave", vec2(75, 77), vec2(5, 7), vec2(-37.5f, -38.5f));

	global.SetCurrentGameMap(present_worldMap);

	present_worldMap->AddGameObject(CreateDancingFlowers(vec2(-0.5f, -1.5f)));
	present_worldMap->AddGameObject(CreateDancingFlowers(vec2(-2.5f, 1.5f)));
	present_worldMap->AddGameObject(CreateWeed(vec2(-2.5f, -0.5f))); // Left Group Start
	present_worldMap->AddGameObject(CreateWeed(vec2(-3.5f, -0.5f)));
	present_worldMap->AddGameObject(CreateWeed(vec2(-3.5f, -1.5f)));
	present_worldMap->AddGameObject(CreateWeed(vec2(2.5f, -0.5f))); // Right Group Start
	present_worldMap->AddGameObject(CreateWeed(vec2(1.5f, -1.5f)));
	present_worldMap->AddGameObject(CreateWeed(vec2(2.5f, -1.5f)));
	present_worldMap->AddGameObject(CreateWeed(vec2(1.5f, -2.5f)));
	present_worldMap->AddGameObject(CreateWeed(vec2(2.5f, -2.5f)));
	present_worldMap->AddGameObject(CreateTree(vec2(-3.0f, 4.0f))); // Left Group Start
	present_worldMap->AddGameObject(CreateTree(vec2(-5.0f, 4.0f)));
	present_worldMap->AddGameObject(CreateTree(vec2(-5.0f, 2.0f)));
	present_worldMap->AddGameObject(CreateTree(vec2(-5.0f, 0.0f)));
	present_worldMap->AddGameObject(CreateTree(vec2(-5.0f, -2.0f)));
	present_worldMap->AddGameObject(CreateTree(vec2(4.0f, 4.0f))); // Right Group Start
	present_worldMap->AddGameObject(CreateTree(vec2(4.0f, 2.0f)));
	present_worldMap->AddGameObject(CreateTree(vec2(4.0f, 0.0f)));
	present_worldMap->AddGameObject(CreateTree(vec2(4.0f, -2.0f)));
	present_worldMap->AddGameObject(CreateBlocker(vec2(0.0f, -3.5f), vec2(10.0f, 1.0f))); // Bottom Wall



	present_worldMap->AddGameObject(CreateDancingFlowers(vec2(-0.5f, 6.5f)));
	present_worldMap->AddGameObject(CreateDancingFlowers(vec2(1.5f, 10.5f)));
	present_worldMap->AddGameObject(CreateWeed(vec2(-2.5f, 5.5f))); // Left Group Start
	present_worldMap->AddGameObject(CreateWeed(vec2(-2.5f, 6.5f)));
	present_worldMap->AddGameObject(CreateWeed(vec2(-2.5f, 7.5f)));
	present_worldMap->AddGameObject(CreateTree(vec2(-4.0f, 8.0f))); // Left Group Start
	present_worldMap->AddGameObject(CreateTree(vec2(-4.0f, 10.0f)));
	present_worldMap->AddGameObject(CreateTree(vec2(-3.0f, 12.0f)));
	present_worldMap->AddGameObject(CreateTree(vec2(-5.0f, 12.0f)));
	present_worldMap->AddGameObject(CreateTree(vec2(-4.0f, 8.0f)));
	present_worldMap->AddGameObject(CreateTree(vec2(4.0f, 6.0f))); // Right Group Start
	present_worldMap->AddGameObject(CreateTree(vec2(4.0f, 8.0f)));
	present_worldMap->AddGameObject(CreateSpookyTree(vec2(4.0f, 10.0f))); // Right Group Start
	present_worldMap->AddGameObject(CreateSpookyTree(vec2(4.0f, 12.0f)));

	global.heroGO = CreateHero(vec2(-0.5f, 3.0f)/*vec2(-0.5f, 0.5f)*/);
	present_worldMap->AddGameObject(global.heroGO);

	global.cameraGO = CreatePlayerCamera(vec2(0.0f, 0.0f));
	present_worldMap->AddGameObject(global.cameraGO);



	global.simulationSpace.SetDimensionRange(global.ScreenSize, global.XLScreenSize);
	global.loadSpace.SetDimensionRange(global.ScreenSize, global.loadSpace_SIZEMULTIPLIER * global.XLScreenSize);
	global.SetSimulationSpaceTarget(global.heroGO); // NOTE: This could probably be moved into the update loop and out of transition bars/ portals
	return true;
}





/*
 * GameUpdate and Utility
 */
vector<GameObject*> GenerateLoadBin(vector<GameObject*> &gameObjects)
{
	vector<GameObject*> result;

	for (size_t i = 0; i < gameObjects.size(); ++i)
	{
		GameObject* go = gameObjects[i];
		vec2 goPosition = go->transform.position;

		vec2 simulationSpacePosition = global.loadSpace.GetPosition();
		vec2 simulationSpaceHalfDimensions = global.loadSpace.GetHalfDimensions();
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

vector<GameObject*> GenerateUpdateBin(vector<GameObject*> &gameObjects)
{
	vector<GameObject*> result;

	for (size_t i = 0; i < gameObjects.size(); ++i)
	{
		GameObject* go = gameObjects[i];
		if (go->ForcesUpdates())
		{
			result.push_back(go);
			continue;
		}
		
		vec2 goPosition = go->transform.position;

		vec2 simulationSpacePosition = global.simulationSpace.GetPosition();
		vec2 simulationSpaceHalfDimensions = global.simulationSpace.GetHalfDimensions();
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

void AdvanceAnimations(vector<GameObject*> &gameObjects, F32 dt)
{
	for (size_t i = 0; i < gameObjects.size(); ++i)
	{
		GameObject* go = gameObjects[i];
		bool hasAnimations = go->animator != NULL && go->animator->NumberOfAnimations();
		if (hasAnimations)
		{
			go->animator->StepElapsedAnimationTime(dt);
		}
	}
}

void ReconcileCameras(vector<GameObject*> &gameObjects)
{
	for (size_t i = 0; i < gameObjects.size(); ++i)
	{
		GameObject* go = gameObjects[i];
		GameObjectType goType = go->GetType();
		if (goType == GameObjectType::PlayerCamera)
		{
			go->camera->position = go->transform.position;
			go->camera->rotationAngle = go->transform.rotationAngle;
		}
	}
}

void DrawCameraGrid()
{
	// NOTE: Camera space grid, corresponding to tiles
	Camera* renderCamera = GetRenderCamera();
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

void DrawGameObject(GameObject* gameObject)
{
	Camera* renderCamera = GetRenderCamera();

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

	/*if (gameObject->collisionShape != NULL && gameObject->debugDraw)
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
	}*/
}

void DrawGameObjects(vector<GameObject*>& gameObjects)
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
	for (size_t i = 0; i < gameObjects.size(); ++i)
	{
		GameObject* go = gameObjects[i];
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
		DrawGameObject(go);
		if (go->Debug != NULL)
		{
			go->Debug(go);
		}
	}


	DrawCameraGrid();
	


	for (size_t i = 0; i < bin_Environment.size(); ++i)
	{
		GameObject* go = bin_Environment[i];
		DrawGameObject(go);
		if (go->Debug != NULL)
		{
			go->Debug(go);
		}
	}

	for (size_t i = 0; i < bin_Characters.size(); ++i)
	{
		GameObject* go = bin_Characters[i];
		DrawGameObject(go);
		if (go->Debug != NULL)
		{
			go->Debug(go);
		}
	}

	for (size_t i = 0; i < bin_Effects.size(); ++i)
	{
		GameObject* go = bin_Effects[i];
		DrawGameObject(go);
		if (go->Debug != NULL)
		{
			go->Debug(go);
		}
	}

	// Draw Simulation Space Outline.
	vec2 ssDimensions = global.simulationSpace.GetDimensions();
	vec2 ssHalfDim = global.simulationSpace.GetHalfDimensions();
	vec2 ssPosition = global.simulationSpace.GetPosition();
	DebugDrawRectangleOutline(ssPosition + vec2(-ssHalfDim.x, ssHalfDim.y), ssDimensions, global.simulationSpaceOutlineColor);


	// Draw Load Space Outline
	vec2 lsDimensions = global.loadSpace.GetDimensions();
	vec2 lsHalfDim = global.loadSpace.GetHalfDimensions();
	vec2 lsPosition = global.loadSpace.GetPosition();
	DebugDrawRectangleOutline(lsPosition + vec2(-lsHalfDim.x, lsHalfDim.y), lsDimensions, global.loadSpaceOutlineColor);



	// NOTE: Draw UI
	/*DrawUVRectangleScreenSpace(&guiPanel, vec2(0, 0), vec2(guiPanel.width, guiPanel.height));*/
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

Rect_CD GetSimulationSpaceFromGameObject(GameObject* target)
{
	Rect_CD newSimulationSpace;

	// Change the simulation space to be centered around the new screen.
	vec2 rayOrigin = target->transform.position;
	vector<GameObject*> rayResults_xPos = global.GetCurrentGameMap()->collisionWorld.RaycastAll(rayOrigin, vec2(1.0f, 0.0f), GameObjectType::TransitionBar);
	vector<GameObject*> rayResults_xNeg = global.GetCurrentGameMap()->collisionWorld.RaycastAll(rayOrigin, vec2(-1.0f, 0.0f), GameObjectType::TransitionBar);
	vector<GameObject*> rayResults_yPos = global.GetCurrentGameMap()->collisionWorld.RaycastAll(rayOrigin, vec2(0.0f, 1.0f), GameObjectType::TransitionBar);
	vector<GameObject*> rayResults_yNeg = global.GetCurrentGameMap()->collisionWorld.RaycastAll(rayOrigin, vec2(0.0f, -1.0f), GameObjectType::TransitionBar);

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

	newSimulationSpace.center = simulationSpaceCenter;
	newSimulationSpace.dimensions = simulationSpaceDimensions;
	return newSimulationSpace;
}

void GameUpdate(F32 dt)
{
	HighResolutionTimer frameTimer = HighResolutionTimer("Frame Time");
	frameTimer.Start();

	// Handle really long frame times. Mainly for debugging/ moving the window around
	//	we should probably pause the game when the player moves the window around instead of just throwing the frame out 
	//  this is just a temp measure.
	if (dt > global.skipThreshold)
	{
		return;
	}

	// Handle freezing the game.
	if (global.WantsToBeFrozen())
	{
		global.StartFreeze();
	}
	if (global.GameFrozen())
	{
		global.StepGameFreeze(dt);
	}

 	Clear();

	vector<GameObject*> gameObjectsToLoad;
	vector<GameObject*> gameObjectsToUpdate;
	GameMap* currentMap = global.GetCurrentGameMap();

	if (global.simulationSpaceTarget != NULL)
	{
		CollisionInfo_2D ci = DetectCollision_2D(&global.simulationSpace.collisionRectangle, global.simulationSpace.transform, global.simulationSpaceTarget->collisionShape, global.simulationSpaceTarget->transform);
		if (!ci.collided)
		{
			Rect_CD newSimulationSpace = GetSimulationSpaceFromGameObject(global.simulationSpaceTarget);
			global.simulationSpace.Set(newSimulationSpace.center, newSimulationSpace.dimensions);
			global.loadSpace.Set(newSimulationSpace.center, newSimulationSpace.dimensions * global.loadSpace_SIZEMULTIPLIER);
			DebugPrintf(512, "SimulationSpace Updated\n");
		}
	}

	if (!global.GameFrozen())
	{
		gameObjectsToLoad = GenerateLoadBin(currentMap->gameObjects);
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


		gameObjectsToUpdate = GenerateUpdateBin(currentMap->gameObjects);
		UpdateGameObjects_PrePhysics(gameObjectsToUpdate, dt);


		HighResolutionTimer cwt = HighResolutionTimer("Collision World Things");
		cwt.Start();
		GameObject::IntegratePhysicsObjects(dt);
		currentMap->collisionWorld.FixupActives();
		vec2 simSpacePosition = global.simulationSpace.GetPosition();
		vec2 simSpaceHalfDimensions = global.simulationSpace.GetHalfDimensions();
		vector<PenetrationInfo_2D> interpenetrationsFixed = currentMap->collisionWorld.ResolveInterpenetrations(simSpacePosition, simSpaceHalfDimensions);
		vector<PenetrationInfo_2D> phantomInterpenetrations = currentMap->collisionWorld.GetPhantomInterpenetrations(simSpacePosition, simSpaceHalfDimensions);

		vector<Event> events_OnCollisionEnter;
		vector<Event> events_OnCollision;
		vector<Event> events_OnCollisionExit;

		GenerateCollisionEvents(events_OnCollisionEnter, events_OnCollision, events_OnCollisionExit, interpenetrationsFixed, global.interpenetrationsFixedLastFrame);
		GenerateCollisionEvents(events_OnCollisionEnter, events_OnCollision, events_OnCollisionExit, phantomInterpenetrations, global.phantomInterpenetrationsFromLastFrame);
		global.interpenetrationsFixedLastFrame = interpenetrationsFixed;
		global.phantomInterpenetrationsFromLastFrame = phantomInterpenetrations;
		DebugPrintf(512, "Interpenetrations Resolved = %u\n", interpenetrationsFixed.size());
		DebugPrintf(512, "Phantom Collisions = %u\n", phantomInterpenetrations.size());




		// Send Collision events.
		SendCollisionEvents(events_OnCollisionEnter);
		SendCollisionEvents(events_OnCollision);
		SendCollisionEvents(events_OnCollisionExit);
		cwt.End();
		cwt.Report();

		global.SendQueuedEvents();

		UpdateGameObjects_PostPhysics(gameObjectsToUpdate, dt);

		global.AreAllAnimationsPaused() ? NULL : AdvanceAnimations(currentMap->gameObjects, dt);

		ReconcileCameras(currentMap->gameObjects);
	}


	DrawGameObjects(currentMap->gameObjects);
	//DrawCollisionChunkRectangles(debug_collisionWorldChunks, vec4(1.0f, 1.0f, 1.0f, 1.0f));
	//DrawCollisionChunkRectangles(debug_cwgoAddChunks, vec4(1.0f, 0.5f, 0.0f, 1.0f));
	//glFinish();


	if (!global.GameFrozen())
	{
		global.AddThisFramesQueuedEventsToProcessingQueue();

		currentMap->ClearDestructionQueue(); // NOTE: Maybe this should be done before we draw?
	}


	frameTimer.End();
	frameTimer.Report();
	DebugPrintf(64, "\n\n\n");
}





/*
 * GameShutdown and Utility
 */
bool GameShutdown()
{
	global.spriteAssetFilepathTable.Destroy();
	ShutdownRenderer();
	return true;
}








/*
* Gameplay Debug API Implementation
*/
// NOTE: DebugDraw functions are shell functions that draw into the world without needing to supply the rendering camera.
//	the default rendering view is assumed
// For now use global.GetRenderCamera() ideally this should come from the renderer 
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
