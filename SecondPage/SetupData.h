#pragma once

#include <wrl.h>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

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

public:
	CSetupData();
	~CSetupData();

	CSetupData(const CSetupData&) = delete;
	CSetupData& operator=(const CSetupData&) = delete;

	bool InsertModelProperty(
		const std::string& geoName, 
		const std::string& meshName, 
		ModelProperty&& mProperty, 
		CMaterial* material);
	bool LoadModel(CModel* model, AllRenderItems* renderItems);
	void GetPassCB(PassConstants* outPc);

private:
	bool LoadMesh(CModel* model, const std::string& geoName, MeshProperty& meshProp);
	bool FillRenderItems(AllRenderItems* renderItems);

private:
	AllModelProperty m_allModelProperty{};
};

