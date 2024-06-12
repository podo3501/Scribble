#include "KeyInputManager.h"

KeyInputManager::KeyInputManager()
{}

void KeyInputManager::PressedKeyList(std::function<std::vector<int>()> keyList)
{
	m_keyList = keyList();

	Flush();
}

void KeyInputManager::AddListener(KeyListener listener)
{
	m_keyListenerList.emplace_back(std::move(listener));
}

void KeyInputManager::Flush()
{
	if (m_keyList.empty() == true)
		return;

	for (auto listener : m_keyListenerList)
	{
		listener(m_keyList);
	}

	m_keyList.clear();
}
