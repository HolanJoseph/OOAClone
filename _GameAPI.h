#pragma once

#include "Types.h"

// NOTE: deltaTime is in milliseconds (seconds?)
bool GameInitialize();
void GameUpdate(F32 deltaTime);
bool GameShutdown();