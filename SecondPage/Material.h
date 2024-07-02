#pragma once

#include <DirectXMath.h>
#include <memory>
#include <unordered_map>
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
public:
	CMaterial();
	~CMaterial();

	CMaterial(const CMaterial&) = delete;
	CMaterial& operator=(const CMaterial&) = delete;

	void MakeMaterialBuffer(IRenderer* renderer);
	inline void SetMaterialList(const MaterialList& materialList) { m_materialList = materialList; };

private:
	MaterialBuffer ConvertUploadBuffer(UINT diffuseIndex, Material* material);

private:
	MaterialList m_materialList{};
};
