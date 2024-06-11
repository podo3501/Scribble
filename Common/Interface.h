#pragma once
#include <vector>

class KeyInputListener
{
public:
	virtual ~KeyInputListener() {};
	virtual void PressedKey(std::vector<int> keyList) = 0;
};