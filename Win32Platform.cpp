#include <windows.h>

//#define NDEBUG
#include <cassert>

#include "Types.h"


LRESULT CALLBACK Win32WindowCallback( HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam )    
{
	/* Window Messages to investigate

	   WM_NCCREATE
	   WM_CREATE
	   WM_CLOSE
	   WM_DESTROY
	   WM_NCDESTROY
	   WM_QUIT

	   WM_ACTIVATEAPP
	   WM_ENABLE
	   WM_QUERYOPEN
	   WM_SHOWWINDOW
	   
	   WM_SETICON
	   WM_SETTEXT
	   WM_SETFONT
	   WM_GETICON
	   WM_QUERYDRAGICON

	   WM_DPICHANGED
	   WM_MOVE
	   WM_SIZE
	   WM_SIZING
	   WM_WIONDOWPOSCHANGED
	   WM_WINDOWPOSCHANGING
	   WM_ENTERSIZEMOVE
	   WM_EXITSIZEMOVE
	*/

	switch (message)
	{
	/*
		Window lifetime messages.
	*/
	case WM_NCCREATE:
	OutputDebugString("WM_NCCREATE\n");
	return DefWindowProc(windowHandle, message, wParam, lParam);
	break;
	
	case WM_CREATE:
	OutputDebugString("WM_CREATE\n");
	return DefWindowProc(windowHandle, message, wParam, lParam);
	break;
	
	case WM_CLOSE:
	OutputDebugString("WM_CLOSE\n");
	return DefWindowProc(windowHandle, message, wParam, lParam);
	break;
	
	case WM_DESTROY:
	OutputDebugString("WM_DESTROY\n");
	return DefWindowProc(windowHandle, message, wParam, lParam);
	break;
	
	case WM_NCDESTROY:
	OutputDebugString("WM_NCDESTROY\n");
	PostQuitMessage(0);
	return DefWindowProc(windowHandle, message, wParam, lParam);
	break;

	case WM_QUIT:
	// NOTE: This message is never received by the wndproc as it is intercepted by the system.
	break;



	/*
		Window show state messages
	*/
	case WM_ACTIVATEAPP:
	OutputDebugString("WM_ACTIVEAPP");
	return DefWindowProc(windowHandle, message, wParam, lParam);
	break;
	
	case WM_ENABLE:
	OutputDebugString("WM_ENABLE");
	return DefWindowProc(windowHandle, message, wParam, lParam);
	break;

	case WM_QUERYOPEN:
	OutputDebugString("WM_QUERYOPEN");
	return DefWindowProc(windowHandle, message, wParam, lParam);
	break;

	case WM_SHOWWINDOW:
	OutputDebugString("WM_SHOWWINDOW");
	return DefWindowProc(windowHandle, message, wParam, lParam);
	break;



	/*
		Window resource messages
	*/
	case WM_SETICON:
	OutputDebugString("WM_SETICON");
	return DefWindowProc(windowHandle, message, wParam, lParam);
	break;

	case WM_SETTEXT:
	OutputDebugString("WM_SETTEXT");
	return DefWindowProc(windowHandle, message, wParam, lParam);
	break;

	case WM_SETFONT:
	OutputDebugString("WM_SETFONT");
	return DefWindowProc(windowHandle, message, wParam, lParam);
	break;

	case WM_GETICON:
	OutputDebugString("WM_GETICON");
	return DefWindowProc(windowHandle, message, wParam, lParam);
	break;

	case WM_QUERYDRAGICON:
	OutputDebugString("WM_QUERYDRAGICON");
	return DefWindowProc(windowHandle, message, wParam, lParam);
	break;



	/*
		Window sizing messages
	*/
	case WM_DPICHANGED:
	OutputDebugString("WM_DPICHANGED");
	return DefWindowProc(windowHandle, message, wParam, lParam);
	break;

	case WM_MOVE:
	OutputDebugString("WM_MOVE");
	return DefWindowProc(windowHandle, message, wParam, lParam);
	break;

	case WM_SIZE:
	OutputDebugString("WM_SIZE");
	return DefWindowProc(windowHandle, message, wParam, lParam);
	break;

	case WM_SIZING:
	OutputDebugString("WM_SIZING");
	return DefWindowProc(windowHandle, message, wParam, lParam);
	break;

	case WM_WINDOWPOSCHANGED:
	OutputDebugString("WM_WINDOWPOSCHANGED");
	return DefWindowProc(windowHandle, message, wParam, lParam);
	break;

	case WM_WINDOWPOSCHANGING:
	OutputDebugString("WM_WINDOWPOSCHANGING");
	return DefWindowProc(windowHandle, message, wParam, lParam);
	break;

	case WM_ENTERSIZEMOVE:
	OutputDebugString("WM_ENTERSIZEMOVE");
	return DefWindowProc(windowHandle, message, wParam, lParam);
	break;

	case WM_EXITSIZEMOVE:
	OutputDebugString("WM_EXITSIZEMOVE");
	return DefWindowProc(windowHandle, message, wParam, lParam);
	break;



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