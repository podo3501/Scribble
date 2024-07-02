#include "Helper.h"
#include "./GameTimer.h"

std::wstring CalculateFrameStats(CGameTimer* timer)
{
	static int _frameCnt = 0;
	static float _timeElapsed = 0.0f;

	_frameCnt++;

	const bool IsOverOneSecond = ((timer->TotalTime() - _timeElapsed) >= 1.0f);
	if (!IsOverOneSecond) return L"";

	float fps = (float)_frameCnt; // fps = frameCnt / 1
	float mspf = 1000.0f / fps;

	std::wstring fpsStr = std::to_wstring(fps);
	std::wstring mspfStr = std::to_wstring(mspf);

	_frameCnt = 0;
	_timeElapsed += 1.0f;

	return std::wstring(L"    fps: " + fpsStr + L"   mspf: " + mspfStr);
}
