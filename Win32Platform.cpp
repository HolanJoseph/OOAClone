#include <windows.h>
#include <stdio.h>

//#define NDEBUG
#include <cassert>

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
	InputCode_LeftMouse = 5,
	InputCode_RightMouse = 6
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

	case VK_LBUTTON:
	code = InputCode_LeftMouse;
	break;

	case VK_RBUTTON:
	code = InputCode_RightMouse;
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
							   I32 middleX = windowPos.left + ((windowPos.right - windowPos.left)/2.0f);
							   I32 middleY = windowPos.top + ((windowPos.bottom - windowPos.top)/2.0f);
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
	while (running)
	{
		// NOTE: Copy over last frames data, so key up/ key down can be queried
		inputBackBuffer = inputBuffer;

		// NOTE: Reset the HD mouse delta for the frame
		hdMouseDeltaX = 0;
		hdMouseDeltaY = 0;

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

		if (getKeyDown(InputCode_W))
		{
			ShowCursor(FALSE);
		}
		if (getKey(InputCode_W))
		{
		}
		if (getKeyUp(InputCode_W))
		{
			ShowCursor(TRUE);
		}

		char hdBuff[256];
		sprintf_s(hdBuff, 256, "hd movement per frame (%i, %i)\n", hdMouseDeltaX, hdMouseDeltaY);
		OutputDebugString(hdBuff);

		Sleep(16);
	}

	// 
	timeEndPeriod(1);

	return 0;
}