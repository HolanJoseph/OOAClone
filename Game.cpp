#include "Types.h"
#include "Math.h"
#include "BitManip.h"
#include "Util.h"
#include "RandomNumberGenerator.h"

#include "GameAPI.h"
#include "DebugAPI.h"
#include "InputAPI.h"
#include "FileAPI.h"
#include "StringAPI.h"

#include "Renderer.h"

#include "AssetLoading.h"
#include "CollisionDetection.h"
#include "CollisionDetection2D.h"

#include "CollisionDetection_Tests.h"

#include "CollisionDetection_Visualization.h"

#include "CollisionDetection2D_Applet.h"

//#define RUN_UNIT_TESTS 1

//#define COLLISION2DAPPLET 1
#define GAME 1

Camera camera;

Transform weedTransform;
Texture weed;

Texture xthing;
Texture sysbar;


void InitScene()
{
	SetWindowTitle("Oracle of Ages Clone");
	SetWindowDimensions(vec2(600, 540));
	SetViewport({ vec2(0, 0), vec2(600, 540) });
	SetClearColor(vec4(0.32f, 0.18f, 0.66f, 0.0f));

	Initialize(&weed, "Assets/x60/weed.bmp");
	weedTransform = Transform();
	weedTransform.scale = vec2(.5, .5);

	camera.halfDim = vec2(5, 4);
	camera.position = vec2(0, 0);
	camera.rotationAngle = 0.0f;
	camera.scale = 1.0f;

	Initialize(&sysbar, "Assets/x60/sysBar.bmp");

	Initialize(&xthing, "Assets/xthing.bmp");

	/*camera.position = vec2(35, -12.5f);
	camera.halfDim = vec2(5, 4.5);

	entities = (Entity*)malloc(sizeof(Entity) * numEntities);

	char* tileFilenames[] = {
		// System Bar
		"Assets/x60/sysBar.bmp",
		"Assets/x60/sysBar.bmp",
		"Assets/x60/sysBar.bmp",
		"Assets/x60/sysBar.bmp",
		"Assets/x60/sysBar.bmp",
		"Assets/x60/sysBar.bmp",
		"Assets/x60/sysBar.bmp",
		"Assets/x60/sysBar.bmp",
		"Assets/x60/sysBar.bmp",
		"Assets/x60/sysBar.bmp",

		// row 1
		"Assets/x60/treeLowRight.bmp",
		"Assets/x60/treeLowLeft.bmp",
		"Assets/x60/treeLowRight.bmp",
		"Assets/x60/yellow.bmp",
		"Assets/x60/yellow.bmp",
		"Assets/x60/greenFringeBL.bmp",
		"Assets/x60/green.bmp",
		"Assets/x60/greenFringeR.bmp",
		"Assets/x60/treeLowLeft.bmp",
		"Assets/x60/treeLowRight.bmp",

		// row 2
		"Assets/x60/treeHighRight.bmp",
		"Assets/x60/greenFringeUL.bmp",
		"Assets/x60/greenFringeU.bmp",
		"Assets/x60/greenFringeUR.bmp",
		"Assets/x60/yellow.bmp",
		"Assets/x60/yellow.bmp",
		"Assets/x60/greenFringeL.bmp",
		"Assets/x60/greenFringeR.bmp",
		"Assets/x60/treeHighLeft.bmp",
		"Assets/x60/treeHighRight.bmp",

		// row 3
		"Assets/x60/treeLowRight.bmp",
		"Assets/x60/greenFringeL.bmp",
		"Assets/x60/greenFlowers.bmp",
		"Assets/x60/greenFringeR.bmp",
		"Assets/x60/yellow.bmp",
		"Assets/x60/greenFringeUL.bmp",
		"Assets/x60/greenWeeds.bmp",
		"Assets/x60/greenFringeBR.bmp",
		"Assets/x60/treeLowLeft.bmp",
		"Assets/x60/treeLowRight.bmp",
		
		// row 4
		"Assets/x60/treeHighRight.bmp",
		"Assets/x60/greenFringeBL.bmp",
		"Assets/x60/greenFringeB.bmp",
		"Assets/x60/greenWeeds.bmp",
		"Assets/x60/greenFringeU.bmp",
		"Assets/x60/green.bmp",
		"Assets/x60/greenFringeBR.bmp",
		"Assets/x60/yellow.bmp",
		"Assets/x60/treeHighLeft.bmp",
		"Assets/x60/treeHighRight.bmp",
		
		// row 5
		"Assets/x60/treeLowRight.bmp",
		"Assets/x60/weed.bmp",
		"Assets/x60/weed.bmp",
		"Assets/x60/greenFringeL.bmp",
		"Assets/x60/green.bmp",
		"Assets/x60/greenFringeR.bmp",
		"Assets/x60/yellow.bmp",
		"Assets/x60/weed.bmp",
		"Assets/x60/treeLowLeft.bmp",
		"Assets/x60/treeLowRight.bmp",

		// row 6
		"Assets/x60/treeHighRight.bmp",
		"Assets/x60/weed.bmp",
		"Assets/x60/greenFringeUL.bmp",
		"Assets/x60/green.bmp",
		"Assets/x60/greenFlowers.bmp",
		"Assets/x60/greenFringeR.bmp",
		"Assets/x60/weed.bmp",
		"Assets/x60/weed.bmp",
		"Assets/x60/treeHighLeft.bmp",
		"Assets/x60/treeHighRight.bmp",

		// row 7
		"Assets/x60/treeLowRight.bmp",
		"Assets/x60/yellow.bmp",
		"Assets/x60/greenFringeBL.bmp",
		"Assets/x60/greenFringeB.bmp",
		"Assets/x60/greenFringeB.bmp",
		"Assets/x60/greenFringeBR.bmp",
		"Assets/x60/weed.bmp",
		"Assets/x60/weed.bmp",
		"Assets/x60/treeLowLeft.bmp",
		"Assets/x60/treeLowRight.bmp",

		// row 8
		"Assets/x60/rails.bmp",
		"Assets/x60/rails.bmp",
		"Assets/x60/rails.bmp",
		"Assets/x60/rails.bmp",
		"Assets/x60/rails.bmp",
		"Assets/x60/rails.bmp",
		"Assets/x60/rails.bmp",
		"Assets/x60/rails.bmp",
		"Assets/x60/rails.bmp",
		"Assets/x60/rails.bmp",
	};

	for (U32 j = 0; j < 9; ++j)
	{
		for (U32 i = 0; i < 10; ++i)
		{
			U32 entityLocation = 10 * j + i;

			vec2 pos(30.5f + i, -8.5f - j);
			char* filename = tileFilenames[entityLocation];

			entities[entityLocation].transform.position = pos;
			entities[entityLocation].transform.scale = 1.0f;

			Initialize(&entities[entityLocation].sprite, filename);
		}
	}

	
	entities[linkEntityLocation].transform.position = camera.position;
	entities[linkEntityLocation].transform.scale = .75f;
	Initialize(&entities[linkEntityLocation].sprite, "Assets/x60/link.bmp");*/
}


bool GameInit()
{
	// Initialize game sub systems.
	InitializeRenderer();



#ifdef RUN_UNIT_TESTS
	CollisionTestsRectRect2();
#endif
	

	// Dispatch applet type.
#ifdef COLLISION2DAPPLET
	InitializeCollisionDetection2DApplet();
#elif GAME
	InitScene();
#endif

	return true;
}




void UpdateGamestate_PrePhysics(F32 dt)
{
}

void UpdateGamestate_PostPhysics(F32 dt)
{
	SetViewport({ vec2(0, 0), vec2(600, 480) });
	for (F32 i = -camera.halfDim.x + .5; i < camera.halfDim.x; ++i)
	{
		for (F32 j = -camera.halfDim.y + .5; j < camera.halfDim.y; ++j)
		{
			weedTransform.position = vec2(i, j);
			DrawUVRectangle(&weed, weedTransform, &camera);
		}
	}

	vec2 winDim = GetWindowDimensions();
	for (size_t i = 0; i < winDim.x; i += sysbar.width)
	{
		DrawUVRectangleScreenSpace(&sysbar, vec2(i, 0), vec2(sysbar.width, sysbar.height));
	}
	
	DrawUVRectangleScreenSpace(&xthing, vec2(100, 100), vec2(xthing.width, xthing.height));
}

void GameUpdate(F32 deltaTime)
{
#ifdef COLLISION2DAPPLET
	UpdateCollisionDetection2DApplet(deltaTime);
#elif GAME
	Clear();
	UpdateGamestate_PrePhysics(deltaTime);
	// something something physics and collision detection
	UpdateGamestate_PostPhysics(deltaTime);
#endif
}

bool GameShutdown()
{
#ifdef COLLISION2DAPPLET
	ShutdownCollisionDetection2DApplet();
#endif

	ShutdownRenderer();
	return true;
}