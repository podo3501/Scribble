#pragma once

#include <wrl.h>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <DirectXMath.h>

class CMaterial;
class CModel;
struct RenderItem;
struct InstanceData;

using InstanceDataList = std::vector<std::shared_ptr<InstanceData>>;

struct ModelProperty
{
	enum class CreateType : int
	{
		None,
		Generator,
		ReadFile,
	};

	CreateType createType{ CreateType::None };
	std::wstring filename{};
	InstanceDataList instanceDataList{};
	bool cullingFrustum{ false };
};

using MeshProperty = std::unordered_map<std::string, ModelProperty>;
using AllModelProperty = std::unordered_map<std::string, MeshProperty>;
using AllRenderItems = std::unordered_map<std::string, std::unique_ptr<RenderItem>>;

class CInstance
{
public:
	CInstance();
	~CInstance();

	CInstance(const CInstance&) = delete;
	CInstance& operator=(const CInstance&) = delete;

	bool CreateMockData();
	bool LoadModel(CModel* model);
	bool FillRenderItems(AllRenderItems& renderItems);

private:
	bool Insert(const std::string& geoName, const std::string& meshName, ModelProperty& mProperty);
	bool LoadMesh(CModel* model, const std::string& geoName, MeshProperty& meshProp);

private:
	AllModelProperty m_allModelProperty{};
};

