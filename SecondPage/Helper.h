#pragma once

#include <string>
#include <map>
#include <memory>

class CGameTimer;
enum class GraphicsPSO : int;

std::wstring CalculateFrameStats(CGameTimer* timer);

struct SubRenderItem;
struct RenderItem;
using AllRenderItems = std::map<GraphicsPSO, std::unique_ptr<RenderItem>>;

RenderItem* GetRenderItem(AllRenderItems& allRenderItems, GraphicsPSO pso);
RenderItem* MakeRenderItem(AllRenderItems& allRenderItems, GraphicsPSO pso);
SubRenderItem* GetSubRenderItem(RenderItem* renderItem, const std::string& meshName);
SubRenderItem* GetSubRenderItem(AllRenderItems& allRenderItems, GraphicsPSO pso, const std::string& meshName);
SubRenderItem* MakeSubRenderItem(AllRenderItems& allRenderItems, GraphicsPSO pso, const std::string& meshName);