#pragma once

#include<vector>
#include<functional>
#include<Windows.h>

class CKeyInput
{
public:
	using KeyListener = std::function<void(std::vector<int>)>;
	using MouseListener = std::function<void(float, float)>;

	CKeyInput(HWND hwnd);
	~CKeyInput();

	CKeyInput() = delete;
	CKeyInput(const CKeyInput&) = delete;
	CKeyInput& operator=(const CKeyInput&) = delete;

	bool CALLBACK MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& lr);

	void AddKeyListener(KeyListener listener);
	void AddMouseListener(MouseListener listener);
	void CheckInput();

private:
	void CALLBACK PressedKeyList(std::function<std::vector<int>()> keyList);
	void CALLBACK DraggedMouse(float dx, float dy);

	void CALLBACK OnMouseDown(WPARAM btnState, int x, int y);
	void CALLBACK OnMouseUp(WPARAM btnState, int x, int y);
	void CALLBACK OnMouseMove(WPARAM btnState, int x, int y);
	
private:
	HWND m_hwnd{ nullptr };

	std::vector<KeyListener> m_keyListenerList{};
	std::vector<MouseListener> m_mouseListenerList{};
	POINT m_lastMousePos{};
};