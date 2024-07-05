#pragma once

#include <DirectXMath.h>
#include <memory>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <combaseapi.h>

interface IRenderer;
struct MaterialBuffer;
enum class eTextureType : int;

struct Material
{
	Material();

	std::string name{};
	eTextureType type;
	std::wstring filename{};
	UINT diffuseIndex{ 0 };
	UINT normalSrvHeapIndex{ 0 };	//normal map

	DirectX::XMFLOAT4 diffuseAlbedo{ 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 fresnelR0{ 0.01f, 0.01f, 0.01f };
	float roughness{ .25f };

	DirectX::XMMATRIX transform = DirectX::XMMatrixIdentity();

	int numFramesDirty;
};

using MaterialList = std::vector<std::shared_ptr<Material>>;

class CMaterial
{
	using TextureList = std::map<eTextureType, std::set<std::wstring>>;
	
public:
	CMaterial();
	~CMaterial();

	CMaterial(const CMaterial&) = delete;
	CMaterial& operator=(const CMaterial&) = delete;

	void SetMaterialList(const MaterialList& materialList);
	bool LoadTextureIntoVRAM(IRenderer* renderer);
	void MakeMaterialBuffer(IRenderer* renderer);

	int GetDiffuseIndex(const std::wstring& filename);
	int GetMaterialIndex(const std::string& matName);

private:
	MaterialBuffer ConvertUploadBuffer(UINT diffuseIndex, Material* material);

private:
	MaterialList m_materialList{};
	TextureList m_textures{};
};
