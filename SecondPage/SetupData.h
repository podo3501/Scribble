#pragma once

#include <wrl.h>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <DirectXMath.h>

interface IRenderer;
class CMaterial;
class CModel;
struct MeshData;
struct Material;
struct RenderItem;
struct InstanceData;
struct PassConstants;
enum class eTextureType : int;

using InstanceDataList = std::vector<std::shared_ptr<InstanceData>>;
using MaterialList = std::vector<std::shared_ptr<Material>>;

struct ModelProperty
{
	enum class CreateType : int
	{
		None,
		Generator,
		ReadFile,
	};

	CreateType createType{ CreateType::None };
	std::unique_ptr<MeshData> meshData{ nullptr };
	std::wstring filename{};
	InstanceDataList instanceDataList{};
	bool cullingFrustum{ false };
	MaterialList materialList{};
};

class CSetupData
{
	using MeshProperty = std::unordered_map<std::string, ModelProperty>;
	using AllModelProperty = std::unordered_map<std::string, MeshProperty>;
	using AllRenderItems = std::unordered_map<std::string, std::unique_ptr<RenderItem>>;

	using TypeTextures = std::pair<eTextureType, std::vector<std::wstring>>;
	using TextureList = std::vector<TypeTextures>;

public:
	CSetupData();
	~CSetupData();

	CSetupData(const CSetupData&) = delete;
	CSetupData& operator=(const CSetupData&) = delete;

	bool CreateMockData();
	bool N_InsertModelProperty(
		const std::string& geoName, 
		const std::string& meshName, 
		ModelProperty&& mProperty, 
		CMaterial* material);
	bool InsertModelProperty(const std::string& geoName, const std::string& meshName, ModelProperty& mProperty);
	bool LoadModel(CModel* model, AllRenderItems* renderItems);
	bool LoadTextureIntoVRAM(IRenderer* renderer, CMaterial* material);
	bool LoadTextureIntoVRAM(IRenderer* renderer);
	void GetPassCB(PassConstants* outPc);

private:
	bool CreateModelMock();
	bool CreateMaterialMock();
	bool CreateTextureMock();

	InstanceDataList CreateSkullInstanceData();
	bool LoadMesh(CModel* model, const std::string& geoName, MeshProperty& meshProp);

	int GetTextureCount(eTextureType texType);
	bool FillRenderItems(AllRenderItems* renderItems);

private:
	AllModelProperty m_allModelProperty{};
	TextureList m_textureList{};
	MaterialList m_materialList{};
};

