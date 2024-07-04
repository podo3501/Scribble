#include "Helper.h"
#include "./GameTimer.h"
#include "../Include/RenderItem.h"

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

RenderItem* GetRenderItem(AllRenderItems& allRenderItems, const std::string& geoName)
{
	RenderItem* pRenderItem = nullptr;
	auto findGeo = allRenderItems.find(geoName);
	if (findGeo != allRenderItems.end())
		pRenderItem = findGeo->second.get();
	else
	{
		auto renderItem = std::make_unique<RenderItem>();
		pRenderItem = renderItem.get();
		allRenderItems.insert(std::make_pair(geoName, std::move(renderItem)));
	}

	return pRenderItem;
}

SubRenderItem* GetSubRenderItem(RenderItem* renderItem, const std::string& meshName)
{
	SubRenderItems& subRItems = renderItem->subRenderItems;
	auto findSubRItems = subRItems.find(meshName);

	if (findSubRItems != subRItems.end())
		return &findSubRItems->second;

	return &subRItems[meshName];
}

SubRenderItem* GetSubRenderItem(AllRenderItems& allRenderItems, const std::string& geoName, const std::string& meshName)
{
	RenderItem* renderItem = GetRenderItem(allRenderItems, geoName);
	return GetSubRenderItem(renderItem, meshName);
}