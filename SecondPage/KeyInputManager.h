#pragma once

#include<vector>
#include<functional>

class CKeyInputManager
{
public:
	using KeyListener = std::function<void(std::vector<int>)>;

	CKeyInputManager();
	CKeyInputManager(const CKeyInputManager&) = delete;
	CKeyInputManager& operator=(const CKeyInputManager&) = delete;

	void PressedKeyList(std::function<std::vector<int>()> keyList);
	void AddListener(KeyListener listener);

private:
	void Flush();
	
private:
	std::vector<int> m_keyList{};
	std::vector<KeyListener> m_keyListenerList{};
};