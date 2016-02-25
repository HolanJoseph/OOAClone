#pragma once
#include "Types.h"
#include "Math.h"

enum KeyCode
{
	KeyCode_NULL = 0,
	KeyCode_W = 4,
	KeyCode_A = 5,
	KeyCode_S = 6,
	KeyCode_D = 7,
	KeyCode_J = 8,
	KeyCode_K = 9,
	KeyCode_L = 10
};

enum MouseCode
{
	MouseCode_NULL = 0,
	MouseCode_Left = 1,
	MouseCode_Middle = 2,
	MouseCode_Right = 3
};

enum GamepadCode
{
	GamepadCode_NULL = 0,
	GamepadCode_Up = 1,
	GamepadCode_Down = 2,
	GamepadCode_Left = 3,
	GamepadCode_Right = 4,
	GamepadCode_Start = 5,
	GamepadCode_Back = 6,
	GamepadCode_L3 = 7,
	GamepadCode_R3 = 8,
	GamepadCode_LeftBumper = 9,
	GamepadCode_RightBumper = 10,
	GamepadCode_A = 11,
	GamepadCode_B = 12,
	GamepadCode_X = 13,
	GamepadCode_Y = 14
};


// Keyboard
bool GetKeyDown(KeyCode code);
bool GetKey(KeyCode code);
bool GetKeyUp(KeyCode code);



// Mouse
vec2 GetMousePosition();
vec2 GetMouseMovement();

bool GetMouseButtonDown(MouseCode code);
bool GetMouseButton(MouseCode code);
bool GetMouseButtonUp(MouseCode code);

void SetMousePosition(vec2 newPos);

void SetCursorVisibility(bool newVis);



// Controller
bool IsGamepadConnected();

bool GetGamepadButtonDown(GamepadCode code);
bool GetGamepadButton(GamepadCode code);
bool GetGamepadButtonUp(GamepadCode code);

F32  GetGamepadLeftTrigger();
F32  GetGamepadRightTrigger();

vec2 GetGamepadLeftStick();
vec2 GetGamepadRightStick();