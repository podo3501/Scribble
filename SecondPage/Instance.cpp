#include "./Instance.h"
#include <algorithm>
#include <ranges>
#include "../Core/Utility.h"
#include "../Core/d3dUtil.h"
#include "./Material.h"
#include "./Model.h"
#include "../Include/RenderItem.h"
#include "../Include/FrameResourceData.h"

using namespace DirectX;

CInstance::CInstance() {}
CInstance::~CInstance() = default;

InstanceDataList CreateSkullInstanceData(CMaterial* material)
{
	InstanceDataList instances{};

	const int n = 5;
	int matCount = 7; 
	int startIndex = 1; 

	float width = 200.0f;
	float height = 200.0f;
	float depth = 200.0f;

	float x = -0.5f * width;
	float y = -0.5f * height;
	float z = -0.5f * depth;
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
				const XMFLOAT3 pos(x + j * dx, y + i * dy, z + k * dz);
				instance->world = XMMATRIX(
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					pos.x, pos.y, pos.z, 1.0f);

				int index = k * n * n + i * n + j;
				instance->texTransform = XMMatrixScaling(2.0f, 2.0f, 1.0f);
				instance->matIndex = WrapAround(index + startIndex, startIndex, matCount + startIndex);
				instances.emplace_back(std::move(instance));
			}
		}
	}

	return instances;
}

InstanceDataList CreateSkyCubeInstanceData()
{
	//하늘맵은 material과 texTransform을 쓰지 않고 shader에서 이렇게 사용
	// return gCubeMap.Sample(gsamLinearWrap, pin.PosL);
	InstanceDataList instances{};
	auto instance = std::make_unique<InstanceData>();
	instance->world = XMMatrixIdentity();
	instances.emplace_back(std::move(instance));

	return instances;
}

bool CInstance::CreateMockData()
{
	ModelProperty cube{};
	cube.createType = ModelProperty::CreateType::Generator;
	cube.cullingFrustum = false;
	cube.filename = L"";
	cube.instanceDataList = CreateSkyCubeInstanceData();
	ReturnIfFalse(Insert("nature", "cube", cube));

	ModelProperty skull{};
	skull.createType = ModelProperty::CreateType::ReadFile;
	skull.cullingFrustum = true;
	skull.filename = L"skull.txt";
	skull.instanceDataList = CreateSkullInstanceData(nullptr);
	ReturnIfFalse(Insert("things", "skull", skull));

	return true;
}

bool FillInstanceInfo(SubRenderItems& subRenderItems, const MeshProperty& meshProperties)
{
	return std::ranges::all_of(meshProperties, [&subRenderItems](auto& mProp) {
		auto findSubItem = subRenderItems.find(mProp.first);
		if (findSubItem == subRenderItems.end())
			return false;

		const ModelProperty& modelProp = mProp.second;
		auto& subItem = findSubItem->second;
		subItem.instanceDataList = modelProp.instanceDataList;
		subItem.cullingFrustum = modelProp.cullingFrustum;
		return true; });
}

bool CInstance::FillRenderItems(AllRenderItems& renderItems)
{
	return std::ranges::all_of(m_allModelProperty, [&renderItems](auto& geoProp) {
		const std::string& geoName = geoProp.first;
		auto findGeo = renderItems.find(geoName);
		if (findGeo == renderItems.end())
			return false;
		SubRenderItems& subRenderItems = findGeo->second->subRenderItems;
		return FillInstanceInfo(subRenderItems, geoProp.second); });
}

bool CInstance::Insert(const std::string& geoName, const std::string& meshName, ModelProperty& mProperty)
{
	auto& mesh = m_allModelProperty[geoName];
	if (mesh.find(meshName) != mesh.end())
		return false;

	m_allModelProperty[geoName].insert(std::make_pair(meshName, std::move(mProperty)));

	return true;
}

bool CInstance::LoadMesh(CModel* model, const std::string& geoName, MeshProperty& meshProp)
{
	return std::ranges::all_of(meshProp, [model, &geoName](auto& mProp) {
		return model->LoadGeometry(geoName, mProp.first, &mProp.second);});
}

bool CInstance::LoadModel(CModel* model)
{
	return std::ranges::all_of(m_allModelProperty, [this, model](auto& geoProp) {
		auto& geoName = geoProp.first;
		return LoadMesh(model, geoName, geoProp.second); });
}