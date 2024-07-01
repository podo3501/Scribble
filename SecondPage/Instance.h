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
class CTexture;
struct RenderItem;
struct InstanceData;
enum class eTextureType : int;

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

class CInstance
{
	using MeshProperty = std::unordered_map<std::string, ModelProperty>;
	using AllModelProperty = std::unordered_map<std::string, MeshProperty>;
	using AllRenderItems = std::unordered_map<std::string, std::unique_ptr<RenderItem>>;

	using TypeTextures = std::pair<eTextureType, std::vector<std::wstring>>;
	using TextureList = std::vector<TypeTextures>;

public:
	CInstance();
	~CInstance();

	CInstance(const CInstance&) = delete;
	CInstance& operator=(const CInstance&) = delete;

	bool CreateMockData();
	bool LoadModel(CModel* model);
	bool LoadTextureIntoVRAM(IRenderer* renderer, CTexture* texture);
	bool FillRenderItems(AllRenderItems& renderItems);

private:
	bool CreateModelMock();
	bool CreateTextureMock();

	InstanceDataList CreateSkullInstanceData();
	bool Insert(const std::string& geoName, const std::string& meshName, ModelProperty& mProperty);
	bool LoadMesh(CModel* model, const std::string& geoName, MeshProperty& meshProp);

	int GetTextureCount(eTextureType texType);

private:
	AllModelProperty m_allModelProperty{};
	TextureList m_textureList{};
};

