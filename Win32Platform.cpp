#include <windows.h>
#include <Xinput.h>
#include "glew/GL/glew.h"
#include "glew/GL/wglew.h"
#include <gl/GL.h>

#include <stdio.h>
//#define NDEBUG
#include <cassert>

#include "Types.h"
#include "Math.h"
#include "BitManip.h"
#include "Util.h"


// NOTE: Functions from these files must be implemented in this file
#include "InputAPI.h"
#include "FileAPI.h"





struct verifyShaderReturnResult
{
	bool compiled;
	char* infoLog;
	I64 infoLogLength;

};
verifyShaderReturnResult verifyShader(GLuint shader);






// NOTE: Windows stuff
HWND  windowHandle;
HDC   windowDC;
HGLRC windowOpenGLContext;

// NOTE: Input globals
U16 inputBuffer;
U16 inputBackBuffer;

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

KeyCode translateVKCodeToKeyCode(UINT_PTR vkCode);
void queryController();


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
							   // Capture mouse
							   //SetCapture(windowHandle);

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
							   //ReleaseCapture();
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
						  setBit(&inputBuffer, translateVKCodeToKeyCode(wParam), 1);
						  return 0;
						  break;
	}

	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
						setBit(&inputBuffer, translateVKCodeToKeyCode(wParam), 0);
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
						   setBit(&inputBuffer, MouseCode_Left, 1);
						   return 0;
						   break;
	}
	case WM_LBUTTONUP:
	{
						 setBit(&inputBuffer, MouseCode_Left, 0);
						 return 0;
						 break;
	}

	case WM_MBUTTONDOWN:
	{
						   setBit(&inputBuffer, MouseCode_Middle, 1);
						   return 0;
						   break;
	}
	case WM_MBUTTONUP:
	{
						 setBit(&inputBuffer, MouseCode_Middle, 0);
						 return 0;
						 break;
	}

	case WM_RBUTTONDOWN:
	{
						   setBit(&inputBuffer, MouseCode_Right, 1);
						   return 0;
						   break;
	}
	case WM_RBUTTONUP:
	{
						 setBit(&inputBuffer, MouseCode_Right, 0);
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
		OutputDebugString("Was not able to register the window class.");
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
		char fuckglew[512];
		sprintf_s(fuckglew, 512, "GLEW Error: %s", glewGetErrorString(error));
		OutputDebugString(fuckglew);
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
		OutputDebugString("GLEW could not support creation of a higher level OpenGL context");
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
	char glbuff[256];
	sprintf_s(glbuff, 256, "GL Version %i.%i\nIs doublebuffered: %i", glMajor, glMinor, glDoublebuffered);
	OutputDebugString(glbuff);

	F32 r = 0.32f;
	F32 g = 0.18f;
	F32 b = 0.66f;
	glClearColor(r, g, b, 0.0f);

	GLuint VAOs[1];
	GLuint Buffers[1];
	const GLuint numVertices = 6;
	glGenVertexArrays(1, VAOs);
	glBindVertexArray(VAOs[0]);
	GLfloat vertices[numVertices][2] =
	{
		{ -0.90f, -0.90f },
		{  0.85f, -0.90f },
		{ -0.90f,  0.85f },
		{  0.90f, -0.85f },
		{  0.90f,  0.90f },
		{ -0.85f,  0.90f }
	};
	glGenBuffers(1, Buffers);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	char* vertexShaderFile = "triangles.vert";
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	U64 vertexShaderFileSize = GetFileSize(vertexShaderFile).fileSize;
	char* vertexShaderSource = (char *)malloc(sizeof(char) * vertexShaderFileSize);
	const GLint glSizeRead = ReadFile(vertexShaderFile, vertexShaderSource, vertexShaderFileSize).numberOfBytesRead;
	glShaderSource(vertexShader, 1, &vertexShaderSource, &glSizeRead);
	glCompileShader(vertexShader);
	verifyShaderReturnResult vertexVerification = verifyShader(vertexShader);
	if (!vertexVerification.compiled)
	{
		OutputDebugString(vertexVerification.infoLog);
		return 1;
	}

	char* fragmentShaderFile = "triangles.frag";
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	U64 fragmentShaderFileSize = GetFileSize(fragmentShaderFile).fileSize;
	char* fragmentShaderSource = (char *)malloc(sizeof(char) * fragmentShaderFileSize);
	const GLint glFragmentShaderSize = ReadFile(fragmentShaderFile, fragmentShaderSource, fragmentShaderFileSize).numberOfBytesRead;
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, &glFragmentShaderSize);
	glCompileShader(fragmentShader);
	verifyShaderReturnResult fragmentVerification = verifyShader(fragmentShader);
	if (!fragmentVerification.compiled)
	{
		OutputDebugString(fragmentVerification.infoLog);
		return 1;
	}

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	GLint programCompileStatus = 0;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &programCompileStatus);
	if (programCompileStatus != GL_TRUE)
	{
		GLint infoLogLength = 0;
		glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLsizei returnedInfoLogLength = 0;
		GLchar* infoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
		glGetProgramInfoLog(shaderProgram, infoLogLength, &returnedInfoLogLength, infoLog);
		OutputDebugString(infoLog);
		return 1;
	}
	glUseProgram(shaderProgram);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);



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
		OutputDebugString("Was not able to set up raw mouse input");
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
		OutputDebugString("Was not able to set timer granularity");
		return 1;
	}

	
	LARGE_INTEGER qpf;
	QueryPerformanceFrequency(&qpf);
	I64 performanceFrequency = (I64)qpf.QuadPart; // This is number of counts per second

	LARGE_INTEGER frameStartLI;
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
			setBit(&gamepad.buttons, GamepadCode_Up,    (U8)clampRange((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP), 0, 1));
			setBit(&gamepad.buttons, GamepadCode_Down,  (U8)clampRange((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN), 0, 1));
			setBit(&gamepad.buttons, GamepadCode_Left,  (U8)clampRange((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT), 0, 1));
			setBit(&gamepad.buttons, GamepadCode_Right, (U8)clampRange((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT), 0, 1));
			setBit(&gamepad.buttons, GamepadCode_Start, (U8)clampRange((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_START), 0, 1));
			setBit(&gamepad.buttons, GamepadCode_Back,  (U8)clampRange((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK), 0, 1));
			setBit(&gamepad.buttons, GamepadCode_X, (U8)clampRange((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_X), 0, 1));
			setBit(&gamepad.buttons, GamepadCode_Y, (U8)clampRange((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_Y), 0, 1));
			setBit(&gamepad.buttons, GamepadCode_A, (U8)clampRange((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_A), 0, 1));
			setBit(&gamepad.buttons, GamepadCode_B, (U8)clampRange((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_B), 0, 1));
			setBit(&gamepad.buttons, GamepadCode_RightBumper, (U8)clampRange((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER), 0, 1));
			setBit(&gamepad.buttons, GamepadCode_R3, (U8)clampRange((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB), 0, 1));
			setBit(&gamepad.buttons, GamepadCode_LeftBumper, (U8)clampRange((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER), 0, 1));
			setBit(&gamepad.buttons, GamepadCode_L3, (U8)clampRange((F32)(controllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB), 0, 1));

			// Thumb sticks
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
			gamepad.connected = false;
		}

		

		queryController();
		vec2 mp = getMousePosition();
		char mpbuff[512];
		sprintf_s(mpbuff, 512, "mousepos %f, %f\n", mp.x, mp.y);
		//OutputDebugString(mpbuff);

		//Sleep(16);

		glViewport(0, 0, 600, 500);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glBindVertexArray(VAOs[0]);
		glDrawArrays(GL_TRIANGLES, 0, numVertices);

		glFlush(); // NOTE: Necessary???
		SwapBuffers(windowDC);

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

	
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteProgram(shaderProgram);

	wglMakeCurrent(windowDC, NULL);
	wglDeleteContext(windowOpenGLContext);
	ReleaseDC(windowHandle, windowDC);

	return 0;
}






verifyShaderReturnResult verifyShader(GLuint shader)
{
	verifyShaderReturnResult result = {0};

	GLint shaderCompileStatus = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompileStatus);
	if (shaderCompileStatus != GL_TRUE)
	{
		GLint infoLogLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLsizei returnedInfoLogLength = 0;
		GLchar *infoLog = (GLchar *)malloc(sizeof(GLchar)* infoLogLength);
		glGetShaderInfoLog(shader, infoLogLength, &returnedInfoLogLength, infoLog);

		result.compiled = false;
		result.infoLog = infoLog;
		result.infoLogLength = returnedInfoLogLength;
	}
	else
	{
		result.compiled = true;
		result.infoLog = NULL;
		result.infoLogLength = 0;
	}

	return result;
}





/*
	INPUT API IMPLEMENTATION
 */

// Keyboard
bool getKeyDown(KeyCode code)
{
	bool result = isBitSet(&inputBuffer, code) && !isBitSet(&inputBackBuffer, code);
	return result;
}

bool getKey(KeyCode code)
{
	bool result = isBitSet(&inputBuffer, code);
	return result;
}

bool getKeyUp(KeyCode code)
{
	bool result = !isBitSet(&inputBuffer, code) && isBitSet(&inputBackBuffer, code);
	return result;
}



// Mouse
vec2 getMousePosition()
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

vec2 getMouseMovement()
{
	return hdMouseMovement;
}

bool getMouseButtonDown(MouseCode code)
{
	bool result = isBitSet(&inputBuffer, code) && !isBitSet(&inputBackBuffer, code);
	return result;
}

bool getMouseButton(MouseCode code)
{
	bool result = isBitSet(&inputBuffer, code);
	return result;
}

bool getMouseButtonUp(MouseCode code)
{
	bool result = !isBitSet(&inputBuffer, code) && isBitSet(&inputBackBuffer, code);
	return result;
}

void setMousePosition(vec2 newPos)
{
	SetCursorPos((I32)newPos.x, (I32)newPos.y);
}

void setCursorVisibility(bool newVis)
{
	if (newVis != cursorVisibility)
	{
		cursorVisibility = newVis;
		ShowCursor(cursorVisibility);
	}
}



// Controller
bool isGamepadConnected()
{
	return gamepad.connected;
}

bool getGamepadButtonDown(GamepadCode code)
{
	bool result = isBitSet(&gamepad.buttons, code) && !isBitSet(&backGamepad.buttons, code);
	return result;
}

bool getGamepadButton(GamepadCode code)
{
	bool result = isBitSet(&gamepad.buttons, code);
	return result;
}

bool getGamepadButtonUp(GamepadCode code)
{
	bool result = !isBitSet(&gamepad.buttons, code) && isBitSet(&backGamepad.buttons, code);
	return result;
}

F32  getGamepadLeftTrigger()
{
	return gamepad.leftTrigger;
}

F32  getGamepadRightTrigger()
{
	return gamepad.rightTrigger;
}

vec2 getGamepadLeftStick()
{
	return gamepad.leftThumbstick;
}

vec2 getGamepadRightStick()
{
	return gamepad.rightThumbstick;
}



// Input API utility functions
KeyCode translateVKCodeToKeyCode(UINT_PTR vkCode)
{
	KeyCode code;
	switch (vkCode)
	{
	case 0x57:
	code = KeyCode_W;
	break;

	case 0x41:
	code = KeyCode_A;
	break;

	case 0x53:
	code = KeyCode_S;
	break;

	case 0x44:
	code = KeyCode_D;
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

	default:
	code = KeyCode_NULL;
	break;
	}
	return code;
}

void queryController()
{
	if (getGamepadButtonDown(GamepadCode_A))
	{
		OutputDebugString("Gamepad A Down\n");
	}
	if (getGamepadButtonUp(GamepadCode_A))
	{
		OutputDebugString("Gamepad A Up\n");
	}
	if (getGamepadButtonDown(GamepadCode_B))
	{
		OutputDebugString("Gamepad B Down\n");
	}
	if (getGamepadButtonUp(GamepadCode_B))
	{
		OutputDebugString("Gamepad B Up\n");
	}
	if (getGamepadButtonDown(GamepadCode_X))
	{
		OutputDebugString("Gamepad X Down\n");
	}
	if (getGamepadButtonUp(GamepadCode_X))
	{
		OutputDebugString("Gamepad X Up\n");
	}
	if (getGamepadButtonDown(GamepadCode_Y))
	{
		OutputDebugString("Gamepad Y Down\n");
	}
	if (getGamepadButtonUp(GamepadCode_Y))
	{
		OutputDebugString("Gamepad Y Up\n");
	}
	if (getGamepadButtonDown(GamepadCode_Up))
	{
		OutputDebugString("Gamepad up Down\n");
	}
	if (getGamepadButtonUp(GamepadCode_Up))
	{
		OutputDebugString("Gamepad up Up\n");
	}
	if (getGamepadButtonDown(GamepadCode_Right))
	{
		OutputDebugString("Gamepad right Down\n");
	}
	if (getGamepadButtonUp(GamepadCode_Right))
	{
		OutputDebugString("Gamepad right Up\n");
	}
	if (getGamepadButtonDown(GamepadCode_Down))
	{
		OutputDebugString("Gamepad down Down\n");
	}
	if (getGamepadButtonUp(GamepadCode_Down))
	{
		OutputDebugString("Gamepad down Up\n");
	}
	if (getGamepadButtonDown(GamepadCode_Left))
	{
		OutputDebugString("Gamepad left Down\n");
	}
	if (getGamepadButtonUp(GamepadCode_Left))
	{
		OutputDebugString("Gamepad left Up\n");
	}

	if (getGamepadButtonDown(GamepadCode_Start))
	{
		OutputDebugString("Gamepad start Down\n");
	}
	if (getGamepadButtonUp(GamepadCode_Start))
	{
		OutputDebugString("Gamepad start Up\n");
	}
	if (getGamepadButtonDown(GamepadCode_Back))
	{
		OutputDebugString("Gamepad Back Down\n");
	}
	if (getGamepadButtonUp(GamepadCode_Back))
	{
		OutputDebugString("Gamepad Back Up\n");
	}

	if (getGamepadButtonDown(GamepadCode_L3))
	{
		OutputDebugString("Gamepad L3 Down\n");
	}
	if (getGamepadButtonUp(GamepadCode_L3))
	{
		OutputDebugString("Gamepad L3 Up\n");
	}
	if (getGamepadButtonDown(GamepadCode_R3))
	{
		OutputDebugString("Gamepad R3 Down\n");
	}
	if (getGamepadButtonUp(GamepadCode_R3))
	{
		OutputDebugString("Gamepad R3 Up\n");
	}

	if (getGamepadButtonDown(GamepadCode_RightBumper))
	{
		OutputDebugString("Gamepad rb Down\n");
	}
	if (getGamepadButtonUp(GamepadCode_RightBumper))
	{
		OutputDebugString("Gamepad rb Up\n");
	}
	if (getGamepadButtonDown(GamepadCode_LeftBumper))
	{
		OutputDebugString("Gamepad lb Down\n");
	}
	if (getGamepadButtonUp(GamepadCode_LeftBumper))
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


	if (gamepad.leftThumbstick != vec2(0, 0))
	{
		char buff[256];
		sprintf_s(buff, 256, "Left Stick: (%f, %f)\n", gamepad.leftThumbstick.x, gamepad.leftThumbstick.y);
		OutputDebugString(buff);
	}

	if (gamepad.rightThumbstick != vec2(0, 0))
	{
		char buff[256];
		sprintf_s(buff, 256, "Right Stick: (%f, %f)\n", gamepad.rightThumbstick.x, gamepad.rightThumbstick.y);
		OutputDebugString(buff);
	}
}






/*
  File API Implementation
 */

GetFileSizeReturnType GetFileSize(char* filename)
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
		result.fileSize = fileSizeLI.QuadPart;
		result.fileExists = true;
	}
	CloseHandle(fileHandle);

	return result;
}

ReadFileReturnType ReadFile(char* filename, char* fileBuffer, U64 numberOfBytesToRead, U64 readPosition)
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
		ReadFile(fileHandle, fileBuffer, numberOfBytesToRead, &numBytesRead, NULL);
		result.numberOfBytesRead = (U64)numBytesRead;
		result.errorEncountered = false;
	}
	CloseHandle(fileHandle);

	return result;
}