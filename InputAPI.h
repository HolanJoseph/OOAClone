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
bool getKeyDown(KeyCode code);
bool getKey(KeyCode code);
bool getKeyUp(KeyCode code);



// Mouse
vec2 getMousePosition();
vec2 getMouseMovement();

bool getMouseButtonDown(MouseCode code);
bool getMouseButton(MouseCode code);
bool getMouseButtonUp(MouseCode code);

void setMousePosition(vec2 newPos);

void setCursorVisibility(bool newVis);



// Controller
bool isGamepadConnected();

bool getGamepadButtonDown(GamepadCode code);
bool getGamepadButton(GamepadCode code);
bool getGamepadButtonUp(GamepadCode code);

F32  getGamepadLeftTrigger();
F32  getGamepadRightTrigger();

vec2 getGamepadLeftStick();
vec2 getGamepadRightStick();