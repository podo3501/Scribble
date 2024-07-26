#include "pch.h"
#include "Helper.h"
#include "../Include/RenderItem.h"
#include "../Include/Types.h"
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

RenderItem* GetRenderItem(AllRenderItems& allRenderItems, GraphicsPSO pso)
{
	RenderItem* pRenderItem = nullptr;
	auto findGeo = allRenderItems.find(pso);
	if (findGeo != allRenderItems.end())
		pRenderItem = findGeo->second.get();
	else
		return nullptr;

	return pRenderItem;
}

RenderItem* MakeRenderItem(AllRenderItems& allRenderItems, GraphicsPSO pso)
{
	RenderItem* curRItem = GetRenderItem(allRenderItems, pso);
	if (curRItem != nullptr)
		return curRItem;

	auto renderItem = std::make_unique<RenderItem>();
	RenderItem* pRenderItem = renderItem.get();
	allRenderItems.insert(std::make_pair(pso, std::move(renderItem)));

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

SubRenderItem* MakeSubRenderItem(AllRenderItems& allRenderItems, GraphicsPSO pso, const std::string& meshName)
{
	auto* rItem = GetRenderItem(allRenderItems, pso);
	if (rItem != nullptr) 
		return GetSubRenderItem(rItem, meshName);
	
	auto renderItem = std::make_unique<RenderItem>();
	RenderItem* pRenderItem = renderItem.get();
	allRenderItems.insert(std::make_pair(pso, std::move(renderItem)));
	
	return GetSubRenderItem(pRenderItem, meshName);
}

SubRenderItem* GetSubRenderItem(AllRenderItems& allRenderItems, GraphicsPSO pso, const std::string& meshName)
{
	RenderItem* renderItem = GetRenderItem(allRenderItems, pso);
	if (renderItem == nullptr) return nullptr;

	return GetSubRenderItem(renderItem, meshName);
}
