#include "KeyInputManager.h"

CKeyInputManager::CKeyInputManager()
{}

void CKeyInputManager::PressedKeyList(std::function<std::vector<int>()> keyList)
{
	m_keyList = keyList();

	Flush();
}

void CKeyInputManager::AddListener(KeyListener listener)
{
	m_keyListenerList.emplace_back(std::move(listener));
}

void CKeyInputManager::Flush()
{
	if (m_keyList.empty() == true)
		return;

	for (auto listener : m_keyListenerList)
	{
		listener(m_keyList);
	}

	m_keyList.clear();
}
