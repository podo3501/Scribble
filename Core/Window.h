#pragma once

#include<wtypes.h>
#include<string>
#include<functional>
#include"CoreInterface.h"

class CWindow
{
public:
	using WndProcListener = std::function<bool(HWND, UINT, WPARAM, LPARAM, LRESULT&)>;

public:
	CWindow(HINSTANCE hInstance);

	CWindow() = delete;
	CWindow(const CWindow&) = delete;
	CWindow& operator=(const CWindow&) = delete;

	bool Initialize();
	inline void AddWndProcListener(WndProcListener listener);

	int GetWidth();
	int GetHeight();
	HWND GetHandle();

private:
	LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	bool MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result);

private:
	HINSTANCE m_appInst{ nullptr };
	HWND m_wnd{ nullptr };
	std::vector<WndProcListener> m_wndProcListeners{};

	std::wstring m_caption = L"Directx3D App";

	int m_width{ 800 };
	int m_height{ 600 };
};