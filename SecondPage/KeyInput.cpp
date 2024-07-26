#include "pch.h"
#include "KeyInput.h"

bool CKeyInput::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& lr)
{
	switch (msg)
	{
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return true;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return true;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return true;
	}

	return false;
}

CKeyInput::CKeyInput(HWND hwnd)
	: m_hwnd(hwnd)
{}
CKeyInput::~CKeyInput() = default;

void CKeyInput::AddKeyListener(KeyListener listener)
{
	m_keyListenerList.emplace_back(std::move(listener));
}

void CKeyInput::AddMouseListener(MouseListener listener)
{
	m_mouseListenerList.emplace_back(std::move(listener));
}

void CKeyInput::PressedKeyList(std::function<std::vector<int>()> keyList)
{
	for (auto listener : m_keyListenerList)
		listener(keyList());
}

void CKeyInput::DraggedMouse(float dx, float dy)
{
	for (auto listener : m_mouseListenerList)
		listener(dx, dy);
}

void CKeyInput::CheckInput()
{
	//�ӽ÷� GetAsyncKeyState�� Ű ������ �����ߴ�. ���߿� �ٸ� input���� �ٲ� ����
	PressedKeyList([]()->std::vector<int> {
		std::vector<int> keyList{ 'W', 'S', 'D', 'A', '1', '2' };
		std::vector<int> pressedKeyList{};
		std::ranges::copy_if(keyList, std::back_inserter(pressedKeyList),
			[](int vKey) { return GetAsyncKeyState(vKey) & 0x8000; });
		return pressedKeyList; });
}

void CKeyInput::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_lastMousePos.x = x;
	m_lastMousePos.y = y;

	SetCapture(m_hwnd);
}

void CKeyInput::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void CKeyInput::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = DirectX::XMConvertToRadians(0.25f * static_cast<float>(x - m_lastMousePos.x));
		float dy = DirectX::XMConvertToRadians(0.25f * static_cast<float>(y - m_lastMousePos.y));

		DraggedMouse(dx, dy);
	}

	m_lastMousePos.x = x;
	m_lastMousePos.y = y;
}