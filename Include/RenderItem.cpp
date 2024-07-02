#include "RenderItem.h"

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