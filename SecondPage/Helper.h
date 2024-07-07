#pragma once

#include <string>
#include <unordered_map>
#include <memory>

class CGameTimer;

std::wstring CalculateFrameStats(CGameTimer* timer);

struct SubRenderItem;
struct RenderItem;
using AllRenderItems = std::unordered_map<std::string, std::unique_ptr<RenderItem>>;

RenderItem* GetRenderItem(AllRenderItems& allRenderItems, const std::string& geoName);
SubRenderItem* GetSubRenderItem(RenderItem* renderItem, const std::string& meshName);
SubRenderItem* GetSubRenderItem(AllRenderItems& allRenderItems, const std::string& geoName, const std::string& meshName);