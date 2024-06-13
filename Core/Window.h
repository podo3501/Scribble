#pragma once

#include<wtypes.h>
#include<string>
#include"CoreInterface.h"

class CWindow
{
public:
	CWindow(HINSTANCE hInstanc);

	bool Initialize(WNDPROC wndProc);

public:
	CWindow(const CWindow&) = delete;
	CWindow& operator=(const CWindow&) = delete;

private:
	HINSTANCE m_appInst{ nullptr };
	HWND m_wnd{ nullptr };

	std::wstring m_caption = L"Directx3D App";

	int m_width{ 800 };
	int m_height{ 600 };
};