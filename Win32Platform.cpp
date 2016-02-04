#include <windows.h>
#include <stdio.h>

//#define NDEBUG
#include <cassert>

#include "Types.h"


LRESULT CALLBACK Win32WindowCallback( HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam )    
{
	switch (message)
	{
	/*
		Window lifetime messages.

		NOTE: Pass through cases are pulled out for consistency and later work
	*/
	case WM_NCCREATE:
		// NOTE: icon, min, max, close already present
		// NOTE: Shows the window title
		return DefWindowProc(windowHandle, message, wParam, lParam);
		break;
	
	case WM_CREATE:
		// NOTE: no visual cue as to what this does
		return DefWindowProc(windowHandle, message, wParam, lParam);
		break;
	
	case WM_CLOSE:
		// NOTE: message sent when the close button is clicked 
		// NOTE: consider prompting the user for confirmation prior to destroying a window
		// NOTE: sends WM_DESTROY message to the app
		// NOTE: Is this correct? I cannot seem to find any documentation on what WM_CLOSE should return on success
		return DestroyWindow(windowHandle); 
		break;
	
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		break;
	
	case WM_NCDESTROY:
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
	
	// NOTE: message loop replace with message pump
	MSG windowsMessage;
	while (GetMessage(&windowsMessage, NULL, 0, 0))
	{
		TranslateMessage(&windowsMessage);
		DispatchMessage(&windowsMessage);
	}

	return 0;
}