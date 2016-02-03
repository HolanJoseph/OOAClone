#include <windows.h>
#include "Types.h"



LRESULT CALLBACK Win32WindowCallback( HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam )    
{
	/* Window Messages to investigate

	   WM_NCCREATE
	   WM_CREATE
	   WM_QUIT
	   WM_DESTROY
	   WM_NCDESTROY
	   WM_CLOSE

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
	case WM_CREATE: // Initialize the window. 
	{
		return 0;
		break;
	}

	case WM_PAINT: // Paint the window's client area.
	{
		return 0;
		break;
	}

	case WM_SIZE: // Set the size and position of the window.
	{
		return 0;
		break;
	}

	case WM_DESTROY: // Clean up window-specific data objects. 
	{
		return 0;
		break;
	}

	// 
	// Process other messages. 
	// 

	default:
	{
		return DefWindowProc(windowHandle, message, wParam, lParam);
	}
	}

	return 0;
}



INT WINAPI WinMain(HINSTANCE instanceHandle, HINSTANCE deadArg, PSTR commandLineArgsUnicode, INT showOnStartup)
{
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
	windowClass.lpszClassName = "OracleOfAgesCloneMainWindow";
	windowClass.hIconSm = LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_APPLICATION));

	if (!RegisterClassEx(&windowClass))
	{
		OutputDebugString("Was not able to register the window class.");
		return 1;
	}

	I32 x;
	x = 5;

	return 0;
}