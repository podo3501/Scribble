#pragma once

#include<vector>
#include<functional>

class KeyInputManager
{
public:
	using KeyListener = std::function<void(std::vector<int>)>;

	KeyInputManager();
	void PressedKeyList(std::function<std::vector<int>()> keyList);
	void AddListener(KeyListener listener);

private:
	void Flush();
	
private:
	std::vector<int> m_keyList{};
	std::vector<KeyListener> m_keyListenerList{};
};