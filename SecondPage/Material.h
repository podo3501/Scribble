#pragma once

#include <DirectXMath.h>
#include <memory>
#include <unordered_map>
#include <string>
#include "./RendererDefine.h"

class CUploadBuffer;
struct FrameResource;

enum class TextureType : int
{
	None = 0,
	Texture,
	CubeTexture,
	Total,
};

struct Material
{
	std::string name{};
	TextureType type{ TextureType::None };
	int matCBIndex{ -1 };
	int diffuseSrvHeapIndex{ -1 };
	int normalSrvHeapIndex{ -1 };

	int numFramesDirty{ gFrameResourceCount };

	DirectX::XMFLOAT4 diffuseAlbedo{ 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 fresnelR0{ 0.01f, 0.01f, 0.01f };
	float roughness{ .25f };

	DirectX::XMMATRIX transform = DirectX::XMMatrixIdentity();
};

class CMaterial
{
public:
	CMaterial();
	~CMaterial() = default;

	CMaterial(const CMaterial&) = delete;
	CMaterial& operator=(const CMaterial&) = delete;

	void Build();
	void MakeMaterialBuffer(CUploadBuffer** outMatBuffer);

	Material* GetMaterial(const std::string& matName);
	size_t GetCount(TextureType type);
	int GetStartIndex(TextureType type);

private:
	std::vector<std::unique_ptr<Material>> m_materials{};
};
