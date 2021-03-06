#include "Types.h"
#include "Math.h"

#include <windows.h>
#include <Xinput.h>
#include "glew/GL/glew.h"
#include "glew/GL/wglew.h"
#include <gl/GL.h>
#include <stdio.h>
//#define NDEBUG
#include <cassert>
#include <stdarg.h>
#include <random>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_BMP
#include "3rdParty/stb_image.h"

#include "BitManip.h"
#include "Util.h"
#include "String.h"


// NOTE: Functions from these files must be implemented in this file
#include "_SystemAPI.h"


// NOTE: These functions are NOT implemented in this file, they
//			are called to let the game do what it needs to do.
#include "_GameAPI.h"



// NOTE: Windows stuff
HWND  windowHandle;
HDC   windowDC;
HGLRC windowOpenGLContext;

ULARGE_INTEGER applicationStartTime;

// NOTE: Input globals
U64 inputBuffer;
U64 inputBackBuffer;

vec2 hdMouseMovement;
bool cursorVisibility;

struct Gamepad
{
	bool connected;
	U16 buttons;
	vec2 leftThumbstick;
	vec2 rightThumbstick;
	F32 leftTrigger;
	F32 rightTrigger;
};
Gamepad gamepad;
Gamepad backGamepad;

KeyCode TranslateVKCodeToKeyCode(UINT_PTR vkCode);
void QueryController();



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
						DebugPrint("WM_NCCREATE\n");
						// NOTE: icon, min, max, close already present
						// NOTE: Shows the window title
						return DefWindowProc(windowHandle, message, wParam, lParam);
						break;;
	}

	case WM_CREATE:
	{
					  DebugPrint("WM_CREATE\n");
					  // NOTE: no visual cue as to what this does
					  return DefWindowProc(windowHandle, message, wParam, lParam);
					  break;
	}

	case WM_CLOSE:
	{
					 DebugPrint("WM_CLOSE\n");
					 // NOTE: message sent when the close button is clicked 
					 // NOTE: consider prompting the user for confirmation prior to destroying a window
					 // NOTE: sends WM_DESTROY message to the app
					 // NOTE: Is this correct? I cannot seem to find any documentation on what WM_CLOSE should return on success
					 return DestroyWindow(windowHandle);
					 break;
	}

	case WM_DESTROY:
	{
					   DebugPrint("WM_DESTROY\n");
					   PostQuitMessage(0);
					   return 0;
					   break;
	}

	case WM_NCDESTROY:
	{
						 DebugPrint("WM_NCDESTROY\n");
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
						   DebugPrint("WM_ACTIVATEAPP\n");
						   if (wParam == TRUE)
						   {
							   // Capture mouse
							   //SetCapture(windowHandle);

							   // Clip
							   RECT windowPos;
							   GetWindowRect(windowHandle, &windowPos);
							   //ClipCursor(&windowPos);

							   // Center
							   I32 middleX = windowPos.left + ((windowPos.right - windowPos.left)/2);
							   I32 middleY = windowPos.top + ((windowPos.bottom - windowPos.top)/2);
							   SetCursorPos(middleX, middleY);
						   }
						   else
						   {
							   //ReleaseCapture();
							   //ClipCursor(NULL);
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
						  SetBit(&inputBuffer, TranslateVKCodeToKeyCode(wParam), 1);
						  return 0;
						  break;
	}

	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
						SetBit(&inputBuffer, TranslateVKCodeToKeyCode(wParam), 0);
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
						 hdMouseMovement.x += ri.data.mouse.lLastX;
						 hdMouseMovement.y += ri.data.mouse.lLastY;
					 }

					 return 0;
					 break;
	}

	case WM_LBUTTONDOWN:
	{
						   SetBit(&inputBuffer, MouseCode_Left, 1);
						   return 0;
						   break;
	}
	case WM_LBUTTONUP:
	{
						 SetBit(&inputBuffer, MouseCode_Left, 0);
						 return 0;
						 break;
	}

	case WM_MBUTTONDOWN:
	{
						   SetBit(&inputBuffer, MouseCode_Middle, 1);
						   return 0;
						   break;
	}
	case WM_MBUTTONUP:
	{
						 SetBit(&inputBuffer, MouseCode_Middle, 0);
						 return 0;
						 break;
	}

	case WM_RBUTTONDOWN:
	{
						   SetBit(&inputBuffer, MouseCode_Right, 1);
						   return 0;
						   break;
	}
	case WM_RBUTTONUP:
	{
						 SetBit(&inputBuffer, MouseCode_Right, 0);
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
// 	U32 windowWidth = 600;
// 	U32 windowHeight = 540;
	U32 windowWidth = 1600;// 800;
	U32 windowHeight = 800;
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
		DebugPrint("Was not able to register the window class.");
		return 1;
	}

	RECT windowRect = {0, 0, windowWidth, windowHeight}; // UL x, UL y, BR x, BR y
	AdjustWindowRect(&windowRect, windowStyle, FALSE);
	U32 adjustedWidth = windowRect.right - windowRect.left;
	U32 adjustedHeight = windowRect.bottom - windowRect.top;
	assert(adjustedWidth > windowWidth);
	assert(adjustedHeight > windowHeight);

	windowHandle = CreateWindow(
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
		DebugPrint("Was not able to register the window class.");
		return 1;
	}

	windowDC = GetDC(windowHandle);
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
		PFD_TYPE_RGBA,            //The kind of framebuffer. RGBA or palette.
		32,                        //Colordepth of the framebuffer.
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24,                        //Number of bits for the depthbuffer
		8,                        //Number of bits for the stencilbuffer
		0,                        //Number of Aux buffers in the framebuffer.
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};
	I32 pixelFormat = ChoosePixelFormat(windowDC, &pfd);
	SetPixelFormat(windowDC, pixelFormat, &pfd);
	HGLRC tempOpenGLContext = wglCreateContext(windowDC);
	wglMakeCurrent(windowDC, tempOpenGLContext);
	glewExperimental=GL_TRUE;
	GLenum error = glewInit();
	if (error != GLEW_OK)
	{
		DebugPrintf(512, "GLEW Error: %s", glewGetErrorString(error));
		return 1;
	}
	I32 openglCreationAttributes[] = {
										WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
										WGL_CONTEXT_MINOR_VERSION_ARB, 3,
										WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
										0
									};
	if (wglewIsSupported("WGL_ARB_create_context") != 1)
	{
		DebugPrint("GLEW could not support creation of a higher level OpenGL context");
		return 1;
	}
	windowOpenGLContext = wglCreateContextAttribsARB(windowDC, NULL, openglCreationAttributes);
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(tempOpenGLContext);
	wglMakeCurrent(windowDC, windowOpenGLContext);
	I32 glMajor; 
	I32 glMinor;
	GLboolean glDoublebuffered;
	glGetIntegerv(GL_MAJOR_VERSION, &glMajor);
	glGetIntegerv(GL_MINOR_VERSION, &glMinor);
	glGetBooleanv(GL_DOUBLEBUFFER, &glDoublebuffered);
	DebugPrintf(256, "GL Version %i.%i\nIs doublebuffered: %i\n", glMajor, glMinor, glDoublebuffered);


	// NOTE: Time
	FILETIME tempFT;
	GetSystemTimeAsFileTime(&tempFT);
	applicationStartTime.HighPart = tempFT.dwHighDateTime;
	applicationStartTime.LowPart = tempFT.dwLowDateTime;


	bool gameSetup = GameInitialize();
	if (!gameSetup)
	{
		DebugPrint("Couldn't set up game state\n");
		return 1;
	}


	ShowWindow(windowHandle, showOnStartup);
	UpdateWindow(windowHandle);

	// NOTE: Buttons Yo
	inputBuffer = 0;
	inputBackBuffer = 0;

	// Register for raw mouse messages
	hdMouseMovement = vec2(0, 0);
	RAWINPUTDEVICE rawMouse;
	rawMouse.usUsagePage = 0x01;
	rawMouse.usUsage = 0x02;
	rawMouse.dwFlags = 0;
	rawMouse.hwndTarget = 0;
	if (RegisterRawInputDevices(&rawMouse, 1, sizeof(rawMouse)) == FALSE)
	{
		DebugPrint("Was not able to set up raw mouse input");
		return 1;
	}
	cursorVisibility = true;

	gamepad = { 0 };
	backGamepad = { 0 };


	// Clip mouse movement to the window
	// NOTE: Account for the borders so the cursor can only move in the client area?

	// Center the mouse in the window

	if (timeBeginPeriod(1) == TIMERR_NOCANDO)
	{
		DebugPrint("Was not able to set timer granularity");
		return 1;
	}

	
	LARGE_INTEGER qpf;
	QueryPerformanceFrequency(&qpf);
	I64 performanceFrequency = (I64)qpf.QuadPart; // This is number of counts per second

	LARGE_INTEGER frameStartLI;
	F32 frameElapsedTime = 1.0f/60.0f;
	QueryPerformanceCounter(&frameStartLI);

	bool running = true;
	MSG windowsMessage;
	while (running)
	{
		// NOTE: Copy over last frames data, so key up/ key down can be queried
		inputBackBuffer = inputBuffer;

		// NOTE: Reset the HD mouse delta for the frame
		hdMouseMovement = vec2(0, 0);

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
			gamepad.connected = true;
			
			// Buttons
			SetBit(&gamepad.buttons, GamepadCode_Up,    (U8)ClampRange_F32((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP), 0, 1));
			SetBit(&gamepad.buttons, GamepadCode_Down,  (U8)ClampRange_F32((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN), 0, 1));
			SetBit(&gamepad.buttons, GamepadCode_Left,  (U8)ClampRange_F32((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT), 0, 1));
			SetBit(&gamepad.buttons, GamepadCode_Right, (U8)ClampRange_F32((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT), 0, 1));
			SetBit(&gamepad.buttons, GamepadCode_Start, (U8)ClampRange_F32((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_START), 0, 1));
			SetBit(&gamepad.buttons, GamepadCode_Back,  (U8)ClampRange_F32((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK), 0, 1));
			SetBit(&gamepad.buttons, GamepadCode_X, (U8)ClampRange_F32((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_X), 0, 1));
			SetBit(&gamepad.buttons, GamepadCode_Y, (U8)ClampRange_F32((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_Y), 0, 1));
			SetBit(&gamepad.buttons, GamepadCode_A, (U8)ClampRange_F32((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_A), 0, 1));
			SetBit(&gamepad.buttons, GamepadCode_B, (U8)ClampRange_F32((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_B), 0, 1));
			SetBit(&gamepad.buttons, GamepadCode_RightBumper, (U8)ClampRange_F32((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER), 0, 1));
			SetBit(&gamepad.buttons, GamepadCode_R3, (U8)ClampRange_F32((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB), 0, 1));
			SetBit(&gamepad.buttons, GamepadCode_LeftBumper, (U8)ClampRange_F32((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER), 0, 1));
			SetBit(&gamepad.buttons, GamepadCode_L3, (U8)ClampRange_F32((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB), 0, 1));

			// Thumb sticks
			gamepad.leftThumbstick = vec2(controllerState.Gamepad.sThumbLX, controllerState.Gamepad.sThumbLY);
			F32 leftStickMagnitude = length(gamepad.leftThumbstick);
			leftStickMagnitude = ClampRange_F32(leftStickMagnitude, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, I16_MAX);
			leftStickMagnitude -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
			leftStickMagnitude /= (I16_MAX - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
			gamepad.leftThumbstick = normalize(gamepad.leftThumbstick) * leftStickMagnitude;

			gamepad.rightThumbstick = vec2(controllerState.Gamepad.sThumbRX, controllerState.Gamepad.sThumbRY);
			F32 rightStickMagnitude = length(gamepad.rightThumbstick);
			rightStickMagnitude = ClampRange_F32(rightStickMagnitude, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE, I16_MAX);
			rightStickMagnitude -= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
			rightStickMagnitude /= (I16_MAX - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
			gamepad.rightThumbstick = normalize(gamepad.rightThumbstick) * rightStickMagnitude;

			// Triggers
			gamepad.leftTrigger = ClampRange_F32(controllerState.Gamepad.bLeftTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD, U8_MAX);
			gamepad.leftTrigger -= XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
			gamepad.leftTrigger /= (U8_MAX - XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

			gamepad.rightTrigger = ClampRange_F32(controllerState.Gamepad.bRightTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD, U8_MAX);
			gamepad.rightTrigger -= XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
			gamepad.rightTrigger /= (U8_MAX - XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

		}
		else
		{
			gamepad.connected = false;
		}

		
		
		GameUpdate(frameElapsedTime);

		glFlush(); // NOTE: Necessary???
		SwapBuffers(windowDC);

		LARGE_INTEGER frameEndLI;
		QueryPerformanceCounter(&frameEndLI);
		frameElapsedTime = (F32)((frameEndLI.QuadPart - frameStartLI.QuadPart)) / performanceFrequency;
		frameStartLI = frameEndLI;
	}

	// 
	timeEndPeriod(1);

	GameShutdown();

	wglMakeCurrent(windowDC, NULL);
	wglDeleteContext(windowOpenGLContext);
	ReleaseDC(windowHandle, windowDC);

	return 0;
}






/*
	WINDOW API IMPLEMENTATION
 */
vec2 GetClientWindowDimensions()
{
	vec2 result;

	RECT windowRectangle;
	GetClientRect( windowHandle,&windowRectangle);

	result.x = (F32)(windowRectangle.right - windowRectangle.left);
	result.y = (F32)(windowRectangle.bottom - windowRectangle.top);

	return result;
}

// NOTE: FIND AND END WHOEVER CAME UP WITH AND DOCUMENTED THIS TRASHCAN API
void SetClientWindowDimensions(vec2 dimensions)
{
	WINDOWINFO windowInfo;
	GetWindowInfo(windowHandle, &windowInfo);

	RECT curPos;
	GetWindowRect(windowHandle, &curPos);

	RECT newDimensions = {0};
	newDimensions.right = (I32)dimensions.x;
	newDimensions.bottom = (I32)dimensions.y;
	AdjustWindowRect(&newDimensions, windowInfo.dwStyle, false);
	
	vec2 d;
	d.x = (F32)(newDimensions.right - newDimensions.left);
	d.y = (F32)(newDimensions.bottom - newDimensions.top);

	SetWindowPos(
		windowHandle,
		NULL,
		curPos.left,
		curPos.top,
		(I32)d.x,
		(I32)d.y,
		NULL
		);

}

char* GetWindowTitle()
{
#define TitleMaxLength 256
	char* result = (char*)malloc(sizeof(char) * TitleMaxLength);

	GetWindowText( windowHandle, result, TitleMaxLength);

	return result;
}

void SetWindowTitle(const char* newTitle)
{
	SetWindowText( windowHandle, newTitle);
}






/*
 * TIME API IMPLEMENTATION
 */
SystemTime GetTimeSinceStartup()
{
	SystemTime result;

	FILETIME tempFileTime;
	GetSystemTimeAsFileTime(&tempFileTime);
	ULARGE_INTEGER currentTime;
	currentTime.HighPart = tempFileTime.dwHighDateTime;
	currentTime.LowPart = tempFileTime.dwLowDateTime;
	ULARGE_INTEGER diffTime;
	diffTime.QuadPart = currentTime.QuadPart - applicationStartTime.QuadPart;
	tempFileTime.dwHighDateTime = diffTime.HighPart;
	tempFileTime.dwLowDateTime = diffTime.LowPart;
	SYSTEMTIME timeDiffSystemTime;
	FileTimeToSystemTime(&tempFileTime, &timeDiffSystemTime);

	result.hours = timeDiffSystemTime.wHour;
	result.minutes = timeDiffSystemTime.wMinute;
	result.seconds = timeDiffSystemTime.wSecond;
	result.milliseconds = timeDiffSystemTime.wMilliseconds;

	return result;
}



/*
*  HIGH RESOLUTION TIMER API
*/
HighResolutionTimer::HighResolutionTimer(const char* name, size_t indentLevel)
{
	this->name = Copy(name);

	size_t numberOfIndentCharacters = 3 * indentLevel;
	this->indentString = DuplicateCharacter(numberOfIndentCharacters, ' ');
}

HighResolutionTimer::~HighResolutionTimer()
{
	free((char*)this->name);
	free((char*)this->indentString);
}

void HighResolutionTimer::Start()
{
	LARGE_INTEGER startCycleLI;
	QueryPerformanceCounter(&startCycleLI);

	this->startCycle = startCycleLI.QuadPart;
}

void HighResolutionTimer::End()
{
	LARGE_INTEGER endCycleLI;
	QueryPerformanceCounter(&endCycleLI);

	this->endCycle = endCycleLI.QuadPart;
}

I64 HighResolutionTimer::GetCycles()
{
	I64 result;

	result = this->endCycle - this->startCycle;

	return result;
}

F32 HighResolutionTimer::GetMS()
{
	F32 result;


	LARGE_INTEGER qpf;
	QueryPerformanceFrequency(&qpf);
	I64 performanceFrequency = (I64)qpf.QuadPart;
	result = (this->GetCycles() / (F32)performanceFrequency) * 1000.0f;

	return result;
}

void HighResolutionTimer::Report()
{
	//DebugPrintf(1024, "Timer %s: %f  %u\n", this->name, this->GetMS(), this->GetCycles());
	DebugPrintf(1024, "%sTimer %s: %llicycles,   %fms\n", this->indentString, this->name, this->GetCycles(), this->GetMS());
}




/*
	INPUT API IMPLEMENTATION
 */

// Keyboard
bool GetKeyDown(KeyCode code)
{
	bool result = IsBitSet(&inputBuffer, code) && !IsBitSet(&inputBackBuffer, code);
	return result;
}

bool GetKey(KeyCode code)
{
	bool result = IsBitSet(&inputBuffer, code);
	return result;
}

bool GetKeyUp(KeyCode code)
{
	bool result = !IsBitSet(&inputBuffer, code) && IsBitSet(&inputBackBuffer, code);
	return result;
}



// Mouse
vec2 GetMousePosition()
{
	POINT cursorPosScreenSpace;
	RECT windowPos;
	RECT windowWidth;

	GetCursorPos(&cursorPosScreenSpace);
	GetWindowRect(windowHandle, &windowPos);
	GetClientRect(windowHandle, &windowWidth);

	I32 ncOffsetX = ((windowPos.right - windowPos.left) - windowWidth.right)/2;
	I32 ncOffsetY = (windowPos.bottom - windowPos.top) - windowWidth.bottom - ncOffsetX;

	vec2 result = vec2(cursorPosScreenSpace.x - windowPos.left - ncOffsetX, cursorPosScreenSpace.y - windowPos.top - ncOffsetY);
	return result;
}

vec2 GetMouseMovement()
{
	return hdMouseMovement;
}

bool GetMouseButtonDown(MouseCode code)
{
	bool result = IsBitSet(&inputBuffer, code) && !IsBitSet(&inputBackBuffer, code);
	return result;
}

bool GetMouseButton(MouseCode code)
{
	bool result = IsBitSet(&inputBuffer, code);
	return result;
}

bool GetMouseButtonUp(MouseCode code)
{
	bool result = !IsBitSet(&inputBuffer, code) && IsBitSet(&inputBackBuffer, code);
	return result;
}

void SetMousePosition(vec2 newPos)
{
	SetCursorPos((I32)newPos.x, (I32)newPos.y);
}

void SetCursorVisibility(bool newVis)
{
	if (newVis != cursorVisibility)
	{
		cursorVisibility = newVis;
		ShowCursor(cursorVisibility);
	}
}



// Controller
bool IsGamepadConnected()
{
	return gamepad.connected;
}

bool GetGamepadButtonDown(GamepadCode code)
{
	bool result = IsBitSet(&gamepad.buttons, code) && !IsBitSet(&backGamepad.buttons, code);
	return result;
}

bool GetGamepadButton(GamepadCode code)
{
	bool result = IsBitSet(&gamepad.buttons, code);
	return result;
}

bool GetGamepadButtonUp(GamepadCode code)
{
	bool result = !IsBitSet(&gamepad.buttons, code) && IsBitSet(&backGamepad.buttons, code);
	return result;
}

F32  GetGamepadLeftTrigger()
{
	return gamepad.leftTrigger;
}

F32  GetGamepadRightTrigger()
{
	return gamepad.rightTrigger;
}

vec2 GetGamepadLeftStick()
{
	return gamepad.leftThumbstick;
}

vec2 GetGamepadRightStick()
{
	return gamepad.rightThumbstick;
}



// Input API utility functions
static KeyCode TranslateVKCodeToKeyCode(UINT_PTR vkCode)
{
	KeyCode code;
	switch (vkCode)
	{

	case 0x30:
	code = KeyCode_0;
	break;

	case 0x31:
	code = KeyCode_1;
	break;

	case 0x32:
	code = KeyCode_2;
	break;

	case 0x33:
	code = KeyCode_3;
	break;

	case 0x34:
	code = KeyCode_4;
	break;

	case 0x35:
	code = KeyCode_5;
	break;

	case 0x36:
	code = KeyCode_6;
	break;

	case 0x37:
	code = KeyCode_7;
	break;

	case 0x38:
	code = KeyCode_8;
	break;

	case 0x39:
	code = KeyCode_9;
	break;

	case 0x41:
	code = KeyCode_A;
	break;

	case 0x42:
	code = KeyCode_B;
	break;

	case 0x43:
	code = KeyCode_C;
	break;

	case 0x44:
	code = KeyCode_D;
	break;

	case 0x45:
	code = KeyCode_E;
	break;

	case 0x46:
	code = KeyCode_F;
	break;

	case 0x47:
	code = KeyCode_G;
	break;

	case 0x48:
	code = KeyCode_H;
	break;

	case 0x49:
	code = KeyCode_I;
	break;

	case 0x4A:
	code = KeyCode_J;
	break;

	case 0x4B:
	code = KeyCode_K;
	break;

	case 0x4C:
	code = KeyCode_L;
	break;

	case 0x4D:
	code = KeyCode_M;
	break;

	case 0x4E:
	code = KeyCode_N;
	break;

	case 0x4F:
	code = KeyCode_O;
	break;

	case 0x50:
	code = KeyCode_P;
	break;

	case 0x51:
	code = KeyCode_Q;
	break;

	case 0x52:
	code = KeyCode_R;
	break;

	case 0x53:
	code = KeyCode_S;
	break;

	case 0x54:
	code = KeyCode_T;
	break;

	case 0x55:
	code = KeyCode_U;
	break;

	case 0x56:
	code = KeyCode_V;
	break;

	case 0x57:
	code = KeyCode_W;
	break;

	case 0x58:
	code = KeyCode_X;
	break;

	case 0x59:
	code = KeyCode_Y;
	break;

	case 0x5A:
	code = KeyCode_Z;
	break;

	case VK_OEM_MINUS:
	code = KeyCode_Minus;
	break;

	case VK_OEM_PLUS:
	code = KeyCode_Equal;
	break;

	case VK_OEM_4:
	code = KeyCode_LeftBracket;
	break;

	case VK_OEM_6:
	code = KeyCode_RightBracket;
	break;

	case VK_OEM_1:
	code = KeyCode_SemiColon;
	break;

	case VK_OEM_7: // NOTE: Why?
	code = KeyCode_BackSlash;
	break;

	case VK_OEM_COMMA:
	code = KeyCode_Comma;
	break;

	case VK_OEM_PERIOD:
	code = KeyCode_Period;
	break;

	case VK_OEM_2:
	code = KeyCode_ForwardSlash;
	break;

	case VK_LEFT:
	code = KeyCode_Left;
	break;

	case VK_UP:
	code = KeyCode_Up;
	break;

	case VK_RIGHT:
	code = KeyCode_Right;
	break;

	case VK_DOWN:
	code = KeyCode_Down;
	break;

	case VK_SPACE:
	code = KeyCode_Spacebar;
	break;

	case VK_OEM_3: // NOTE: why?
	code = KeyCode_Quote;
	break;

	default:
	code = KeyCode_NULL;
	break;
	}
	return code;
}

static void QueryController()
{
	if (GetGamepadButtonDown(GamepadCode_A))
	{
		DebugPrint("Gamepad A Down\n");
	}
	if (GetGamepadButtonUp(GamepadCode_A))
	{
		DebugPrint("Gamepad A Up\n");
	}
	if (GetGamepadButtonDown(GamepadCode_B))
	{
		DebugPrint("Gamepad B Down\n");
	}
	if (GetGamepadButtonUp(GamepadCode_B))
	{
		DebugPrint("Gamepad B Up\n");
	}
	if (GetGamepadButtonDown(GamepadCode_X))
	{
		DebugPrint("Gamepad X Down\n");
	}
	if (GetGamepadButtonUp(GamepadCode_X))
	{
		DebugPrint("Gamepad X Up\n");
	}
	if (GetGamepadButtonDown(GamepadCode_Y))
	{
		DebugPrint("Gamepad Y Down\n");
	}
	if (GetGamepadButtonUp(GamepadCode_Y))
	{
		DebugPrint("Gamepad Y Up\n");
	}
	if (GetGamepadButtonDown(GamepadCode_Up))
	{
		DebugPrint("Gamepad up Down\n");
	}
	if (GetGamepadButtonUp(GamepadCode_Up))
	{
		DebugPrint("Gamepad up Up\n");
	}
	if (GetGamepadButtonDown(GamepadCode_Right))
	{
		DebugPrint("Gamepad right Down\n");
	}
	if (GetGamepadButtonUp(GamepadCode_Right))
	{
		DebugPrint("Gamepad right Up\n");
	}
	if (GetGamepadButtonDown(GamepadCode_Down))
	{
		DebugPrint("Gamepad down Down\n");
	}
	if (GetGamepadButtonUp(GamepadCode_Down))
	{
		DebugPrint("Gamepad down Up\n");
	}
	if (GetGamepadButtonDown(GamepadCode_Left))
	{
		DebugPrint("Gamepad left Down\n");
	}
	if (GetGamepadButtonUp(GamepadCode_Left))
	{
		DebugPrint("Gamepad left Up\n");
	}

	if (GetGamepadButtonDown(GamepadCode_Start))
	{
		DebugPrint("Gamepad start Down\n");
	}
	if (GetGamepadButtonUp(GamepadCode_Start))
	{
		DebugPrint("Gamepad start Up\n");
	}
	if (GetGamepadButtonDown(GamepadCode_Back))
	{
		DebugPrint("Gamepad Back Down\n");
	}
	if (GetGamepadButtonUp(GamepadCode_Back))
	{
		DebugPrint("Gamepad Back Up\n");
	}

	if (GetGamepadButtonDown(GamepadCode_L3))
	{
		DebugPrint("Gamepad L3 Down\n");
	}
	if (GetGamepadButtonUp(GamepadCode_L3))
	{
		DebugPrint("Gamepad L3 Up\n");
	}
	if (GetGamepadButtonDown(GamepadCode_R3))
	{
		DebugPrint("Gamepad R3 Down\n");
	}
	if (GetGamepadButtonUp(GamepadCode_R3))
	{
		DebugPrint("Gamepad R3 Up\n");
	}

	if (GetGamepadButtonDown(GamepadCode_RightBumper))
	{
		DebugPrint("Gamepad rb Down\n");
	}
	if (GetGamepadButtonUp(GamepadCode_RightBumper))
	{
		DebugPrint("Gamepad rb Up\n");
	}
	if (GetGamepadButtonDown(GamepadCode_LeftBumper))
	{
		DebugPrint("Gamepad lb Down\n");
	}
	if (GetGamepadButtonUp(GamepadCode_LeftBumper))
	{
		DebugPrint("Gamepad lb Up\n");
	}

	if (gamepad.leftTrigger)
	{
		DebugPrintf(256, "Left Trigger: %f\n", gamepad.leftTrigger);
	}

	if (gamepad.rightTrigger)
	{
		DebugPrintf(256, "Right Trigger: %f\n", gamepad.rightTrigger);
	}


	if (gamepad.leftThumbstick != vec2(0, 0))
	{
		DebugPrintf(256, "Left Stick: (%f, %f)\n", gamepad.leftThumbstick.x, gamepad.leftThumbstick.y);
	}

	if (gamepad.rightThumbstick != vec2(0, 0))
	{
		DebugPrintf(256, "Right Stick: (%f, %f)\n", gamepad.rightThumbstick.x, gamepad.rightThumbstick.y);
	}
}






/*
  File API Implementation
 */

bool CreateFile(const char* filename, bool overwriteIfExists)
{
	bool result = true;
	DWORD creationDisposition = (overwriteIfExists) ? CREATE_ALWAYS : CREATE_NEW;
	HANDLE fileHandle = CreateFile(
		filename,
		GENERIC_READ,
		0,
		NULL,
		creationDisposition,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		result = false;
	}

	CloseHandle(fileHandle);

	return result;
}

GetFileSizeReturnType GetFileSize(const char* filename)
{
	GetFileSizeReturnType result = {0};
	HANDLE fileHandle = CreateFile(
		filename,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		result.fileExists = false;
	}
	else
	{
		LARGE_INTEGER fileSizeLI;
		GetFileSizeEx(fileHandle, &fileSizeLI);
		result.fileSize = (size_t)fileSizeLI.QuadPart;
		result.fileExists = true;
	}
	CloseHandle(fileHandle);

	return result;
}

// NOTE: casting down U64 to U32 
//		if huge files need to be supported this actually needs to be dealt with.
ReadFileReturnType ReadFile(const char* filename, char* fileBuffer, U64 numberOfBytesToRead, U64 readPosition)
{
	ReadFileReturnType result = {0};

	HANDLE fileHandle = CreateFile(
		filename,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		result.numberOfBytesRead = 0;
		result.errorEncountered = true;
	}
	else
	{
		DWORD numBytesRead = 0;
		DWORD* offsetList = (DWORD*)&readPosition;
		OVERLAPPED ol = {0};
		ol.OffsetHigh = *(offsetList + 1);
		ol.Offset = *(offsetList + 0);
		ReadFile(fileHandle, fileBuffer, (DWORD)numberOfBytesToRead, &numBytesRead, &ol);
		result.numberOfBytesRead = (U64)numBytesRead;
		result.errorEncountered = false;
	}
	CloseHandle(fileHandle);

	return result;
}

// NOTE: casting down U64 to U32 
//		if huge files need to be supported this actually needs to be dealt with.
WriteFileReturnType WriteFileconst (char* filename, const char* fileBuffer, U64 numberOfBytesToWrite, U64 writePosition)
{
	WriteFileReturnType result = {0};

	HANDLE fileHandle = CreateFile(
		filename,
		GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		result.numberOfBytesWritten = 0;
		result.errorEncountered = true;
	}
	else
	{
		DWORD numBytesWritten = 0;
		DWORD* offsetList = (DWORD*)&writePosition;
		OVERLAPPED ol = { 0 };
		ol.OffsetHigh = *(offsetList + 1);
		ol.Offset = *(offsetList + 0);
		WriteFile(fileHandle, fileBuffer, (DWORD)numberOfBytesToWrite, &numBytesWritten, &ol);
		result.numberOfBytesWritten = (U64)numBytesWritten;
		result.errorEncountered = false;
	}
	CloseHandle(fileHandle);

	return result;
}






/*
	Debug API
*/
void Assert(int expression)
{
	assert(expression);
}

void DebugPrint(const char* outputString)
{
	OutputDebugString(outputString);
}

void DebugPrintf(U32 size, const char* formatString, ...)
{
	va_list vl;
	va_start(vl, formatString);
	char* buff = (char*)malloc(sizeof(char) * size);
	vsprintf_s(buff, size, formatString, vl);
	OutputDebugString(buff);
	va_end(vl);
	free(buff);
}

void SystemSleep(U32 milliseconds)
{
	Sleep(milliseconds);
}