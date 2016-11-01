#pragma once

#include "Types.h"

bool GameInitialize();
void GameUpdate(F32 deltaTime); // NOTE: deltaTime is in seconds
bool GameShutdown();