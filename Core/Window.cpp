#include "Window.h"
#include <algorithm>

LRESULT CALLBACK CWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//for_each�� ���ٰ� ���ǹǷ� return�� �ص� WndProc�Լ��� �������� �ν����� �ʴ´� �׷��� for�����
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

//���̻� ó���Ұ� ������ true�� ����, ó���Ұ� ������ false, ó���Ұ� �ִµ� �ý��ۿ��� �ļ� ó���� �ʿ��ϸ� LRESULT�� ���� �ִ´�.
bool CWindow::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result)
{
	switch (msg)
	{
	case WM_SIZE:
		m_width = static_cast<int>(LOWORD(lParam));
		m_height = static_cast<int>(HIWORD(lParam));
		return false;	//�ٸ� MsgProc���� ó���� �� ���Ҵ�.

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
	static CWindow* gWindow = this;	//ĸ�İ� ������ WNDPROC���� ��ȯ�� ���� �ʴ´�. �׷��� ��������Ȱ��
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
