#pragma once

#include <DirectXMath.h>
#include <memory>
#include <unordered_map>
#include <string>
#include "./RendererDefine.h"

class CUploadBuffer;
struct FrameResource;

struct Material
{
	std::string name{};
	int matCBIndex{ -1 };
	int diffuseSrvHeapIndex{ -1 };
	int normalSrvHeapIndex{ -1 };

	// Dirty flag indicating the material has changed and we need to update the constant buffer.
	// Because we have a material constant buffer for each FrameResource, we have to apply the
	// update to each FrameResource.  Thus, when we modify a material we should set 
	// NumFramesDirty = gFrameResourceCount so that each frame resource gets the update.
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
	size_t GetCount();

private:
	std::unordered_map<std::string, std::unique_ptr<Material>> m_materials{};
};
