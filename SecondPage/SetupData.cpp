#include "./SetupData.h"
#include <algorithm>
#include <ranges>
#include "../Include/RenderItem.h"
#include "../Include/FrameResourceData.h"
#include "../Include/Types.h"
#include "../Include/Interface.h"
#include "./Utility.h"
#include "./Material.h"
#include "./Model.h"
#include "./Helper.h"
#include "./GeometryGenerator.h"

using namespace DirectX;

CSetupData::CSetupData() {}
CSetupData::~CSetupData() = default;

InstanceDataList CSetupData::CreateSkullInstanceData()
{
	InstanceDataList instances{};

	const int n = 5;
	int matCount = GetTextureCount(eTextureType::Common);
	int startIndex = GetTextureCount(eTextureType::Cube);

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

bool CSetupData::CreateModelMock()
{
	ModelProperty cube{};
	cube.createType = ModelProperty::CreateType::Generator;
	cube.meshData = Generator("cube");
	cube.cullingFrustum = false;
	cube.filename = L"";
	cube.instanceDataList = CreateSkyCubeInstanceData();
	ReturnIfFalse(InsertModelProperty("nature", "cube", cube));

	ModelProperty skull{};
	skull.createType = ModelProperty::CreateType::ReadFile;
	skull.meshData = nullptr;
	skull.cullingFrustum = true;
	skull.filename = L"skull.txt";
	skull.instanceDataList = CreateSkullInstanceData();
	ReturnIfFalse(InsertModelProperty("things", "skull", skull));

	//ModelProperty grid{};
	//grid.createType = ModelProperty::CreateType::Generator;
	//grid.meshData = Generator("grid");
	//grid.cullingFrustum = false;
	//grid.filename = L"";
	//grid.instanceDataList = CreateSkullInstanceData();
	//ReturnIfFalse(InsertModelProperty("things", "grid", grid));

	return true;
}

bool CSetupData::CreateTextureMock()
{
	TypeTextures cubeTextures(eTextureType::Cube, { L"grasscube1024.dds" });
	TypeTextures skullTextures(eTextureType::Common,
		{ L"bricks.dds", L"stone.dds", L"tile.dds", L"WoodCrate01.dds", L"ice.dds", L"grass.dds", L"white1x1.dds" });

	m_textureList.emplace_back(std::move(cubeTextures));
	m_textureList.emplace_back(std::move(skullTextures));

	return true;
}

bool CSetupData::CreateMaterialMock()
{
	auto MakeMaterial = [this, diffuseIndex{ 0u }](std::string&& name, eTextureType type, std::wstring&& filename,
		XMFLOAT4 diffuseAlbedo, XMFLOAT3 fresnelR0, float rough) mutable {
			std::shared_ptr<Material> curMat = std::make_shared<Material>();
			curMat->name = std::move(name);
			curMat->type = type;
			curMat->filename = std::move(filename);
			curMat->diffuseIndex = diffuseIndex++;
			curMat->normalSrvHeapIndex = 0;
			curMat->diffuseAlbedo = diffuseAlbedo;
			curMat->fresnelR0 = fresnelR0;
			curMat->roughness = rough;
			curMat->transform = DirectX::XMMatrixIdentity();
			m_materialList.emplace_back(std::move(curMat));
		};

	MakeMaterial("sky", eTextureType::Cube, L"grasscube1024.dds", { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f }, 1.0f);
	MakeMaterial("bricks0", eTextureType::Common, L"bricks.dds", { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.002f, 0.002f, 0.02f }, 0.1f);
	MakeMaterial("stone0", eTextureType::Common, L"stone.dds", { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.3f);
	MakeMaterial("tile0", eTextureType::Common, L"tile.dds", { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.02f, 0.02f, 0.02f }, 0.3f);
	MakeMaterial("checkboard0", eTextureType::Common, L"WoodCrate01.dds", { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.2f);
	MakeMaterial("ice0", eTextureType::Common, L"ice.dds", { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f }, 0.0f);
	MakeMaterial("grass0", eTextureType::Common, L"grass.dds", { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.2f);
	MakeMaterial("skullMat", eTextureType::Common, L"white1x1.dds", { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.5f);

	return true;
}

bool CSetupData::CreateMockData()
{
	ReturnIfFalse(CreateMaterialMock());
	ReturnIfFalse(CreateTextureMock());
	ReturnIfFalse(CreateModelMock());

	return true;
}

bool CSetupData::FillRenderItems(AllRenderItems* renderItems)
{
	std::ranges::for_each(m_allModelProperty, [renderItems](auto& geoProp) {
		std::ranges::for_each(geoProp.second, [renderItems, &geoProp](auto& meshProp) {
			SubRenderItem* subRenderItem = GetSubRenderItem((*renderItems), geoProp.first, meshProp.first);
			subRenderItem->instanceDataList = meshProp.second.instanceDataList;
			subRenderItem->cullingFrustum = meshProp.second.cullingFrustum;
			});
		});

	return true;
}

bool CSetupData::InsertModelProperty(const std::string& geoName, const std::string& meshName, ModelProperty& mProperty)
{
	auto& mesh = m_allModelProperty[geoName];
	if (mesh.find(meshName) != mesh.end())
		return false;

	m_allModelProperty[geoName].insert(std::make_pair(meshName, std::move(mProperty)));

	return true;
}

bool CSetupData::LoadMesh(CModel* model, const std::string& geoName, MeshProperty& meshProp)
{
	return std::ranges::all_of(meshProp, [model, &geoName](auto& mProp) {
		return model->LoadGeometry(geoName, mProp.first, &mProp.second);});
}

bool CSetupData::LoadModel(CModel* model, AllRenderItems* renderItems)
{
	ReturnIfFalse( std::ranges::all_of(m_allModelProperty, [this, model](auto& geoProp) {
		auto& geoName = geoProp.first;
		return LoadMesh(model, geoName, geoProp.second); }));

	return FillRenderItems(renderItems);
}

bool CSetupData::LoadTextureIntoVRAM(IRenderer* renderer, CMaterial* material)
{
	ReturnIfFalse(std::ranges::all_of(m_textureList, [renderer](auto& typeTex) {
		return renderer->LoadTexture(typeTex.first, typeTex.second); }));

	material->SetMaterialList(m_materialList);
	return true;
}

int CSetupData::GetTextureCount(eTextureType texType)
{
	auto find = std::ranges::find_if(m_textureList, [texType](auto& typeTex) { return typeTex.first == texType; });
	return static_cast<int>(find->second.size());
}