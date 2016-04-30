#pragma once
#include "Types.h"

// NOTE: deltaTime is in milliseconds
bool GameInitialize();
void GameUpdate(F32 deltaTime);
bool GameShutdown();