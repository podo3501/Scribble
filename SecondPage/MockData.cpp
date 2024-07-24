#include "MockData.h"
#include <algorithm>
#include <ranges>
#include "../Include/RenderItem.h"
#include "../Include/Types.h"
#include "../Include/FrameResourceData.h"
#include "../Include/Interface.h"
#include "./Material.h"
#include "./SetupData.h"
#include "./GeometryGenerator.h"
#include "./Mesh.h"
#include "./Utility.h"

std::wstring GetFilename(std::vector<std::wstring>& fileList, int index)
{
	return fileList.size() >= index + 1 ? fileList[index] : L"";
}

std::unique_ptr<Material> MakeMaterial(std::string&& name, SrvOffset type, std::vector<std::wstring> texFilenames,
	DirectX::XMFLOAT4 diffuseAlbedo, DirectX::XMFLOAT3 fresnelR0, float rough)
{
	std::unique_ptr<Material> material = std::make_unique<Material>();
	material->name = std::move(name);
	material->type = type;
	material->diffuseName = GetFilename(texFilenames, 0);
	material->normalName = GetFilename(texFilenames, 1);
	material->diffuseAlbedo = diffuseAlbedo;
	material->fresnelR0 = fresnelR0;
	material->roughness = rough;
	material->transform = DirectX::XMMatrixIdentity();

	return std::move(material);
}

std::unique_ptr<MeshData> Generator(std::string&& meshName)
{
	auto meshData = std::make_unique<MeshData>();
	
	CGeometryGenerator geoGen{};
	CGeometryGenerator::MeshData genMeshData{};
	if(meshName == "cube")
		genMeshData = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3);
	else if(meshName == "grid")
		genMeshData = geoGen.CreateGrid(20.0f, 30.0f, 60, 40);
	else if(meshName == "cylinder")
		genMeshData = geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);
	else if(meshName == "sphere")
		genMeshData = geoGen.CreateSphere(0.5f, 20, 20);
	else if(meshName == "debug")
		genMeshData = geoGen.CreateQuad(0.4f, -0.4f, 0.59f, 0.59f, 0.0f);
	meshData->name = std::move(meshName);

	std::ranges::transform(genMeshData.Vertices, std::back_inserter(meshData->vertices),
		[](auto& gen) { 
			DirectX::XMFLOAT4 tangentU( gen.TangentU.x, gen.TangentU.y, gen.TangentU.z, 0.0f );
			return Vertex(gen.Position, gen.Normal, gen.TexC, tangentU	); });
	meshData->indices.insert(meshData->indices.end(), genMeshData.Indices32.begin(), genMeshData.Indices32.end());

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

InstanceDataList CreateSoldierInstanceData(const std::vector<std::string>& materialNameList)
{
	InstanceDataList instances{};

	return instances;
}

InstanceDataList CreateCylinderInstanceData(const std::vector<std::string>& materialNameList)
{
	InstanceDataList instances{};
	
	for(auto i : std::views::iota(0, 8))
	{
		auto instance = std::make_unique<InstanceData>();
		instance->world = DirectX::XMMatrixTranslation(-3.0f, 1.5f, -10.0f + i * 3.0f);
		instances.emplace_back(std::move(instance));
	}

	for (auto i : std::views::iota(0, 8))
	{
		auto instance = std::make_unique<InstanceData>();
		instance->world = DirectX::XMMatrixTranslation(+3.0f, 1.5f, -10.0f + i * 3.0f);
		instances.emplace_back(std::move(instance));
	}
	
	std::ranges::for_each(instances, [&materialNameList](auto& instance) {
		instance->texTransform = DirectX::XMMatrixScaling(1.5f, 2.0f, 1.0f);
		instance->matName = *materialNameList.begin(); });
	
	return instances;
}

InstanceDataList CreateSkyCubeInstanceData(const std::vector<std::string>& materialNameList)
{
	//하늘맵은 material과 texTransform을 쓰지 않고 shader에서 이렇게 사용
	// return gCubeMap.Sample(gsamLinearWrap, pin.PosL);
	InstanceDataList instances{};
	auto instance = std::make_unique<InstanceData>();
	instance->world = DirectX::XMMatrixIdentity();
	instance->texTransform = DirectX::XMMatrixIdentity();
	instance->matName = *materialNameList.begin();
	instances.emplace_back(std::move(instance));

	return instances;
}

InstanceDataList CreateGridInstanceData(const std::vector<std::string>& materialNameList)
{
	InstanceDataList instances{};
	auto instance = std::make_unique<InstanceData>();
	instance->world = DirectX::XMMatrixIdentity();
	instance->texTransform = DirectX::XMMatrixScaling(8.0f, 8.0f, 1.0f);
	instance->matName = *materialNameList.begin();
	instances.emplace_back(std::move(instance));

	return instances;
}

InstanceDataList CreateSphereInstanceData(const std::vector<std::string>& materialNameList)
{
	InstanceDataList instances{};

	for (auto i : std::views::iota(0, 8))
	{
		auto instance = std::make_unique<InstanceData>();
		instance->world = DirectX::XMMatrixTranslation(-3.0f, 3.5f, -10.0f + i * 3.0f);
		instances.emplace_back(std::move(instance));
	}

	for (auto i : std::views::iota(0, 8))
	{
		auto instance = std::make_unique<InstanceData>();
		instance->world = DirectX::XMMatrixTranslation(+3.0f, 3.5f, -10.0f + i * 3.0f);
		instances.emplace_back(std::move(instance));
	}

	std::ranges::for_each(instances, [&materialNameList](auto& instance) {
		instance->texTransform = DirectX::XMMatrixIdentity();
		instance->matName = *materialNameList.begin(); });

	return instances;
}

InstanceDataList CreateDebugInstanceData()
{
	InstanceDataList instances{};
	auto instance = std::make_unique<InstanceData>();
	instance->world = DirectX::XMMatrixIdentity();
	instance->texTransform = DirectX::XMMatrixIdentity();
	instance->matName = {};
	instances.emplace_back(std::move(instance));

	return instances;
}

ModelProperty CreateCubeMock()
{
	MaterialList materialList;
	materialList.emplace_back(MakeMaterial("sky", SrvOffset::TextureCube, { L"grasscube1024.dds" },
		{ 1.0f, 1.0f, 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f }, 1.0f));

	std::vector<std::string> materialNameList{};
	std::ranges::transform(materialList, std::back_inserter(materialNameList), [](auto& mat) {
		return mat->name; });

	ModelProperty  modelProp{};
	modelProp.createType = ModelProperty::CreateType::Generator;
	modelProp.meshData = Generator("cube");
	modelProp.cullingFrustum = false;
	modelProp.filename = {};
	modelProp.instanceDataList = CreateSkyCubeInstanceData(materialNameList);
	modelProp.materialList = materialList;

	return modelProp;
}

ModelProperty CreateSkullMock()
{
	MaterialList materialList;
	materialList.emplace_back(MakeMaterial("bricks0", SrvOffset::Texture2D, { L"bricks.dds" }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.002f, 0.002f, 0.02f }, 0.1f));
	materialList.emplace_back(MakeMaterial("stone0", SrvOffset::Texture2D, { L"stone.dds" }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.3f));
	materialList.emplace_back(MakeMaterial("tile0", SrvOffset::Texture2D, { L"tile.dds" }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.02f, 0.02f, 0.02f }, 0.3f));
	materialList.emplace_back(MakeMaterial("checkboard0", SrvOffset::Texture2D, { L"WoodCrate01.dds" }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.2f));
	materialList.emplace_back(MakeMaterial("ice0", SrvOffset::Texture2D, { L"ice.dds" }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f }, 0.0f));
	materialList.emplace_back(MakeMaterial("grass0", SrvOffset::Texture2D, { L"grass.dds" }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.2f));
	materialList.emplace_back(MakeMaterial("skullMat", SrvOffset::Texture2D, { L"white1x1.dds" }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.05f, 0.05f, 0.05f }, 0.5f));

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

ModelProperty CreateSoldierMock()
{
	ModelProperty  modelProp{};
	modelProp.createType = ModelProperty::CreateType::ReadFile;
	modelProp.meshData = nullptr;
	modelProp.cullingFrustum = false;
	modelProp.filename = L"soldier.m3d";
	modelProp.instanceDataList = {};
	modelProp.materialList = {};

	return modelProp;
}

ModelProperty CreateGridMock()
{
	MaterialList materialList;
	materialList.emplace_back(MakeMaterial("nTile", SrvOffset::Texture2D, { L"tile.dds", L"tile_nmap.dds" }, { 0.9f, 0.9f, 0.9f, 1.0f }, { 0.2f, 0.2f, 0.2f }, 0.1f));

	std::vector<std::string> materialNameList{};
	std::ranges::transform(materialList, std::back_inserter(materialNameList), [](auto& mat) {
		return mat->name; });

	ModelProperty  modelProp{};
	modelProp.createType = ModelProperty::CreateType::Generator;
	modelProp.meshData = Generator("grid");
	modelProp.cullingFrustum = false;
	modelProp.filename = {};
	modelProp.instanceDataList = CreateGridInstanceData(materialNameList);
	modelProp.materialList = materialList;

	return modelProp;
}

ModelProperty CreateCylinderMock()
{
	MaterialList materialList;
	materialList.emplace_back(MakeMaterial("nBricks2", SrvOffset::Texture2D, { L"bricks.dds", L"bricks_nmap.dds" }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f }, 0.3f));

	std::vector<std::string> materialNameList{};
	std::ranges::transform(materialList, std::back_inserter(materialNameList), [](auto& mat) {
		return mat->name; });

	ModelProperty  modelProp{};
	modelProp.createType = ModelProperty::CreateType::Generator;
	modelProp.meshData = Generator("cylinder");
	modelProp.cullingFrustum = false;
	modelProp.filename = {};
	modelProp.instanceDataList = CreateCylinderInstanceData(materialNameList);
	modelProp.materialList = materialList;

	return modelProp;
}

ModelProperty CreateSphereMock()
{
	MaterialList materialList;
	materialList.emplace_back(MakeMaterial("mirror0", SrvOffset::Texture2D, { L"white1x1.dds", L"default_nmap.dds" },
		{ 0.0f, 0.0f, 0.0f, 1.0f }, { 0.98f, 0.97f, 0.95f }, 0.1f));

	std::vector<std::string> materialNameList{};
	std::ranges::transform(materialList, std::back_inserter(materialNameList), [](auto& mat) {
		return mat->name; });

	ModelProperty  modelProp{};
	modelProp.createType = ModelProperty::CreateType::Generator;
	modelProp.meshData = Generator("sphere");
	modelProp.cullingFrustum = false;
	modelProp.filename = {};
	modelProp.instanceDataList = CreateSphereInstanceData(materialNameList);
	modelProp.materialList = materialList;

	return modelProp;
}

ModelProperty CreateDebugMock()
{
	ModelProperty  modelProp{};
	modelProp.createType = ModelProperty::CreateType::Generator;
	modelProp.meshData = Generator("debug");
	modelProp.cullingFrustum = false;
	modelProp.filename = {};
	modelProp.instanceDataList = CreateDebugInstanceData();
	modelProp.materialList = {};

	return modelProp;
}

ModelProperty CreateMock(const std::string& meshName)
{
	if (meshName == "cube")
		return CreateCubeMock();
	else if (meshName == "skull")
		return CreateSkullMock();
	else if (meshName == "soldier")
		return CreateSoldierMock();
	else if (meshName == "grid")
		return CreateGridMock();
	else if (meshName == "cylinder")
		return CreateCylinderMock();
	else if (meshName == "sphere")
		return CreateSphereMock();
	else if (meshName == "debug")
		return CreateDebugMock();

	return ModelProperty{};
}

CreateModelNames MakeMockData()
{
	return CreateModelNames
	{
		{GraphicsPSO::Sky, { "cube" }},
		{GraphicsPSO::Opaque, { "skull" }},
		{GraphicsPSO::NormalOpaque, { "grid", "cylinder", "sphere" }},
		{GraphicsPSO::Debug, { "debug" }},
		{GraphicsPSO::SkinnedOpaque, {"soldier"}},
	};
}

void GetMockLight(PassConstants* outPc)
{
	(*outPc).nearZ = 1.0f;
	(*outPc).farZ = 1000.0f;
	(*outPc).ambientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	(*outPc).lights[0].direction = { 0.57735f, -0.57735f, 0.57735f };
	(*outPc).lights[0].strength = { 0.8f, 0.8f, 0.8f };
	(*outPc).lights[1].direction = { -0.57735f, -0.57735f, 0.57735f };
	(*outPc).lights[1].strength = { 0.4f, 0.4f, 0.4f };
	(*outPc).lights[2].direction = { 0.0f, -0.707f, -0.707f };
	(*outPc).lights[2].strength = { 0.2f, 0.2f, 0.2f };
}

ShaderFileList GetShaderFileList()
{
	ShaderFileList shaderFileList{};
	auto InsertShaderFile = [&shaderFileList](GraphicsPSO pso, ShaderType type, const std::wstring filename) {
		shaderFileList[pso].emplace_back(std::make_pair(type, filename)); };

	InsertShaderFile(GraphicsPSO::Sky, ShaderType::VS, L"Cube/VS.hlsl");
	InsertShaderFile(GraphicsPSO::Sky, ShaderType::PS, L"Cube/PS.hlsl");
	InsertShaderFile(GraphicsPSO::Opaque, ShaderType::VS, L"Opaque/VS.hlsl");
	InsertShaderFile(GraphicsPSO::Opaque, ShaderType::PS, L"Opaque/PS.hlsl");
	InsertShaderFile(GraphicsPSO::NormalOpaque, ShaderType::VS, L"NormalOpaque/VS.hlsl");
	InsertShaderFile(GraphicsPSO::NormalOpaque, ShaderType::PS, L"NormalOpaque/PS.hlsl");
	InsertShaderFile(GraphicsPSO::SkinnedOpaque, ShaderType::VS, L"Skinned/Opaque/VS.hlsl");
	InsertShaderFile(GraphicsPSO::SkinnedOpaque, ShaderType::PS, L"Skinned/Opaque/PS.hlsl");
	InsertShaderFile(GraphicsPSO::SkinnedDrawNormals, ShaderType::VS, L"Skinned/DrawNormals/VS.hlsl");
	InsertShaderFile(GraphicsPSO::SkinnedDrawNormals, ShaderType::PS, L"Skinned/DrawNormals/PS.hlsl");
	InsertShaderFile(GraphicsPSO::SkinnedShadowOpaque, ShaderType::VS, L"Skinned/Shadow/VS.hlsl");
	InsertShaderFile(GraphicsPSO::SkinnedShadowOpaque, ShaderType::PS, L"Skinned/Shadow/PS.hlsl");
	InsertShaderFile(GraphicsPSO::ShadowMap, ShaderType::VS, L"Shadow/VS.hlsl");
	InsertShaderFile(GraphicsPSO::ShadowMap, ShaderType::PS, L"Shadow/PS.hlsl");
	InsertShaderFile(GraphicsPSO::SsaoDrawNormals, ShaderType::VS, L"Ssao/DrawNormals/VS.hlsl");
	InsertShaderFile(GraphicsPSO::SsaoDrawNormals, ShaderType::PS, L"Ssao/DrawNormals/PS.hlsl");
	InsertShaderFile(GraphicsPSO::SsaoMap, ShaderType::VS, L"Ssao/Ssao/VS.hlsl");
	InsertShaderFile(GraphicsPSO::SsaoMap, ShaderType::PS, L"Ssao/Ssao/PS.hlsl");
	InsertShaderFile(GraphicsPSO::SsaoBlur, ShaderType::VS, L"Ssao/SsaoBlur/VS.hlsl");
	InsertShaderFile(GraphicsPSO::SsaoBlur, ShaderType::PS, L"Ssao/SsaoBlur/PS.hlsl");
	InsertShaderFile(GraphicsPSO::Debug, ShaderType::VS, L"Debug/VS.hlsl");
	InsertShaderFile(GraphicsPSO::Debug, ShaderType::PS, L"Debug/PS.hlsl");

	return shaderFileList;
}