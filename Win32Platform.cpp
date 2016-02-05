#include <windows.h>
#include <stdio.h>

//#define NDEBUG
#include <cassert>

#include "Types.h"


U8 inputBuffer;
U8 inputBackBuffer;
enum InputCode
{
	InputCode_W = 1 << 0,
	InputCode_A = 1 << 1,
	InputCode_S = 1 << 2,
	InputCode_D = 1 << 3
};

void setDown(U8* buffer, InputCode code)
{
	(*buffer) = (*buffer) | code;
}

void setUp(U8* buffer, InputCode code)
{
	(*buffer) = (*buffer) & ~code;
}

bool checkDown(U8* buffer, InputCode code)
{
	bool result = ((*buffer) >> code) & 1;
	return result;
}

bool getKeyDown(U8* buffer, U8* backBuffer, InputCode code)
{
	bool result = checkDown(buffer, code) && !checkDown(backBuffer, code);
	return result;
}

bool getKey(U8* buffer, InputCode code)
{
	bool result = checkDown(buffer, code);
	return result;
}

bool getKeyUp(U8* buffer, U8* backBuffer, InputCode code)
{
	bool result = !checkDown(buffer, code) && checkDown(backBuffer, code);
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
		OutputDebugString("WM_NCCREATE\n");
		// NOTE: icon, min, max, close already present
		// NOTE: Shows the window title
		return DefWindowProc(windowHandle, message, wParam, lParam);
		break;
	
	case WM_CREATE:
		OutputDebugString("WM_CREATE\n");
		// NOTE: no visual cue as to what this does
		return DefWindowProc(windowHandle, message, wParam, lParam);
		break;
	
	case WM_CLOSE:
		OutputDebugString("WM_CLOSE\n");
		// NOTE: message sent when the close button is clicked 
		// NOTE: consider prompting the user for confirmation prior to destroying a window
		// NOTE: sends WM_DESTROY message to the app
		// NOTE: Is this correct? I cannot seem to find any documentation on what WM_CLOSE should return on success
		return DestroyWindow(windowHandle); 
		break;
	
	case WM_DESTROY:
		OutputDebugString("WM_DESTROY\n");
		PostQuitMessage(0);
		return 0;
		break;
	
	case WM_NCDESTROY:
		OutputDebugString("WM_NCDESTROY\n");
		// NOTE: Why do we get this message?
		return DefWindowProc(windowHandle, message, wParam, lParam);
		break;

	case WM_QUIT:
		// NOTE: This message is never received by the wndproc as it is intercepted by the system.
		break;



	/*
		Window show state messages

		WM_ACTIVATEAPP
		WM_ENABLE
		WM_QUERYOPEN
		WM_SHOWWINDOW
	*/



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

	

	default:
		return DefWindowProc(windowHandle, message, wParam, lParam);
		break;
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


	inputBuffer = 0;
	inputBackBuffer = 0;


	setDown(&inputBuffer, InputCode_W);
	setDown(&inputBuffer, InputCode_D);
	// run
	inputBackBuffer = inputBuffer;
	inputBuffer = inputBuffer;

	setDown(&inputBuffer, InputCode_S);
	//run
	inputBackBuffer = inputBuffer;
	inputBuffer = inputBuffer;

	setUp(&inputBuffer, InputCode_W);
	// run
	inputBackBuffer = inputBuffer;
	inputBuffer = inputBuffer;


	bool running = true;
	MSG windowsMessage;
	while (running)
	{
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
	}

	return 0;
}