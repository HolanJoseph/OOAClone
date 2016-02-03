#include <windows.h>
#include "Types.h"



LRESULT CALLBACK Win32WindowCallback( HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam )    
{
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
	return 0;
}