#pragma once

#include "Types.h"
#include "Math.h"



/*
 *  DEBUG API
 */
void Assert(int expression);

void DebugPrint(char* outputString);
void DebugPrintf(U32 size, char* formatString, ...);






/*
 *  FILE API
 */
bool CreateFile(char* filename, bool overwriteIfExists);

struct GetFileSizeReturnType
{
	size_t fileSize;
	bool fileExists;
};
GetFileSizeReturnType GetFileSize(char* filename);

struct ReadFileReturnType
{
	size_t numberOfBytesRead;
	bool errorEncountered;
};
ReadFileReturnType ReadFile(char* filename, char* fileBuffer, U64 numberOfBytesToRead, U64 readPosition = 0);

struct WriteFileReturnType
{
	size_t numberOfBytesWritten;
	bool errorEncountered;
};
WriteFileReturnType WriteFile(char* filename, char* fileBuffer, U64 numberOfBytesToWrite, U64 writePosition = 0);






/*
 *  WINDOW API
 */
vec2 GetClientWindowDimensions();
void SetClientWindowDimensions(vec2 dimensions);

char* GetWindowTitle();
void  SetWindowTitle(char* newTitle);






/*
 *  INPUT API
 */
enum KeyCode
{
	KeyCode_NULL = 0,

	KeyCode_1 = 4,
	KeyCode_2 = 5,
	KeyCode_3 = 6,
	KeyCode_4 = 7,
	KeyCode_5 = 8,
	KeyCode_6 = 9,
	KeyCode_7 = 10,
	KeyCode_8 = 11,
	KeyCode_9 = 12,
	KeyCode_0 = 13,
	KeyCode_Minus = 14,
	KeyCode_Equal = 15,
	KeyCode_Q = 16,
	KeyCode_W = 17,
	KeyCode_E = 18,
	KeyCode_R = 19,
	KeyCode_T = 20,
	KeyCode_Y = 21,
	KeyCode_U = 22,
	KeyCode_I = 23,
	KeyCode_O = 24,
	KeyCode_P = 25,
	KeyCode_LeftBracket = 26,
	KeyCode_RightBracket = 27,
	KeyCode_BackSlash = 28,
	KeyCode_A = 29,
	KeyCode_S = 30,
	KeyCode_D = 31,
	KeyCode_F = 32,
	KeyCode_G = 33,
	KeyCode_H = 34,
	KeyCode_J = 35,
	KeyCode_K = 36,
	KeyCode_L = 37,
	KeyCode_SemiColon = 38,
	KeyCode_Quote = 39,
	KeyCode_Z = 40,
	KeyCode_X = 41,
	KeyCode_C = 42,
	KeyCode_V = 43,
	KeyCode_B = 44,
	KeyCode_N = 45,
	KeyCode_M = 46,
	KeyCode_Comma = 47,
	KeyCode_Period = 48,
	KeyCode_ForwardSlash = 49,
	KeyCode_Spacebar = 50,
	KeyCode_Up = 51,
	KeyCode_Down = 52,
	KeyCode_Left = 53,
	KeyCode_Right = 54,

	KeyCode_COUNT
};

enum MouseCode
{
	MouseCode_NULL = 0,

	MouseCode_Left = 1,
	MouseCode_Middle = 2,
	MouseCode_Right = 3,

	MouseCode_COUNT
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
	GamepadCode_Y = 14,

	GamepadCode_COUNT
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