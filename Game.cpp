#include "glew/GL/glew.h"
#include <gl/GL.h>

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
//#include "CollisionDetection3D_Applet.h"


//#define COLLISION3DAPPLET 1
#define COLLISION2DAPPLET 1



VertexData_Pos2D theGrid;
//VertexData_Pos2D_UV texturedQuad;
// VertexData_Pos2D	equalateralTriangle;
// VertexData_Pos2D	circleInPoint;
// VertexData_Pos2D	debugPoint;
// VertexData_Pos2D	debugLine;
// VertexData_Indexed_Pos2D_UV texturedQuad;


// SpriteShaderProgram2D texturedQuadProgram;
// BasicShaderProgram2D solidColorQuadProgram;
// BasicShaderProgram2D solidColorCircleInPointProgram;
// BasicShaderProgram2D solidColorTriangleProgram;



struct Entity
{
	Transform transform;
	Texture sprite;
};

struct GameCamera
{
	vec2 position;
	vec2 viewArea;
};

#define numGridLines 10
#define gridLinePointDimensionality 2
#define pointsPerLine 2


U32 numEntities = (10 * 9) + 1;
Entity* entities;
GameCamera  camera;
U32 linkEntityLocation = numEntities - 1;

void InitScene()
{
	camera.position = vec2(35, -12.5f);
	camera.viewArea.x = 10;
	camera.viewArea.y = 9;

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
	Initialize(&entities[linkEntityLocation].sprite, "Assets/x60/link.bmp");
}


bool GameInit()
{
	// NOTE: Handle turning on and off unit tests
	//CollisionTestsRectRect2();

	InitializeRenderer();

	glClearColor(0.32f, 0.18f, 0.66f, 0.0f);

	//const size_t tqNumVertices = 4;
// 	vec2 tqPositions[tqNumVertices] =
// 	{
// 		vec2(-1.0f, -1.0f),
// 		vec2( 1.0f, -1.0f),
// 		vec2( 1.0f,  1.0f),
// 		vec2(-1.0f,  1.0f)
// 	};
// 
// 	vec2 tqUVs[tqNumVertices] =
// 	{
// 		vec2(0.0f, 0.0f),
// 		vec2(1.0f, 0.0f),
// 		vec2(1.0f, 1.0f),
// 		vec2(0.0f, 1.0f)
// 	};
// 	Initialize(&texturedQuad, tqPositions, tqUVs, tqNumVertices);

// 	vec2 tqiPositions[tqNumVertices] =
// 	{
// 		vec2(-1.0f, -1.0f),
// 		vec2( 1.0f, -1.0f),
// 		vec2( 1.0f,  1.0f),
// 		vec2(-1.0f,  1.0f)
// 	};
// 
// 	vec2 tqiUVs[tqNumVertices] =
// 	{
// 		vec2(0.0f, 0.0f),
// 		vec2(1.0f, 0.0f),
// 		vec2(1.0f, 1.0f),
// 		vec2(0.0f, 1.0f)
// 	};
// 
// 	const size_t tqiNumIndices = 6;
// 	U32 tqiIndices[tqiNumIndices] = {0,1,2,   0,2,3};
// 	Initialize(&texturedQuad, tqiPositions, tqiUVs, tqNumVertices, tqiIndices, tqiNumIndices);

	// Grid
	const size_t numGridLinePositions = (numGridLines + 1) * gridLinePointDimensionality * pointsPerLine;
	vec2 gridLinePositions[numGridLinePositions] =
	{
		// varying y values
		vec2(-numGridLines / 2.0f, (-numGridLines / 2.0f) + 0.0f), vec2(numGridLines / 2.0f, (-numGridLines / 2.0f) + 0.0f),
		vec2(-numGridLines / 2.0f, (-numGridLines / 2.0f) + 1.0f), vec2(numGridLines / 2.0f, (-numGridLines / 2.0f) + 1.0f),
		vec2(-numGridLines / 2.0f, (-numGridLines / 2.0f) + 2.0f), vec2(numGridLines / 2.0f, (-numGridLines / 2.0f) + 2.0f),
		vec2(-numGridLines / 2.0f, (-numGridLines / 2.0f) + 3.0f), vec2(numGridLines / 2.0f, (-numGridLines / 2.0f) + 3.0f),
		vec2(-numGridLines / 2.0f, (-numGridLines / 2.0f) + 4.0f), vec2(numGridLines / 2.0f, (-numGridLines / 2.0f) + 4.0f),
		vec2(-numGridLines / 2.0f, (-numGridLines / 2.0f) + 5.0f), vec2(numGridLines / 2.0f, (-numGridLines / 2.0f) + 5.0f),
		vec2(-numGridLines / 2.0f, (-numGridLines / 2.0f) + 6.0f), vec2(numGridLines / 2.0f, (-numGridLines / 2.0f) + 6.0f),
		vec2(-numGridLines / 2.0f, (-numGridLines / 2.0f) + 7.0f), vec2(numGridLines / 2.0f, (-numGridLines / 2.0f) + 7.0f),
		vec2(-numGridLines / 2.0f, (-numGridLines / 2.0f) + 8.0f), vec2(numGridLines / 2.0f, (-numGridLines / 2.0f) + 8.0f),
		vec2(-numGridLines / 2.0f, (-numGridLines / 2.0f) + 9.0f), vec2(numGridLines / 2.0f, (-numGridLines / 2.0f) + 9.0f),
		vec2(-numGridLines / 2.0f, (-numGridLines / 2.0f) + 10.0f), vec2(numGridLines / 2.0f, (-numGridLines / 2.0f) + 10.0f),

		// varying x values
		vec2((-numGridLines / 2.0f) + 0.0f, -numGridLines / 2.0f), vec2((-numGridLines / 2.0f) + 0.0f, numGridLines / 2.0f),
		vec2((-numGridLines / 2.0f) + 1.0f, -numGridLines / 2.0f), vec2((-numGridLines / 2.0f) + 1.0f, numGridLines / 2.0f),
		vec2((-numGridLines / 2.0f) + 2.0f, -numGridLines / 2.0f), vec2((-numGridLines / 2.0f) + 2.0f, numGridLines / 2.0f),
		vec2((-numGridLines / 2.0f) + 3.0f, -numGridLines / 2.0f), vec2((-numGridLines / 2.0f) + 3.0f, numGridLines / 2.0f),
		vec2((-numGridLines / 2.0f) + 4.0f, -numGridLines / 2.0f), vec2((-numGridLines / 2.0f) + 4.0f, numGridLines / 2.0f),
		vec2((-numGridLines / 2.0f) + 5.0f, -numGridLines / 2.0f), vec2((-numGridLines / 2.0f) + 5.0f, numGridLines / 2.0f),
		vec2((-numGridLines / 2.0f) + 6.0f, -numGridLines / 2.0f), vec2((-numGridLines / 2.0f) + 6.0f, numGridLines / 2.0f),
		vec2((-numGridLines / 2.0f) + 7.0f, -numGridLines / 2.0f), vec2((-numGridLines / 2.0f) + 7.0f, numGridLines / 2.0f),
		vec2((-numGridLines / 2.0f) + 8.0f, -numGridLines / 2.0f), vec2((-numGridLines / 2.0f) + 8.0f, numGridLines / 2.0f),
		vec2((-numGridLines / 2.0f) + 9.0f, -numGridLines / 2.0f), vec2((-numGridLines / 2.0f) + 9.0f, numGridLines / 2.0f),
		vec2((-numGridLines / 2.0f) + 10.0f, -numGridLines / 2.0f), vec2((-numGridLines / 2.0f) + 10.0f, numGridLines / 2.0f),
	};
	Initialize(&theGrid, gridLinePositions, numGridLinePositions);

	/*// Equilateral triangle
	vec2 equalateralTrianglePositions[3] =
	{
		vec2(-0.6f, -0.3f),
		vec2(0.6f, -0.3f),
		vec2(0.0f, 0.6f)
	};
	Initialize(&equalateralTriangle, equalateralTrianglePositions, 3);

	// Circle
	vec2 circlePositions[1] =
	{
		vec2(0.0f, 0.0f)
	};
	Initialize(&circleInPoint, circlePositions, 1);

	vec2 debugPointPositions[1] = 
	{
		vec2(0.0f, 0.0f)
	};
	Initialize(&debugPoint, debugPointPositions, 1);

	vec2 debugLinePositions[2] = 
	{
		vec2(0.0f, 0.0f),
		vec2(1.0f, 1.0f)
	};
	Initialize(&debugLine, debugLinePositions, 2);*/

// 	Initialize(&texturedQuadProgram, "uvMapped2D.vert", "uvMapped2D.frag");
// 	Initialize(&solidColorQuadProgram,"singleColor2D.vert", "singleColor2D.frag");
// 	Initialize(&solidColorCircleInPointProgram, "solidColorCircleInPoint.vert", "solidColorCircleInPoint.frag");
// 	Initialize(&solidColorTriangleProgram, "singleColor3D.vert", "singleColor3D.frag");


	//InitScene();
#ifdef COLLISION3DAPPLET
	InitializeCollisionDetection3DApplet();
#elif COLLISION2DAPPLET
	InitializeCollisionDetection2DApplet();
#endif

	return true;
}








void GameUpdate(F32 deltaTime)
{
#ifdef COLLISION3DAPPLET
	UpdateCollisionDetection3DApplet(deltaTime);
#elif COLLISION2DAPPLET
	UpdateCollisionDetection2DApplet(deltaTime);
#endif
}

bool GameShutdown()
{
	ShutdownRenderer();
// 	Destroy(&texturedQuadProgram);
// 	Destroy(&solidColorQuadProgram);
// 	Destroy(&solidColorCircleInPointProgram);
// 	Destroy(&solidColorTriangleProgram);

	return true;
}