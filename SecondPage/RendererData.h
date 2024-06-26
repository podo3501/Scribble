#pragma once

#include "RendererDefine.h"
#include <d3dcommon.h>
#include <DirectXCollision.h>
#include <vector>
#include <d3d12.h>

struct Material;
struct Geometry;
struct InstanceBuffer;

struct InstanceData
{
	DirectX::XMMATRIX world{};
	DirectX::XMMATRIX texTransform{};
	UINT matIndex{ 0u };
};

struct RenderItem
{
	RenderItem() = default;

	DirectX::XMFLOAT4X4 world{};
	DirectX::XMFLOAT4X4 texTransform{};

	int NumFramesDirty{ gFrameResourceCount };
	UINT objCBIndex = -1;

	Material* mat{ nullptr };
	Geometry* geo{ nullptr };

	D3D12_PRIMITIVE_TOPOLOGY primitiveType{ D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST };

	DirectX::BoundingBox boundingBox{};
	DirectX::BoundingSphere boundingSphere{};
	std::vector<std::shared_ptr<InstanceData>> instances{};

	UINT indexCount{ 0 };
	UINT startIndexLocation{ 0 };
	int baseVertexLocation{ 0 };
	UINT instanceCount{ 0 };
};
