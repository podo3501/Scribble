#pragma once

#include "RendererDefine.h"
#include <d3dcommon.h>
#include <DirectXCollision.h>
#include <vector>
#include <d3d12.h>

struct Material;
struct Geometry;
struct InstanceData;

struct RenderItem
{
	RenderItem() = default;

	DirectX::XMFLOAT4X4 World{};
	DirectX::XMFLOAT4X4 TexTransform{};

	int NumFramesDirty = gFrameResourceCount;
	UINT ObjCBIndex = -1;

	Material* Mat = nullptr;
	Geometry* Geo = nullptr;

	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	DirectX::BoundingBox BoundingBoxBounds{};
	DirectX::BoundingSphere BoundingSphere{};
	std::vector<InstanceData> Instances;

	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0; 
	UINT InstanceCount = 0;
};
