#pragma once

#include <wtypes.h>
#include <string>
#include <functional>
#include <utility>
#include "CoreInterface.h"

class CWindow
{
public:
	using WndProcListener = std::function<bool(HWND, UINT, WPARAM, LPARAM, LRESULT&)>;

public:
	CWindow(HINSTANCE hInstance);
	~CWindow();

	CWindow() = delete;
	CWindow(const CWindow&) = delete;
	CWindow& operator=(const CWindow&) = delete;

	bool Initialize();
	inline void AddWndProcListener(WndProcListener listener);

	int GetWidth();
	int GetHeight();
	HWND GetHandle();

	inline void SetText(std::wstring text);

private:
	LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	bool MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result);

private:
	std::wstring m_className{};
	HINSTANCE m_appInst{ nullptr };
	HWND m_wnd{ nullptr };
	std::vector<WndProcListener> m_wndProcListeners{};

	std::wstring m_caption = L"Directx3D App";

	int m_width{ 800 };
	int m_height{ 600 };
};

inline void CWindow::SetText(std::wstring text)
{
	SetWindowText(m_wnd, text.c_str());
}
