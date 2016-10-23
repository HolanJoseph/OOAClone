#pragma once

#include "Types.h"
#include "Math.h"

#include "_GameplayAPI.h"
#include "GameObject.h"

#include <vector>
using std::vector;



GameObject* CreateFire(vec2 position, bool debugDraw);





inline void Update_PrePhysics_Hero(GameObject* go, F32 dt)
{
	if (go->frozen)
	{
		return;
	}
	// NOTE: This method seems to be more stable WHY???
	// NOTE: If pressing A and D and then pressing W or S we move up/ down with 1.5 speed
	//			this is because both of the diagonal forces are being applied
	//bool getKey_A = GetKey(KeyCode_A);
	//bool getKey_D = GetKey(KeyCode_D);
	//bool getKey_W = GetKey(KeyCode_W);
	//bool getKey_S = GetKey(KeyCode_S);
	//bool getKeys_WD = getKey_W && getKey_D;
	//bool getKeys_SD = getKey_S && getKey_D;
	//bool getKeys_WA = getKey_W && getKey_A;
	//bool getKeys_SA = getKey_S && getKey_A;
	//
	//if (getKeys_WD)
	//{
	//	this->rigidbody->ApplyImpulse(normalize(vec2(1.0f, 1.0f)), 1.5f);
	//}
	//if (getKeys_SD)
	//{
	//	this->rigidbody->ApplyImpulse(normalize(vec2(1.0f, -1.0f)), 1.5f);
	//}
	//if (getKeys_WA)
	//{
	//	this->rigidbody->ApplyImpulse(normalize(vec2(-1.0f, 1.0f)), 1.5f);
	//}
	//if (getKeys_SA)
	//{
	//	this->rigidbody->ApplyImpulse(normalize(vec2(-1.0f, -1.0f)), 1.5f);
	//}
	//if (!getKeys_WA && !getKeys_SA && getKey_A)
	//{
	//	this->rigidbody->ApplyImpulse(vec2(-1.0f, 0.0f), 1.5f);
	//}
	//if (!getKeys_WD && !getKeys_SD && getKey_D)
	//{
	//	this->rigidbody->ApplyImpulse(vec2(1.0f, 0.0f), 1.5f);
	//}
	//if (!getKeys_WD && !getKeys_WA && getKey_W)
	//{
	//	this->rigidbody->ApplyImpulse(vec2(0.0f, 1.0f), 1.5f);
	//}
	//if (!getKeys_SD && !getKeys_SA && getKey_S)
	//{
	//	this->rigidbody->ApplyImpulse(vec2(0.0f, -1.0f), 1.5f);
	//}
	DebugPrintf(512, "HELLO\n");

	vec2 forceDirection = vec2(0.0f, 0.0f);
	bool getKey_A = GetKey(KeyCode_A);
	bool getKey_D = GetKey(KeyCode_D);
	bool getKey_W = GetKey(KeyCode_W);
	bool getKey_S = GetKey(KeyCode_S);
	DebugPrintf(512, "Keys: %s %s %s %s\n", (getKey_W ? "W" : ""), (getKey_A ? "A" : ""), (getKey_S ? "S" : ""), (getKey_D ? "D" : ""));
	if (getKey_W)
	{
		forceDirection += vec2(0.0f, 1.0f);
	}
	if (getKey_S)
	{
		forceDirection += vec2(0.0f, -1.0f);
	}
	if (getKey_D)
	{
		forceDirection += vec2(1.0f, 0.0f);
	}
	if (getKey_A)
	{
		forceDirection += vec2(-1.0f, 0.0f);
	}
	DebugPrintf(512, "force direction = (%f, %f)\n", forceDirection.x, forceDirection.y);
	if (forceDirection != vec2(0.0f, 0.0f))
	{
		forceDirection = normalize(forceDirection);
		F32 movementSpeed = go->movementSpeed;
		if (go->isSlowed)
		{
			movementSpeed *= go->slowPercentage;
		}
		go->rigidbody->ApplyImpulse(forceDirection, movementSpeed);
	}

	DebugPrintf(512, "Velocity = (%f, %f)\n", go->rigidbody->velocity.x, go->rigidbody->velocity.y);
	if (go->rigidbody->velocity == vec2(0.0f, 0.0f))
	{
		go->moving = false;
		go->animator->StopAnimation();
	}
	else
	{
		go->moving = true;
		//this->animator->StartAnimation();
	}

	vec2 velocityDirection = go->moving ? normalize(go->rigidbody->velocity) : vec2(0.0f, 0.0f);
	F32 facingDOTvelocityDirection = dot(go->facing, velocityDirection);
	vec2 newFacing = go->facing;
	if (facingDOTvelocityDirection > 0.0f || !go->moving)
	{
		// Keep the current facing direction.
	}
	else
	{
		vec2 rotatedFacing = vec2(RotationMatrix_2D(90.0f) * vec3(go->facing.x, go->facing.y, 0.0f));
		rotatedFacing.x = round(rotatedFacing.x);
		rotatedFacing.y = round(rotatedFacing.y);
		F32 rotatedFacingDOTvelocityDirection = dot(rotatedFacing, velocityDirection);

		if (rotatedFacingDOTvelocityDirection > 0.0f)
		{
			newFacing = rotatedFacing;
		}
		else if (rotatedFacingDOTvelocityDirection == 0.0f)
		{
			newFacing = -go->facing;
		}
		else
		{
			newFacing = -rotatedFacing;
		}
	}

	// NOTE: The next 21 lines are the the implementation that works for successfully starting the correct animation for facing directions
	//if (this->moving && (newFacing != this->facing))
	//{
	//	if (newFacing == vec2(1.0f, 0.0f))
	//	{
	//		this->animator->StartAnimation("right");
	//	}
	//	else if (newFacing == vec2(-1.0f, 0.0f))
	//	{
	//		this->animator->StartAnimation("left");
	//	}
	//	else if (newFacing == vec2(0.0f, 1.0f))
	//	{
	//		this->animator->StartAnimation("up");
	//	}
	//	else if (newFacing == vec2(0.0f, -1.0f))
	//	{
	//		this->animator->StartAnimation("down");
	//	}
	//}
	//this->facing = newFacing;
	//this->pushingForward = false;
	go->facing = newFacing;
	go->pushingForward = false;
}

inline void Update_PostPhysics_Hero(GameObject* go, F32 dt)
{
	if (go->frozen)
	{
		return;
	}

	if (go->moving)
	{
		if (go->pushingForward)
		{
			if (go->facing == vec2(1.0f, 0.0f))
			{
				go->animator->StartAnimation("pushRight", false);
			}
			else if (go->facing == vec2(-1.0f, 0.0f))
			{
				go->animator->StartAnimation("pushLeft", false);
			}
			else if (go->facing == vec2(0.0f, 1.0f))
			{
				go->animator->StartAnimation("pushUp", false);
			}
			else if (go->facing == vec2(0.0f, -1.0f))
			{
				go->animator->StartAnimation("pushDown", false);
			}
		}
		else
		{
			if (go->facing == vec2(1.0f, 0.0f))
			{
				go->animator->StartAnimation("right", false);
			}
			else if (go->facing == vec2(-1.0f, 0.0f))
			{
				go->animator->StartAnimation("left", false);
			}
			else if (go->facing == vec2(0.0f, 1.0f))
			{
				go->animator->StartAnimation("up", false);
			}
			else if (go->facing == vec2(0.0f, -1.0f))
			{
				go->animator->StartAnimation("down", false);
			}
		}
	}
	else
	{
		if (go->facing == vec2(1.0f, 0.0f))
		{
			go->animator->StartAnimation("right", false);
		}
		else if (go->facing == vec2(-1.0f, 0.0f))
		{
			go->animator->StartAnimation("left", false);
		}
		else if (go->facing == vec2(0.0f, 1.0f))
		{
			go->animator->StartAnimation("up", false);
		}
		else if (go->facing == vec2(0.0f, -1.0f))
		{
			go->animator->StartAnimation("down", false);
		}
		go->animator->StopAnimation();
	}
	//DebugPrintf(512, "Player Facing: (%f, %f)\n", this->facing.x, this->facing.y);
	//DebugPrintf(512, "Pushing forward: %s\n", (this->pushingForward ? "true" : "false"));

	//vector<GameObject*> infront = RaycastAll_Line_2D(this->transform.position, this->facing, 1000.0f);
	//for (size_t i = 0; i < infront.size(); ++i)
	//{
	//	GameObject* go = infront[i];
	//	go->SetDebugState(true);
	//}
	GameObject* infront = RaycastFirst_Line_2D(go->transform.position, go->facing, 1000.0f);
	if (infront != NULL && go->showRay)
	{
		infront->SetDebugState(true);
	}

	if (GetKeyDown(KeyCode_Spacebar))
	{
		CreateFire(go->transform.position + go->facing, true);
	}
}

inline void DoEvent_Hero(GameObject* go, Event* e)
{
	switch (e->GetType())
	{

	case ET_OnCollision:
	{
						   GameObject* collidedWith = (GameObject*)e->arguments[1].AsPointer();
						   vec2 collisionNormal = e->arguments[2].AsVec2();

						   //DebugPrintf(512, "Collision Normal = (%f, %f)\n", collisionNormal.x, collisionNormal.y);
						   //DebugPrintf(512, "Player Facing = (%f, %f)\n", this->facing.x, this->facing.y);

						   if (!collidedWith->collisionShape->IsPhantom() && collisionNormal == go->facing)
						   {
							   go->pushingForward = true;
						   }
	}
	break;

	case ET_Freeze:
	{
					  go->frozen = true;
					  go->animator->PauseAnimation();
	}
	break;

	case ET_Unfreeze:
	{
						go->frozen = false;
						go->animator->StartAnimation();
	}
	break;

	default:
	break;
	}
}

inline GameObject* CreateHero(vec2 position = vec2(0.0f, 0.0f), bool debugDraw = true)
{
	GameObject* hero = CreateGameObject(GameObjectType::PlayerCharacter);

	hero->transform.position = position;
	hero->AddTag(GameObjectTags::Hero);
	hero->AddAnimator();
	F32 movementAnimationsSpeed = 0.2f; //0.2475f; // 0.33f; // NOTE: This is NOT a measured speed
	hero->animator->AddAnimation("up", "link_Up", 2, movementAnimationsSpeed);
	hero->animator->AddAnimation("down", "link_Down", 2, movementAnimationsSpeed);
	hero->animator->AddAnimation("right", "link_Right", 2, movementAnimationsSpeed);
	hero->animator->AddAnimation("left", "link_Left", 2, movementAnimationsSpeed);
	hero->animator->AddAnimation("pushUp", "link_PushUp", 2, movementAnimationsSpeed);
	hero->animator->AddAnimation("pushDown", "link_PushDown", 2, movementAnimationsSpeed);
	hero->animator->AddAnimation("pushRight", "link_PushRight", 2, movementAnimationsSpeed);
	hero->animator->AddAnimation("pushLeft", "link_PushLeft", 2, movementAnimationsSpeed);
	hero->animator->StartAnimation("down");
	hero->animator->PauseAnimation();
	hero->AddRigidbody(1.0f, 0.0f, vec2(0.0f, 0.0f), vec2(0.0f, 0.0f));
	// NOTE: hero position as bottom of feet
	//hero->animator->SetSpriteOffset(vec2(0.0f, 0.4375f)); 
	//hero->AddCollisionShape(Rectangle_2D(vec2(TileDimensions.x * 0.5f, TileDimensions.y * 0.5625f), vec2(0.0f, 4.5f/16.0f)));
	hero->AddCollisionShape(Rectangle_2D(vec2(TileDimensions.x * 0.5f, TileDimensions.y * 0.5625f), vec2(0.0f, -2.5f / 16.0f)));

	// State
	hero->facing = vec2(0.0f, -1.0f);
	hero->moving = false;
	hero->movementSpeed = 2.5f;
	hero->slowPercentage = 0.5f;
	hero->isSlowed = false;
	hero->pushingForward = false;
	hero->showRay = debugDraw;
	hero->frozen = false;

	hero->Update_Pre = Update_PrePhysics_Hero;
	hero->Update_Post = Update_PostPhysics_Hero;
	hero->DoEvent = DoEvent_Hero;

	hero->SetDebugState(debugDraw);

	return hero;
}






inline void Update_PrePhysics_PlayerCamera(GameObject* go, F32 dt)
{
	/*
	* Camera Controls
	*
	* - zoom camera out  2x
	* + zoom camera in 2x
	*
	* WASD Move camera to the next screen
	*/
	if (GetKeyDown(KeyCode_Spacebar))
	{
		go->bound = !(go->bound);
	}
	if (GetKeyDown(KeyCode_Minus))
	{
		go->camera->halfDim *= 2.0f;
	}
	if (GetKeyDown(KeyCode_Equal))
	{
		go->camera->halfDim /= 2.0f;
	}
	if (GetKeyDown(KeyCode_Up))
	{
		go->transform.position.y += 8.0f;
	}
	if (GetKeyDown(KeyCode_Down))
	{
		go->transform.position.y -= 8.0f;
	}
	if (GetKeyDown(KeyCode_Right))
	{
		go->transform.position.x += 10.0f;
	}
	if (GetKeyDown(KeyCode_Left))
	{
		go->transform.position.x -= 10.0f;
	}

	if (go->bound)
	{
		go->transform.position = go->target->transform.position;

		vector<vec2> directions = { vec2(1.0f, 0.0f), vec2(-1.0f, 0.0f), vec2(0.0f, 1.0f), vec2(0.0f, -1.0f) };
		vector<F32>  closenesses = { 5.0f, 5.0f, 4.0f, 4.0f };
		for (size_t i = 0; i < directions.size(); ++i)
		{
			vec2 direction = directions[i];
			F32  closeness = closenesses[i];
			//HighResolutionTimer cameraRayTimer = HighResolutionTimer("Camera Ray", 1);
			//cameraRayTimer.Start();
			vector<GameObject*> gameObjectsInDirection = RaycastAll_Line_2D(go->transform.position, direction, 20.0f); // NOTE: arbitrary number
			//cameraRayTimer.End();
			//cameraRayTimer.Report();
			GameObject* closestTransitionBarInDirection = NULL;
			F32 distanceToTransitionBar = 10000.0f;
			for (size_t k = 0; k < gameObjectsInDirection.size(); ++k)
			{
				GameObject* g = gameObjectsInDirection[k];
				GameObjectType gType = g->GetType();
				if (gType == GameObjectType::TransitionBar)
				{
					vec2 camera1D = VVM(go->transform.position, direction);
					vec2 transitionBar1D = VVM(g->transform.position, direction);
					F32 distanceBetween = length(camera1D - transitionBar1D);
					if (distanceBetween < distanceToTransitionBar)
					{
						closestTransitionBarInDirection = g;
						distanceToTransitionBar = distanceBetween;
					}
				}
			}

			if (distanceToTransitionBar < closeness)
			{
				F32 difference = closeness - distanceToTransitionBar;
				vec2 deltaMovement = direction * difference;
				go->transform.position -= deltaMovement;
			}
		}
	}
}

inline GameObject* CreatePlayerCamera(vec2 position, bool debugDraw = true)
{
	GameObject* camera = CreateGameObject(GameObjectType::PlayerCamera);

	camera->ForceUpdates(true);
	camera->transform.position = position;
	camera->AddCamera(ScreenDimensions);
	camera->SetDebugState(debugDraw);

	//vector<GameObject*> inRaycast = RaycastAll_Rectangle_2D(camera->transform.position, ScreenDimensions, 0.0f);
	//GameObject* closestTetherPoint = NULL;
	//F32 closestTetherPoint_Distance = 10000.0f;
	//for (size_t i = 0; i < inRaycast.size(); ++i)
	//{
	//	GameObject* go = inRaycast[i];
	//	GameObject::Type goType = go->GetType();
	//	if (goType == GameObject::CameraTetherPoint)
	//	{
	//		vec2 toTether = go->transform.position - camera->transform.position;
	//		F32 distanceToTether = length(toTether);
	//		if (distanceToTether < closestTetherPoint_Distance)
	//		{
	//			closestTetherPoint_Distance = distanceToTether;
	//			closestTetherPoint = go;
	//		}
	//	}
	//}
	//camera->tetherPoint = closestTetherPoint;
	camera->bound = true;
	vector<GameObject*> playerCharacters = FindGameObjectByType(GameObjectType::PlayerCharacter);
	if (playerCharacters.size() > 0)
	{
		camera->target = playerCharacters[0];
	}

	camera->Update_Pre = Update_PrePhysics_PlayerCamera;

	return camera;
}






inline void Update_PostPhysics_Fire(GameObject* go, F32 dt)
{
	go->lifetime += dt;
	if (go->lifetime >= 3.0f)
	{
		// Do a rectangle cast the size of our collision shape
		// Destroy every burnable GameObject returned by the cast.
		vector<GameObject*> inMe = RaycastAll_Rectangle_2D(go->transform.position, ((Rectangle_2D*)go->collisionShape)->halfDim, go->transform.rotationAngle);
		for (size_t i = 0; i < inMe.size(); ++i)
		{
			GameObject* g = inMe[i];
			if (g->HasTag(Burnable))
			{
				DestroyGameObject(g);
			}
		}

		DestroyGameObject(go);
	}
}

inline GameObject* CreateFire(vec2 position, bool debugDraw = true)
{
	GameObject* fire = CreateGameObject(GameObjectType::Fire);
	fire->AddTag(GameObjectTags::Effect);
	fire->transform.position = position;
	fire->AddAnimator();
	fire->animator->AddAnimation("fire", "fire_Effect", 3, 0.30f);
	fire->animator->StartAnimation("fire");
	fire->AddCollisionShape(Rectangle_2D(TileDimensions, vec2(0.0f, 0.0f), true));
	fire->SetDebugState(debugDraw);

	fire->lifetime = 0.0f;

	fire->Update_Post = Update_PostPhysics_Fire;

	return fire;
}






inline void DoEvent_TransitionBar(GameObject* transitionBar, Event* e)
{
	switch (e->GetType())
	{
						  case ET_OnCollisionEnter:
						  {
													  GameObject* go = (GameObject*)e->arguments[1].AsPointer();
													  vec2 collisionNormal = e->arguments[2].AsVec2();

													  if (go->GetType() == PlayerCharacter)
													  {
														  vector<GameObject*> playerCameras = FindGameObjectByType(PlayerCamera);
														  GameObject* playerCamera = playerCameras[0];
														  vec2 endPoint = transitionBar->transform.position + VVM(TileDimensions, -collisionNormal);

														  vec2 playerEndPos = go->transform.position;
														  vec2 cameraEndPos = playerCamera->transform.position;

														  vector<GameObject*> inRaycast;
														  vector<GameObject*> cameraTethers;

														  if (collisionNormal.x != 0.0f)
														  {
															  playerEndPos.x = endPoint.x;

															  if (collisionNormal.x > 0)
															  {
																  cameraEndPos.x -= ScreenDimensions.x * 2.0f;

															  }
															  else
															  {
																  cameraEndPos.x += ScreenDimensions.x * 2.0f;
															  }
														  }
														  if (collisionNormal.y != 0.0f)
														  {
															  playerEndPos.y = endPoint.y;

															  if (collisionNormal.y > 0)
															  {
																  cameraEndPos.y -= ScreenDimensions.y * 2.0f;
															  }
															  else
															  {
																  cameraEndPos.y += ScreenDimensions.y * 2.0f;
															  }
														  }
														  // 												   inRaycast = RaycastAll_Line_2D(playerCamera->tetherPoint->transform.position, -collisionNormal, 20.0f);
														  // 
														  // 												   for (size_t i = 0; i < inRaycast.size(); ++i)
														  // 												   {
														  // 													   GameObject* go = inRaycast[i];
														  // 													   GameObject::Type goType = go->GetType();
														  // 													   if (goType == CameraTetherPoint)
														  // 													   {
														  // 														   cameraTethers.push_back(go);
														  // 													   }
														  // 												   }
														  //
														  // 												   GameObject* newCameraTether = NULL;
														  // 												   F32 shortestTetherDistance = 10000.0f;
														  // 												   for (size_t i = 0; i < cameraTethers.size(); i++)
														  // 												   {
														  // 													   GameObject* go = cameraTethers[i];
														  // 													   vec2 toTether = go->transform.position - playerCamera->transform.position;
														  // 													   F32 distanceToTether = length(toTether);
														  // 													   if (distanceToTether <= shortestTetherDistance && distanceToTether > 0.0f)
														  // 													   {
														  // 														   shortestTetherDistance = distanceToTether;
														  // 														   newCameraTether = go;
														  // 													   }
														  // 												   }
														  // 												   GameObject* oldCameraTether = playerCamera->tetherPoint;
														  // 												   playerCamera->tetherPoint = NULL;
														  playerCamera->bound = false;

														  U32 freezeLength = 250;
														  FreezeGame(freezeLength);
														  PauseAllAnimations();

														  // Queue an event to handle the transition.
														  Event e;
														  e.SetType(ET_Transition);
														  e.arguments[0] = EventArgument(GetTimeSinceStartup() + freezeLength);
														  e.arguments[1] = EventArgument(750);
														  e.arguments[2] = EventArgument(go->transform.position);
														  e.arguments[3] = EventArgument(playerEndPos);
														  e.arguments[4] = EventArgument(playerCamera->transform.position);
														  e.arguments[5] = EventArgument(cameraEndPos);
														  e.arguments[6] = EventArgument((void*)go);
														  e.arguments[7] = EventArgument((void*)(playerCamera));
														  e.arguments[8] = EventArgument((void*)NULL/*(newCameraTether)*/);
														  QueueEvent(transitionBar, &e, 2); // NOTE: Watch to see if this causes jumps again.

														  // Queue an even to freeze the player
														  Event freezeE;
														  freezeE.SetType(ET_Freeze);
														  QueueEvent(go, &freezeE, 2);
													  }
						  }
						  break;

						  case ET_Transition:
						  {
												//
												SystemTime transitionStartTime = e->arguments[0].AsSystemTime();
												U32 transitionLength = e->arguments[1].AsU32();
												vec2 playerStartPos = e->arguments[2].AsVec2();
												vec2 playerEndPos = e->arguments[3].AsVec2();
												vec2 cameraStartPos = e->arguments[4].AsVec2();
												vec2 cameraEndPos = e->arguments[5].AsVec2();
												GameObject* player = (GameObject*)e->arguments[6].AsPointer();
												GameObject* camera = (GameObject*)e->arguments[7].AsPointer();
												GameObject* newCameraTether = (GameObject*)e->arguments[8].AsPointer();
												SystemTime currentTime = GetTimeSinceStartup();
												SystemTime diffTime = currentTime - transitionStartTime;
												F32 percentageTime = diffTime / transitionLength;

												vec2 newPlayerPos = LerpClamped(playerStartPos, playerEndPos, percentageTime);
												vec2 newCameraPos = LerpClamped(cameraStartPos, cameraEndPos, percentageTime);

												player->transform.position = newPlayerPos;
												camera->transform.position = newCameraPos;

												if (percentageTime < 1.0f)
												{
													// Send this event again.
													QueueEvent(transitionBar, e, 1);
												}
												else
												{
													// Queue an event to the player to unfreeze
													Event unfreezeE;
													unfreezeE.SetType(ET_Unfreeze);
													QueueEvent(player, &unfreezeE, 1);

													//camera->tetherPoint = newCameraTether;
													camera->bound = true;

													UnpauseAllAnimations(); // Note: This should be done on the same frame as the unfreeze.

													SetSimulationSpace(player);
												}
						  }
						  break;

						  default:
						  break;
}
}

inline GameObject* CreateHorizontalTransitionBar(vec2 position, F32 length = 10.0f, bool debugDraw = true)
{
	GameObject* ctb = CreateGameObject(GameObjectType::TransitionBar);

	ctb->transform.position = position;
	ctb->transform.scale = vec2(1.0f, 1.0f); // vec2(10.0f, 0.5f);
	ctb->AddTag(GameObjectTags::Environment);
	ctb->AddCollisionShape(Rectangle_2D(vec2(length / 2.0f, 0.125f), vec2(0.0f, -0.125f), true));
	ctb->SetDebugState(debugDraw);

	ctb->DoEvent = DoEvent_TransitionBar;

	return ctb;
}

inline GameObject* CreateVerticalTransitionBar(vec2 position, F32 length = 8.0f, bool debugDraw = true)
{
	GameObject* ctb = CreateGameObject(GameObjectType::TransitionBar);

	ctb->transform.position = position;
	ctb->transform.scale = vec2(1.0f, 1.0f); // vec2(10.0f, 0.5f);
	ctb->AddTag(GameObjectTags::Environment);
	ctb->AddCollisionShape(Rectangle_2D(vec2(0.125f, length / 2.0f), vec2(0.0f, 0.0f), true));
	ctb->SetDebugState(debugDraw);

	ctb->DoEvent = DoEvent_TransitionBar;

	return ctb;
}






inline GameObject* CreateBackground(const char * backgroundName, vec2 position = vec2(0.0f, 0.0f), vec2 scale = vec2(10.0f, 8.0f), bool debugDraw = true)
{
	GameObject* bg = CreateGameObject(GameObjectType::StaticEnvironmentPiece);
	bg->AddTag(GameObjectTags::Background);
	bg->transform.position = position;
	bg->transform.scale = scale;
	bg->AddSprite(backgroundName);

	bg->SetDebugState(debugDraw);

	return bg;
}

inline GameObject* CreateTree(vec2 position = vec2(0.0f, 0.0f), bool debugDraw = true)
{
	GameObject* tree = CreateGameObject(GameObjectType::StaticEnvironmentPiece);

	tree->transform.position = position;
	tree->transform.scale = vec2(2.0f, 2.0f);
	tree->AddTag(GameObjectTags::Environment);
	tree->AddSprite("tree_Generic");
	tree->AddCollisionShape(Rectangle_2D(TileDimensions));

	tree->SetDebugState(debugDraw);

	return tree;
}

inline GameObject* CreateDancingFlowers(vec2 position = vec2(0.0f, 0.0f), bool debugDraw = true)
{
	GameObject* df = CreateGameObject(GameObjectType::StaticEnvironmentPiece);

	df->transform.position = position;
	df->AddTag(GameObjectTags::Environment);
	df->AddAnimator();
	df->animator->AddAnimation("dance", "dancing_Flower", 4, 1.0f);
	df->animator->StartAnimation("dance");

	df->SetDebugState(debugDraw);

	return df;
}

inline GameObject* CreateWeed(vec2 position = vec2(0.0f, 0.0f), bool debugDraw = true)
{
	GameObject* weed = CreateGameObject(GameObjectType::StaticEnvironmentPiece);

	weed->transform.position = position;
	weed->AddTag(GameObjectTags::Environment);
	weed->AddTag(GameObjectTags::Cutable);
	weed->AddTag(GameObjectTags::Burnable);
	weed->AddSprite("weed");
	weed->AddCollisionShape(Rectangle_2D(TileDimensions));

	weed->SetDebugState(debugDraw);

	return weed;
}

inline GameObject* CreateSpookyTree(vec2 position = vec2(0.0f, 0.0f), bool debugDraw = true)
{
	GameObject* spooky = CreateGameObject(GameObjectType::StaticEnvironmentPiece);

	spooky->transform.position = position;
	spooky->transform.scale = vec2(2.0f, 2.0f);
	spooky->AddTag(GameObjectTags::Environment);
	spooky->AddSprite("tree_Spooky");
	spooky->AddCollisionShape(Rectangle_2D(TileDimensions));

	spooky->SetDebugState(debugDraw);

	return spooky;
}

inline GameObject* CreateBlocker(vec2 position = vec2(0.0f, 0.0f), vec2 scale = vec2(1.0f, 1.0f), bool debugDraw = true)
{
	GameObject* blocker = CreateGameObject(GameObjectType::StaticEnvironmentPiece);
	blocker->transform.position = position;
	blocker->transform.scale = scale;
	blocker->AddTag(GameObjectTags::Environment);
	blocker->AddCollisionShape(Rectangle_2D(TileDimensions));

	blocker->SetDebugState(debugDraw);

	return blocker;
}

inline GameObject* CreateButton(vec2 position, bool debugDraw = true)
{
	GameObject* button = CreateGameObject(GameObjectType::Button);

	button->AddTag(GameObjectTags::Environment);
	button->transform.position = position;
	button->AddCollisionShape(Rectangle_2D(TileDimensions, vec2(0.0f, 0.0f), true));

	button->SetDebugState(debugDraw);

	return button;
}

inline GameObject* CreateMiniWall_2x1(vec2 position, bool debugDraw)
{
	GameObject* mw = CreateGameObject(GameObjectType::StaticEnvironmentPiece);
	mw->transform.position = position;
	mw->transform.scale = vec2(1.0f, 0.5f);
	mw->AddSprite("blockers2x1");
	mw->AddTag(GameObjectTags::Environment);
	mw->AddCollisionShape(Rectangle_2D(TileDimensions));

	mw->SetDebugState(debugDraw);

	return mw;
}

inline GameObject* CreateMiniWall_1x2(vec2 position, bool debugDraw)
{
	GameObject* mw = CreateGameObject(GameObjectType::StaticEnvironmentPiece);
	mw->transform.position = position;
	mw->transform.scale = vec2(0.5f, 1.0f);
	mw->AddSprite("blockers1x2");
	mw->AddTag(GameObjectTags::Environment);
	mw->AddCollisionShape(Rectangle_2D(TileDimensions));

	mw->SetDebugState(debugDraw);

	return mw;
}

inline GameObject* CreateMiniWall_1x1(vec2 position, bool debugDraw)
{
	GameObject* mw = CreateGameObject(GameObjectType::StaticEnvironmentPiece);
	mw->transform.position = position;
	mw->transform.scale = vec2(0.5f, 0.5f);
	mw->AddSprite("blockers1x1");
	mw->AddTag(GameObjectTags::Environment);
	mw->AddCollisionShape(Rectangle_2D(TileDimensions));

	mw->SetDebugState(debugDraw);

	return mw;
}
