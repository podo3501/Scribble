#pragma once

#include <wtypes.h>
#include <string>
#include <functional>
#include <utility>
#include <vector>
#include "CoreInterface.h"

class CWindow
{
public:
	using WndProcListener = std::function<bool(HWND, UINT, WPARAM, LPARAM, LRESULT&)>;
	using OnResizeListener = std::function<bool(int, int)>;
	using AppPauseListener = std::function<void(bool)>;

public:
	CWindow(HINSTANCE hInstance);
	~CWindow();

	CWindow() = delete;
	CWindow(const CWindow&) = delete;
	CWindow& operator=(const CWindow&) = delete;

	bool Initialize(bool bShow);

	inline void AddWndProcListener(WndProcListener listener);
	inline void AddOnResizeListener(OnResizeListener listener);
	inline void AddAppPauseListener(AppPauseListener listener);

	int GetWidth();
	int GetHeight();
	HWND GetHandle();

	inline void SetText(std::wstring text);

private:
	LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	bool CALLBACK OnResize();
	void CALLBACK AppPause(bool pause);

	bool MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result);

private:
	std::wstring m_className{};
	HINSTANCE m_appInst{ nullptr };
	HWND m_wnd{ nullptr };
	std::vector<WndProcListener> m_wndProcListeners{};
	OnResizeListener m_onResizeListener{};
	AppPauseListener m_appPauseListener{};

	std::wstring m_caption = L"Directx3D App";

	int m_width{ 800 };
	int m_height{ 600 };

	bool m_minimized{ false };
	bool m_maximized{ false };
	bool m_resizing{ false };
};

inline void CWindow::SetText(std::wstring text)
{
	SetWindowText(m_wnd, text.c_str());
}

inline void CWindow::AddWndProcListener(WndProcListener listener)		{	m_wndProcListeners.emplace_back(std::move(listener)); }
inline void CWindow::AddOnResizeListener(OnResizeListener listener)		{	m_onResizeListener = listener; }
inline void CWindow::AddAppPauseListener(AppPauseListener listener)		{	m_appPauseListener = listener; }

