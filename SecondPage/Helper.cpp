#include "Helper.h"
#include <algorithm>
#include <ranges>
#include "../Include/RenderItem.h"
#include "../Include/Types.h"
#include "../Include/FrameResourceData.h"
#include "./GameTimer.h"
#include "./Material.h"
#include "./SetupData.h"
#include "./GeometryGenerator.h"
#include "./Model.h"

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

std::unique_ptr<Material> MakeMaterial(std::string&& name, eTextureType type, std::wstring&& filename,
	DirectX::XMFLOAT4 diffuseAlbedo, DirectX::XMFLOAT3 fresnelR0, float rough)
{
	std::unique_ptr<Material> material = std::make_unique<Material>();
	material->name = std::move(name);
	material->type = type;
	material->filename = std::move(filename);
	material->normalSrvHeapIndex = 0;
	material->diffuseAlbedo = diffuseAlbedo;
	material->fresnelR0 = fresnelR0;
	material->roughness = rough;
	material->transform = DirectX::XMMatrixIdentity();

	return std::move(material);
}

std::unique_ptr<MeshData> Generator(std::string&& meshName)
{
	auto meshData = std::make_unique<MeshData>();
	meshData->name = std::move(meshName);

	CGeometryGenerator geoGen{};
	CGeometryGenerator::MeshData box = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3);

	std::ranges::transform(box.Vertices, std::back_inserter(meshData->vertices),
		[](auto& gen) { return Vertex(gen.Position, gen.Normal, gen.TexC); });
	meshData->indices.insert(meshData->indices.end(), box.Indices32.begin(), box.Indices32.end());

	return std::move(meshData);
}

InstanceDataList CreateSkyCubeInstanceData()
{
	//하늘맵은 material과 texTransform을 쓰지 않고 shader에서 이렇게 사용
	// return gCubeMap.Sample(gsamLinearWrap, pin.PosL);
	InstanceDataList instances{};
	auto instance = std::make_unique<InstanceData>();
	instance->world = DirectX::XMMatrixIdentity();
	instances.emplace_back(std::move(instance));

	return instances;
}

ModelProperty CreateMock(const std::string& meshName)
{
	MaterialList materialList;
	materialList.emplace_back(MakeMaterial("sky", eTextureType::Cube, L"grasscube1024.dds", 
		{ 1.0f, 1.0f, 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f }, 1.0f));

	ModelProperty  modelProp{};
	modelProp.createType = ModelProperty::CreateType::Generator;
	modelProp.meshData = Generator("cube");
	modelProp.cullingFrustum = false;
	modelProp.filename = L"";
	modelProp.instanceDataList = CreateSkyCubeInstanceData();
	modelProp.materialList = materialList;

	return modelProp;
}