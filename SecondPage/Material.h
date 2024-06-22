#pragma once

#include <DirectXMath.h>
#include <memory>
#include <unordered_map>
#include <string>
#include "./RendererDefine.h"

struct FrameResource;

// Simple struct to represent a material for our demos.  A production 3D engine
// would likely create a class hierarchy of Materials.
struct Material
{
	// Unique material name for lookup.
	std::string name;

	// Index into constant buffer corresponding to this material.
	int matCBIndex = -1;

	// Index into SRV heap for diffuse texture.
	int diffuseSrvHeapIndex = -1;

	// Index into SRV heap for normal texture.
	int normalSrvHeapIndex = -1;

	// Dirty flag indicating the material has changed and we need to update the constant buffer.
	// Because we have a material constant buffer for each FrameResource, we have to apply the
	// update to each FrameResource.  Thus, when we modify a material we should set 
	// NumFramesDirty = gFrameResourceCount so that each frame resource gets the update.
	int numFramesDirty = gFrameResourceCount;

	// Material constant buffer data used for shading.
	DirectX::XMFLOAT4 diffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 fresnelR0 = { 0.01f, 0.01f, 0.01f };
	float roughness = .25f;
	DirectX::XMFLOAT4X4 matTransform = {};
};

class CMaterial
{
public:
	CMaterial();
	~CMaterial() = default;

	CMaterial(const CMaterial&) = delete;
	CMaterial& operator=(const CMaterial&) = delete;

	void Build();

	Material* GetMaterial(const std::string& matName);
	size_t GetCount();
	inline const auto& GetTotalMaterial() { return m_materials; };

private:
	std::unordered_map<std::string, std::unique_ptr<Material>> m_materials{};
};
