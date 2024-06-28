#pragma once

#include "RendererDefine.h"
#include <d3dcommon.h>
#include <DirectXCollision.h>
#include <vector>
#include <d3d12.h>

struct Material;
struct Geometry;
struct InstanceBuffer;
struct SubmeshGeometry;

struct InstanceData
{
	DirectX::XMMATRIX world{};
	DirectX::XMMATRIX texTransform{};
	UINT matIndex{ 0u };
};

using InstanceDataList = std::vector<std::shared_ptr<InstanceData>>;

struct InstanceInfo
{
	InstanceInfo() = default;

	InstanceDataList instanceDataList{};
	bool cullingFrustum{ false };
};

struct SubItem
{
	UINT indexCount{ 0u };
	UINT startIndexLocation{ 0u };
	INT baseVertexLocation{ 0 };

	DirectX::BoundingBox boundingBox{};
	DirectX::BoundingSphere boundingSphere{};
};

struct SubRenderItem
{
	SubRenderItem() = default;

	SubItem subItem{};

	InstanceInfo instanceInfo{};
	UINT instanceCount{ 0 };
	int startIndexInstance{ 0 };
};

using SubRenderItems = std::unordered_map<std::string, SubRenderItem>;

struct RenderItem
{
	RenderItem() = default;

	D3D12_PRIMITIVE_TOPOLOGY primitiveType{ D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST };
	
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferGPU{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferUploader{ nullptr };
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};

	Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferGPU{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferUploader{ nullptr };
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};

	SubRenderItems subRenderItems{};

	int NumFramesDirty{ gFrameResourceCount };

	// We can free this memory after we finish upload to the GPU.
	void DisposeUploaders()
	{
		vertexBufferUploader.Reset();
		indexBufferUploader.Reset();
	}
};

using AllRenderItems = std::unordered_map<std::string, std::unique_ptr<RenderItem>>;
