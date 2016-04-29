#pragma once
#include <string>
#include <Windows.h>

struct MSWindowBinds
{
	HINSTANCE instanceHandle;
	std::string windowClassName;
	std::string windowTitle;
	HWND windowHandle;
};

MSWindowBinds CreateMSWindow(const std::string& title, unsigned int x, unsigned int y, unsigned int width, unsigned int height);
void DestroyMSWindow(MSWindowBinds& binds );
//pumps existing msgs then returns
void MessagePumpMSWindow(const MSWindowBinds& binds);
//dummy event handler
LRESULT CALLBACK EventHandleMSWindow(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);