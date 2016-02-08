#include <windows.h>
#include <Xinput.h>

#include <stdio.h>
//#define NDEBUG
#include <cassert>

#include "glm/glm.hpp"
//#include "glm/vec2.hpp"
using glm::vec2;
using glm::length;
using glm::normalize;

#include "Types.h"



U8 inputBuffer;
U8 inputBackBuffer;
enum InputCode
{
	InputCode_NULL = 0,
	InputCode_W = 1,
	InputCode_A = 2,
	InputCode_S = 3,
	InputCode_D = 4,
	InputCode_J = 5,
	InputCode_K = 6,
	InputCode_L = 7
};

I32 hdMouseDeltaX;
I32 hdMouseDeltaY;

InputCode translateVKCodeToInputCode(UINT_PTR vkCode)
{
	InputCode code;
	switch (vkCode)
	{
	case 0x57:
	code = InputCode_W;
	break;

	case 0x41:
	code = InputCode_A;
	break;

	case 0x53:
	code = InputCode_S;
	break;

	case 0x44:
	code = InputCode_D;
	break;

	case 0x4A:
	code = InputCode_J;
	break;

	case 0x4B:
	code = InputCode_K;
	break;

	case 0x4C:
	code = InputCode_L;
	break;

	default:
	code = InputCode_NULL;
	break;
	}
	return code;
}

void setBit(U8* buffer, InputCode code, U8 val)
{
	(*buffer) = (*buffer) ^ ((-val ^ (*buffer)) & (1 << code));
}

bool checkDown(U8* buffer, InputCode code)
{
	bool result = ((*buffer) >> code) & 1;
	return result;
}



// NOTE: getKeyDown, getKey, getKeyUp are referring to the current frame.
bool getKeyDown(InputCode code)
{
	bool result = checkDown(&inputBuffer, code) && !checkDown(&inputBackBuffer, code);
	return result;
}

bool getKey(InputCode code)
{
	bool result = checkDown(&inputBuffer, code);
	return result;
}

bool getKeyUp(InputCode code)
{
	bool result = !checkDown(&inputBuffer, code) && checkDown(&inputBackBuffer, code);
	return result;
}



struct Gamepad
{
	U16 buttons;
	vec2 leftThumbstick;
	vec2 rightThumbstick;
	F32 leftTrigger;
	F32 rightTrigger;
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

bool controllerConnected = false;
Gamepad gamepad;
Gamepad backGamepad;

void setBit(U16* buffer, GamepadCode code, U8 val)
{
	(*buffer) = (*buffer) ^ ((-val ^ (*buffer)) & (1 << code));
}

bool checkDown(U16* buffer, GamepadCode code)
{
	bool result = ((*buffer) >> code) & 1;
	return result;
}

bool getButtonDown(GamepadCode code)
{
	bool result = checkDown(&gamepad.buttons, code) && !checkDown(&backGamepad.buttons, code);
	return result;
}

bool getButton(GamepadCode code)
{
	bool result = checkDown(&gamepad.buttons, code);
	return result;
}

bool getButtonUp(GamepadCode code)
{
	bool result = !checkDown(&gamepad.buttons, code) && checkDown(&backGamepad.buttons, code);
	return result;
}

F32 clampRange(F32 val, F32 low, F32 high)
{
	F32 result = val;

	if (val < low)
	{
		result = low;
	}
	else if (val > high)
	{
		result = high;
	}

	return result;
}

LRESULT CALLBACK Win32WindowCallback( HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam )    
{
	switch (message)
	{
	/*
		Window lifetime messages.

		NOTE: Pass through cases are pulled out for consistency and later work
	*/
	case WM_NCCREATE:
	{
						OutputDebugString("WM_NCCREATE\n");
						// NOTE: icon, min, max, close already present
						// NOTE: Shows the window title
						return DefWindowProc(windowHandle, message, wParam, lParam);
						break;
	}

	case WM_CREATE:
	{
					  OutputDebugString("WM_CREATE\n");
					  // NOTE: no visual cue as to what this does
					  return DefWindowProc(windowHandle, message, wParam, lParam);
					  break;
	}

	case WM_CLOSE:
	{
					 OutputDebugString("WM_CLOSE\n");
					 // NOTE: message sent when the close button is clicked 
					 // NOTE: consider prompting the user for confirmation prior to destroying a window
					 // NOTE: sends WM_DESTROY message to the app
					 // NOTE: Is this correct? I cannot seem to find any documentation on what WM_CLOSE should return on success
					 return DestroyWindow(windowHandle);
					 break;
	}

	case WM_DESTROY:
	{
					   OutputDebugString("WM_DESTROY\n");
					   PostQuitMessage(0);
					   return 0;
					   break;
	}

	case WM_NCDESTROY:
	{
						 OutputDebugString("WM_NCDESTROY\n");
						 // NOTE: Why do we get this message?
						 return DefWindowProc(windowHandle, message, wParam, lParam);
						 break;
	}

	case WM_QUIT:
	{
					// NOTE: This message is never received by the wndproc as it is intercepted by the system.
					break;
	}



	/*
		Window show state messages

		WM_ACTIVATEAPP
		WM_ENABLE
		WM_QUERYOPEN
		WM_SHOWWINDOW
	*/
	case WM_ACTIVATEAPP:
	{
						   // Confine the mouse cursor to the window when the app is active,
						   //	and release it when it is inactive
						   OutputDebugString("WM_ACTIVATEAPP\n");
						   if (wParam == TRUE)
						   {
							   // Clip
							   RECT windowPos;
							   GetWindowRect(windowHandle, &windowPos);
							   ClipCursor(&windowPos);

							   // Center
							   I32 middleX = windowPos.left + ((windowPos.right - windowPos.left)/2);
							   I32 middleY = windowPos.top + ((windowPos.bottom - windowPos.top)/2);
							   SetCursorPos(middleX, middleY);
						   }
						   else
						   {
							   ClipCursor(NULL);
						   }
						   break;
	}



	/*
		Window resource messages

		WM_SETICON
		WM_GETICON
		WM_QUERYDRAGICON
	*/



	/*
		Window sizing messages

		WM_DPICHANGED
		WM_SIZING
		WM_WINDOWPOSCHANGING
		WM_WINDOWPOSCHANGED
		WM_SIZE
		WM_MOVE
		WM_ENTERSIZEMOVE
		WM_EXITSIZEMOVE
	*/



	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	{
						  setBit(&inputBuffer, translateVKCodeToInputCode(wParam), 1);
						  return 0;
						  break;
	}

	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
						setBit(&inputBuffer, translateVKCodeToInputCode(wParam), 0);
						return 0;
						break;
	}

	case WM_INPUT:
	{


					 UINT riSize = sizeof(RAWINPUT);
					 RAWINPUT ri;
					 I32 retval = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &ri, &riSize, sizeof(RAWINPUTHEADER));
					 if (ri.header.dwType == RIM_TYPEMOUSE)
					 {
						 hdMouseDeltaX += ri.data.mouse.lLastX;
						 hdMouseDeltaY += ri.data.mouse.lLastY;
					 }

					 return 0;
					 break;
	}

	default:
	{
			   return DefWindowProc(windowHandle, message, wParam, lParam);
			   break;
	}
	}
	return 0; // NOTE: This should never be reached.
}



INT WINAPI WinMain(HINSTANCE instanceHandle, HINSTANCE deadArg, PSTR commandLineArgsUnicode, INT showOnStartup)
{
	char* windowClassName = "OracleOfAgesCloneMainWindow";
	char* windowTitle = "Oracle of Ages Clone";
	U32 windowWidth = 600;
	U32 windowHeight = 500;
	DWORD windowStyle = WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_OVERLAPPED | WS_SIZEBOX | WS_VISIBLE;

	WNDCLASSEX windowClass;
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = Win32WindowCallback;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = instanceHandle;
	windowClass.hIcon = LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_APPLICATION));
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); //NULL this for painting to client area
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = windowClassName;
	windowClass.hIconSm = LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_APPLICATION));
	if (!RegisterClassEx(&windowClass))
	{
		OutputDebugString("Was not able to register the window class.");
		return 1;
	}

	RECT windowRect = {0, 0, windowWidth, windowHeight}; // UL x, UL y, BR x, BR y
	AdjustWindowRect(&windowRect, windowStyle, FALSE);
	U32 adjustedWidth = windowRect.right - windowRect.left;
	U32 adjustedHeight = windowRect.bottom - windowRect.top;
	assert(adjustedWidth > windowWidth);
	assert(adjustedHeight > windowHeight);

	HWND windowHandle = CreateWindow(
		windowClassName,
		windowTitle,
		windowStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		adjustedWidth, // total sizes
		adjustedHeight,
		NULL,
		NULL,
		instanceHandle,
		NULL);
	if (!windowHandle)
	{
		OutputDebugString("Was not able to register the window class.");
		return 1;
	}

	ShowWindow(windowHandle, showOnStartup);
	UpdateWindow(windowHandle);



	// NOTE: Buttons Yo
	inputBuffer = 0;
	inputBackBuffer = 0;

	// Register for raw mouse messages
	hdMouseDeltaX = 0;
	hdMouseDeltaY = 0;
	RAWINPUTDEVICE rawMouse;
	rawMouse.usUsagePage = 0x01;
	rawMouse.usUsage = 0x02;
	rawMouse.dwFlags = 0;
	rawMouse.hwndTarget = 0;
	if (RegisterRawInputDevices(&rawMouse, 1, sizeof(rawMouse)) == FALSE)
	{
		OutputDebugString("Was not able to set up raw mouse input");
		return 1;
	}

	gamepad = { 0 };
	backGamepad = { 0 };


	// Clip mouse movement to the window
	// NOTE: Account for the borders so the cursor can only move in the client area?

	// Center the mouse in the window

	if (timeBeginPeriod(1) == TIMERR_NOCANDO)
	{
		OutputDebugString("Was not able to set timer granularity");
		return 1;
	}

	bool running = true;
	MSG windowsMessage;
	
	LARGE_INTEGER qpf;
	QueryPerformanceFrequency(&qpf);
	I64 performanceFrequency = (I64)qpf.QuadPart; // This is number of counts per second

	LARGE_INTEGER frameStartLI;
	QueryPerformanceCounter(&frameStartLI);

	while (running)
	{
		// NOTE: Copy over last frames data, so key up/ key down can be queried
		inputBackBuffer = inputBuffer;

		// NOTE: Reset the HD mouse delta for the frame
		hdMouseDeltaX = 0;
		hdMouseDeltaY = 0;

		// NOTE: Copy over last frames gamepad state
		backGamepad = gamepad;

		// look at all of the messages in the message queue
		while (PeekMessage(&windowsMessage, NULL, 0, 0, PM_REMOVE))
		{
			// NOTE: WM_QUIT is kept in the message queue until it is the last message
			if (windowsMessage.message == WM_QUIT)
			{
				running = false;
				break;
			}
			else
			{
				TranslateMessage(&windowsMessage);
				DispatchMessage(&windowsMessage);
			}
		}

		// Query controller states
		XINPUT_STATE controllerState;
		DWORD controllerStatus = XInputGetState(0, &controllerState);
		if (controllerStatus == ERROR_SUCCESS)
		{
			controllerConnected = true;

			if ((controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0)
			{
				setBit(&(gamepad.buttons), GamepadCode_Up, 1);
			}
			else 
			{
				setBit(&(gamepad.buttons), GamepadCode_Up, 0);
			}

			if ((controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0)
			{
				setBit(&(gamepad.buttons), GamepadCode_Down, 1);
			}
			else
			{
				setBit(&(gamepad.buttons), GamepadCode_Down, 0);
			}

			if ((controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0)
			{
				setBit(&(gamepad.buttons), GamepadCode_Left, 1);
			}
			else
			{
				setBit(&(gamepad.buttons), GamepadCode_Left, 0);
			}

			if ((controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0)
			{
				setBit(&(gamepad.buttons), GamepadCode_Right, 1);
			}
			else
			{
				setBit(&(gamepad.buttons), GamepadCode_Right, 0);
			}

			if ((controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0)
			{
				setBit(&(gamepad.buttons), GamepadCode_Start, 1);
			}
			else
			{
				setBit(&(gamepad.buttons), GamepadCode_Start, 0);
			}

			if ((controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0)
			{
				setBit(&(gamepad.buttons), GamepadCode_Back, 1);
			}
			else
			{
				setBit(&(gamepad.buttons), GamepadCode_Back, 0);
			}

			if ((controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0)
			{
				setBit(&(gamepad.buttons), GamepadCode_L3, 1);
			}
			else
			{
				setBit(&(gamepad.buttons), GamepadCode_L3, 0);
			}

			if ((controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0)
			{
				setBit(&(gamepad.buttons), GamepadCode_R3, 1);
			}
			else
			{
				setBit(&(gamepad.buttons), GamepadCode_R3, 0);
			}

			if ((controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0)
			{
				setBit(&(gamepad.buttons), GamepadCode_LeftBumper, 1);
			}
			else
			{
				setBit(&(gamepad.buttons), GamepadCode_LeftBumper, 0);
			}

			if ((controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0)
			{
				setBit(&(gamepad.buttons), GamepadCode_RightBumper, 1);
			}
			else
			{
				setBit(&(gamepad.buttons), GamepadCode_RightBumper, 0);
			}

			if ((controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0)
			{
				setBit(&(gamepad.buttons), GamepadCode_A, 1);
			}
			else
			{
				setBit(&(gamepad.buttons), GamepadCode_A, 0);
			}

			if ((controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0)
			{
				setBit(&(gamepad.buttons), GamepadCode_B, 1);
			}
			else
			{
				setBit(&(gamepad.buttons), GamepadCode_B, 0);
			}

			if ((controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0)
			{
				setBit(&(gamepad.buttons), GamepadCode_X, 1);
			}
			else
			{
				setBit(&(gamepad.buttons), GamepadCode_X, 0);
			}

			if ((controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0)
			{
				setBit(&(gamepad.buttons), GamepadCode_Y, 1);
			}
			else
			{
				setBit(&(gamepad.buttons), GamepadCode_Y, 0);
			}


			gamepad.leftTrigger = controllerState.Gamepad.bLeftTrigger;
			gamepad.rightTrigger = controllerState.Gamepad.bRightTrigger;

			gamepad.leftThumbstick = vec2(controllerState.Gamepad.sThumbLX, controllerState.Gamepad.sThumbLY);
			F32 leftStickMagnitude = length(gamepad.leftThumbstick);
			leftStickMagnitude = clampRange(leftStickMagnitude, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, I16_MAX);
			leftStickMagnitude -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
			leftStickMagnitude /= (I16_MAX - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
			gamepad.leftThumbstick = normalize(gamepad.leftThumbstick) * leftStickMagnitude;

			gamepad.rightThumbstick = vec2(controllerState.Gamepad.sThumbRX, controllerState.Gamepad.sThumbRY);
			F32 rightStickMagnitude = length(gamepad.rightThumbstick);
			rightStickMagnitude = clampRange(rightStickMagnitude, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE, I16_MAX);
			rightStickMagnitude -= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
			rightStickMagnitude /= (I16_MAX - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
			gamepad.rightThumbstick = normalize(gamepad.rightThumbstick) * rightStickMagnitude;

			// Triggers
			gamepad.leftTrigger = clampRange(controllerState.Gamepad.bLeftTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD, U8_MAX);
			gamepad.leftTrigger -= XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
			gamepad.leftTrigger /= (U8_MAX - XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

			gamepad.rightTrigger = clampRange(controllerState.Gamepad.bRightTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD, U8_MAX);
			gamepad.rightTrigger -= XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
			gamepad.rightTrigger /= (U8_MAX - XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

		}
		else
		{
			controllerConnected = false;
		}

		if (getButtonDown(GamepadCode_A))
		{
			OutputDebugString("Gamepad A Down\n");
		}
		if (getButtonUp(GamepadCode_A))
		{
			OutputDebugString("Gamepad A Up\n");
		}
		if (getButtonDown(GamepadCode_B))
		{
			OutputDebugString("Gamepad B Down\n");
		}
		if (getButtonUp(GamepadCode_B))
		{
			OutputDebugString("Gamepad B Up\n");
		}
		if (getButtonDown(GamepadCode_X))
		{
			OutputDebugString("Gamepad X Down\n");
		}
		if (getButtonUp(GamepadCode_X))
		{
			OutputDebugString("Gamepad X Up\n");
		}
		if (getButtonDown(GamepadCode_Y))
		{
			OutputDebugString("Gamepad Y Down\n");
		}
		if (getButtonUp(GamepadCode_Y))
		{
			OutputDebugString("Gamepad Y Up\n");
		}
		if (getButtonDown(GamepadCode_Up))
		{
			OutputDebugString("Gamepad up Down\n");
		}
		if (getButtonUp(GamepadCode_Up))
		{
			OutputDebugString("Gamepad up Up\n");
		}
		if (getButtonDown(GamepadCode_Right))
		{
			OutputDebugString("Gamepad right Down\n");
		}
		if (getButtonUp(GamepadCode_Right))
		{
			OutputDebugString("Gamepad right Up\n");
		}
		if (getButtonDown(GamepadCode_Down))
		{
			OutputDebugString("Gamepad down Down\n");
		}
		if (getButtonUp(GamepadCode_Down))
		{
			OutputDebugString("Gamepad down Up\n");
		}
		if (getButtonDown(GamepadCode_Left))
		{
			OutputDebugString("Gamepad left Down\n");
		}
		if (getButtonUp(GamepadCode_Left))
		{
			OutputDebugString("Gamepad left Up\n");
		}

		if (getButtonDown(GamepadCode_Start))
		{
			OutputDebugString("Gamepad start Down\n");
		}
		if (getButtonUp(GamepadCode_Start))
		{
			OutputDebugString("Gamepad start Up\n");
		}
		if (getButtonDown(GamepadCode_Back))
		{
			OutputDebugString("Gamepad Back Down\n");
		}
		if (getButtonUp(GamepadCode_Back))
		{
			OutputDebugString("Gamepad Back Up\n");
		}

		if (getButtonDown(GamepadCode_L3))
		{
			OutputDebugString("Gamepad L3 Down\n");
		}
		if (getButtonUp(GamepadCode_L3))
		{
			OutputDebugString("Gamepad L3 Up\n");
		}
		if (getButtonDown(GamepadCode_R3))
		{
			OutputDebugString("Gamepad R3 Down\n");
		}
		if (getButtonUp(GamepadCode_R3))
		{
			OutputDebugString("Gamepad R3 Up\n");
		}

		if (getButtonDown(GamepadCode_RightBumper))
		{
			OutputDebugString("Gamepad rb Down\n");
		}
		if (getButtonUp(GamepadCode_RightBumper))
		{
			OutputDebugString("Gamepad rb Up\n");
		}
		if (getButtonDown(GamepadCode_LeftBumper))
		{
			OutputDebugString("Gamepad lb Down\n");
		}
		if (getButtonUp(GamepadCode_LeftBumper))
		{
			OutputDebugString("Gamepad lb Up\n");
		}

		if (gamepad.leftTrigger)
		{
			char buff[256];
			sprintf_s(buff, 256, "Left Trigger: %f\n", gamepad.leftTrigger);
			OutputDebugString(buff);
		}

		if (gamepad.rightTrigger)
		{
			char buff[256];
			sprintf_s(buff, 256, "Right Trigger: %f\n", gamepad.rightTrigger);
			OutputDebugString(buff);
		}


		if (gamepad.leftThumbstick != vec2(0,0))
		{
			char buff[256];
			sprintf_s(buff, 256, "Left Stick: (%f, %f)\n", gamepad.leftThumbstick.x, gamepad.leftThumbstick.y);
			OutputDebugString(buff);
		}


		Sleep(16);

		LARGE_INTEGER frameEndLI;
		QueryPerformanceCounter(&frameEndLI);
		F32 frameElapsedTime = (F32)((frameEndLI.QuadPart - frameStartLI.QuadPart) * 1000) / performanceFrequency;
		frameStartLI = frameEndLI;

		char buff[512];
		sprintf_s(buff, 512, "frame time %fms\n", frameElapsedTime);
		//OutputDebugString(buff); 
	}

	// 
	timeEndPeriod(1);

	return 0;
}