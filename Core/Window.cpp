#include "Window.h"

LRESULT CALLBACK CWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

CWindow::CWindow(HINSTANCE hInstance)
	: m_appInst(hInstance)
{}

int CWindow::GetWidth() { return m_width; }
int CWindow::GetHeight() { return m_height; }
HWND CWindow::GetHandle() { return m_wnd; }

bool CWindow::Initialize()
{
	static CWindow* gWindow = this;
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

void CWindow::AddWndProcListener(WndProcListener listener)
{
	m_wndProcListeners.emplace_back(std::move(listener));
}
