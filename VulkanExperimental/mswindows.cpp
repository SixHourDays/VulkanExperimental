#include "mswindows.h"

#include <iostream>

#include "utility.h"

MSWindowBinds CreateMSWindow(const std::string& title, unsigned int x, unsigned int y, unsigned int width, unsigned int height) 
{
	MSWindowBinds windowsBinds;
	windowsBinds.instanceHandle = GetModuleHandle(nullptr);
	windowsBinds.windowClassName = "DummyWindowClass";
	windowsBinds.windowTitle = title;

	//register window class type
	WNDCLASSEX windowClass;
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW; //causes redraw when width or height changes
	windowClass.lpfnWndProc = &EventHandleMSWindow;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = windowsBinds.instanceHandle;
	windowClass.hIcon = LoadIcon(nullptr, IDI_WINLOGO); //predefined win2000 icon
	windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW); //default arrow cursor
	windowClass.hbrBackground = (HBRUSH)(COLOR_MENU +1); //background color is window color
	windowClass.lpszMenuName = nullptr;
	windowClass.lpszClassName = windowsBinds.windowClassName.data();
	windowClass.hIconSm = LoadIcon(nullptr, IDI_WINLOGO);

	bool success = RegisterClassEx(&windowClass) != 0;
	assert(success, "mswindow registration failed");

	//extend window by titlebar and border sizes
	RECT rect = { 0, 0, (long)width, (long)height };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
	
	//instantiate window
	windowsBinds.windowHandle = CreateWindowEx(0,
		windowsBinds.windowClassName.data(),
		windowsBinds.windowTitle.data(),
		WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_SYSMENU,
		x, y,
		rect.right - rect.left,
		rect.bottom - rect.top,
		nullptr,               // handle to parent
		nullptr,               // handle to menu
		windowsBinds.instanceHandle,
		nullptr);              // no extra parameters
	assert(windowsBinds.windowHandle != 0, "mswindow creation failed");
	
	UpdateWindow(windowsBinds.windowHandle);

	return windowsBinds;
}

void DestroyMSWindow(MSWindowBinds& binds)
{
	bool ret = DestroyWindow(binds.windowHandle) == TRUE;
	assert(ret, "Failed to destroy window");

	binds.windowHandle = 0;
}

//pumps existing msgs then returns
void MessagePumpMSWindow(const MSWindowBinds& binds)
{
	MSG msg;
	while (PeekMessage(&msg, binds.windowHandle, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

//event handler has to match WNDPROC signature
LRESULT CALLBACK EventHandleMSWindow(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	LRESULT retVal = 0;

	switch (uMsg)
	{
	case WM_PAINT:
	{
		hdc = BeginPaint(hWnd, &ps);

		RECT rect;
		GetClientRect(hWnd, &rect);
		const char* dummyMsg = "woo";
		TextOut(hdc,
			(unsigned int)rand() % (rect.right - rect.left), //randomize coord
			(unsigned int)rand() % (rect.bottom - rect.top), //window cords are top y=0
			dummyMsg,
			(int)strlen(dummyMsg));

		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
	break;
	default:
		retVal = DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return retVal;
}