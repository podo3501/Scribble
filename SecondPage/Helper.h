#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <DirectXMath.h>

class CGameTimer;

std::wstring CalculateFrameStats(CGameTimer* timer);

struct SubRenderItem;
struct RenderItem;
using AllRenderItems = std::unordered_map<std::string, std::unique_ptr<RenderItem>>;

RenderItem* GetRenderItem(AllRenderItems& allRenderItems, const std::string& geoName);
SubRenderItem* GetSubRenderItem(RenderItem* renderItem, const std::string& meshName);
SubRenderItem* GetSubRenderItem(AllRenderItems& allRenderItems, const std::string& geoName, const std::string& meshName);

enum class eTextureType : int;
struct Material;
struct ModelProperty;

std::unique_ptr<Material> MakeMaterial(std::string&& name, eTextureType type, std::wstring&& filename,
	DirectX::XMFLOAT4 diffuseAlbedo, DirectX::XMFLOAT3 fresnelR0, float rough);

ModelProperty CreateMock(const std::string& meshName);