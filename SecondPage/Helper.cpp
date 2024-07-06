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
#include "./Utility.h"

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

InstanceDataList CreateSkullInstanceData(const std::vector<std::string>& materialNameList)
{
	InstanceDataList instances{};

	const int n = 5;
	int matCount = static_cast<int>(materialNameList.size());

	float width = 200.0f;
	float height = 200.0f;
	float depth = 200.0f;

	float x = -0.5f * width;
	float y = -0.5f * height;
	float z = 1.0f * depth;
	float dx = width / (n - 1);
	float dy = height / (n - 1);
	float dz = depth / (n - 1);

	for (int k = 0; k < n; ++k)
	{
		for (int i = 0; i < n; ++i)
		{
			for (int j = 0; j < n; ++j)
			{
				auto instance = std::make_unique<InstanceData>();
				const DirectX::XMFLOAT3 pos(x + j * dx, y + i * dy, z + k * dz);
				instance->world = DirectX::XMMATRIX(
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					pos.x, pos.y, pos.z, 1.0f);

				int index = k * n * n + i * n + j;
				instance->texTransform = DirectX::XMMatrixScaling(2.0f, 2.0f, 1.0f);
				
				instance->matName = materialNameList[index % matCount];
				instances.emplace_back(std::move(instance));
			}
		}
	}

	return instances;
}

InstanceDataList CreateSkyCubeInstanceData(const std::vector<std::string>& materialNameList)
{
	//하늘맵은 material과 texTransform을 쓰지 않고 shader에서 이렇게 사용
	// return gCubeMap.Sample(gsamLinearWrap, pin.PosL);
	InstanceDataList instances{};
	auto instance = std::make_unique<InstanceData>();
	instance->world = DirectX::XMMatrixIdentity();
	instance->matName = *materialNameList.begin();
	instances.emplace_back(std::move(instance));

	return instances;
}

ModelProperty CreateCubeMock()
{
	MaterialList materialList;
	materialList.emplace_back(MakeMaterial("sky", eTextureType::Cube, L"grasscube1024.dds",
		{ 1.0f, 1.0f, 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f }, 1.0f));

	std::vector<std::string> materialNameList{};
	std::ranges::transform(materialList, std::back_inserter(materialNameList), [](auto& mat) {
		return mat->name; });

	ModelProperty  modelProp{};
	modelProp.createType = ModelProperty::CreateType::Generator;
	modelProp.meshData = Generator("cube");
	modelProp.cullingFrustum = false;
	modelProp.filename = L"";
	modelProp.instanceDataList = CreateSkyCubeInstanceData(materialNameList);
	modelProp.materialList = materialList;

	return modelProp;
}

ModelProperty CreateSkullMock()
{
	MaterialList materialList;
	materialList.emplace_back(MakeMaterial("bricks0", eTextureType::Common, L"bricks.dds", { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.002f, 0.002f, 0.02f }, 0.1f));
	materialList.emplace_back(MakeMaterial("bricks1", eTextureType::Common, L"bricks2.dds", { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.002f, 0.002f, 0.02f }, 0.1f));
	materialList.emplace_back(MakeMaterial("bricks2", eTextureType::Common, L"bricks3.dds", { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.002f, 0.002f, 0.02f }, 0.1f));
	materialList.emplace_back(MakeMaterial("stone0", eTextureType::Common, L"stone.dds", { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.3f));
	materialList.emplace_back(MakeMaterial("tile0", eTextureType::Common, L"tile.dds", { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.02f, 0.02f, 0.02f }, 0.3f));
	materialList.emplace_back(MakeMaterial("checkboard0", eTextureType::Common, L"WoodCrate01.dds", { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.2f));
	materialList.emplace_back(MakeMaterial("ice0", eTextureType::Common, L"ice.dds", { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f }, 0.0f));
	materialList.emplace_back(MakeMaterial("grass0", eTextureType::Common, L"grass.dds", { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.2f));
	materialList.emplace_back(MakeMaterial("skullMat", eTextureType::Common, L"white1x1.dds", { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.5f));

	std::vector<std::string> materialNameList{};
	std::ranges::transform(materialList, std::back_inserter(materialNameList), [](auto& mat) {
		return mat->name; });

	ModelProperty  modelProp{};
	modelProp.createType = ModelProperty::CreateType::ReadFile;
	modelProp.meshData = nullptr;
	modelProp.cullingFrustum = true;
	modelProp.filename = L"skull.txt";
	modelProp.instanceDataList = CreateSkullInstanceData(materialNameList);
	modelProp.materialList = materialList;

	return modelProp;
}

ModelProperty CreateMock(const std::string& meshName)
{
	if (meshName == "cube")
		return CreateCubeMock();
	else if (meshName == "skull")
		return CreateSkullMock();

	return ModelProperty{};
}

bool MakeMockData(CSetupData* setupData, CMaterial* material)
{
	ReturnIfFalse(setupData->InsertModelProperty("nature", "cube", CreateMock("cube"), material));
	ReturnIfFalse(setupData->InsertModelProperty("things", "skull", CreateMock("skull"), material));

	return true;
}