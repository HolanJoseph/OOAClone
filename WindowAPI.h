#pragma once

#include "Types.h"
#include "Math.h"

vec2 GetClientWindowDimensions();
void SetClientWindowDimensions(vec2 dimensions);

char* GetWindowTitle();
void  SetWindowTitle(char* newTitle);