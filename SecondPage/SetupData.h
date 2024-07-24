#pragma once

#include <wrl.h>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <unordered_map>

interface IRenderer;
class CMaterial;
class CMesh;
class CSkinnedMesh;
struct MeshData;
struct Material;
struct RenderItem;
struct InstanceData;
struct PassConstants;
enum class SrvOffset : int;
enum class GraphicsPSO : int; 

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
	using ModelProperties = std::unordered_map<std::string, ModelProperty>;
	using AllModelProperty = std::map<GraphicsPSO, ModelProperties>;
	using AllRenderItems = std::map<GraphicsPSO, std::unique_ptr<RenderItem>>;

public:
	CSetupData();
	~CSetupData();

	CSetupData(const CSetupData&) = delete;
	CSetupData& operator=(const CSetupData&) = delete;

	bool InsertModelProperty(
		GraphicsPSO pso,
		const std::string& meshName, 
		ModelProperty&& mProperty, 
		CMaterial* material);
	bool LoadMesh(CMesh* mesh, CSkinnedMesh* skinnedMesh, AllRenderItems* renderItems);

private:
	bool LoadMesh(CMesh* mesh, CSkinnedMesh* skinnedMesh, GraphicsPSO pso, ModelProperties& modelProp);
	bool FillRenderItems(AllRenderItems* renderItems);

private:
	AllModelProperty m_allModelProperty;
};

