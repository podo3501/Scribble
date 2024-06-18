#include "Window.h"
#include <algorithm>

LRESULT CALLBACK CWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//for_each는 람다가 사용되므로 return을 해도 WndProc함수의 리턴으로 인식하지 않는다 그래서 for를사용
	for (auto Iter = m_wndProcListeners.begin(); Iter != m_wndProcListeners.end(); ++Iter)
	{
		WndProcListener& listener = (*Iter);
		LRESULT lResult{ 0 };
		bool bResult = listener(hwnd, msg, wParam, lParam, lResult);
		if (bResult == true)
			return lResult;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

//더이상 처리할게 없으면 true를 리턴, 처리할게 있으면 false, 처리할게 있는데 시스템에서 후속 처리가 필요하면 LRESULT에 값을 넣는다.
bool CWindow::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result)
{
	switch (msg)
	{
	case WM_SIZE:
		m_width = static_cast<int>(LOWORD(lParam));
		m_height = static_cast<int>(HIWORD(lParam));
		return false;	//다른 MsgProc에서 처리할 게 남았다.

	case WM_DESTROY:
		PostQuitMessage(0);
		return true;

	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		result = MAKELRESULT(0, MNC_CLOSE);
		return true;

		// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return true;

	case WM_KEYUP:
		if (wParam == VK_ESCAPE)
			PostQuitMessage(0);
		return false;
	}

	return false;
}


CWindow::CWindow(HINSTANCE hInstance)
	: m_appInst(hInstance)
{
	AddWndProcListener([window = this](HWND w, UINT m, WPARAM wp, LPARAM lp, LRESULT& lr)->bool {
		return window->MsgProc(w, m, wp, lp, lr); }); 
}

int CWindow::GetWidth() { return m_width; }
int CWindow::GetHeight() { return m_height; }
HWND CWindow::GetHandle() { return m_wnd; }

bool CWindow::Initialize()
{
	static CWindow* gWindow = this;	//캡쳐가 있으면 WNDPROC으로 변환이 되지 않는다. 그래서 정적변수활용
	auto windowProc = [](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)->LRESULT {
		return gWindow->WndProc(hwnd, msg, wParam, lParam);
		};

	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = windowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_appInst;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"MainWnd";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, m_width, m_height };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	m_wnd = CreateWindow(L"MainWnd", m_caption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, m_appInst, 0);
	if (!m_wnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(m_wnd, SW_SHOW);
	UpdateWindow(m_wnd);

	return true;
}

inline void CWindow::AddWndProcListener(WndProcListener listener)
{
	m_wndProcListeners.emplace_back(std::move(listener));
}
