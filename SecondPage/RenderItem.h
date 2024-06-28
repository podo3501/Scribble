#pragma once

#include "RendererDefine.h"
#include "./SubItem.h"
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

struct RenderItem
{
	RenderItem() = default;

	DirectX::BoundingBox boundingBox{};
	DirectX::BoundingSphere boundingSphere{};
	std::vector<std::shared_ptr<InstanceData>> instances{};
	bool cullingFrustum{ false };

	int baseVertexLocation{ 0 };

	UINT startIndexLocation{ 0 };
	UINT indexCount{ 0 };
	
	UINT instanceCount{ 0 };
	int startIndexInstance{ 0 };


	D3D12_PRIMITIVE_TOPOLOGY primitiveType{ D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST };

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};

	int NumFramesDirty{ gFrameResourceCount };
};

struct SubRenderItem
{
	SubRenderItem() = default;

	SubItem subItem{};

	std::vector<std::shared_ptr<InstanceData>> instances{};
	bool cullingFrustum{ false };
	UINT instanceCount{ 0 };
	int startIndexInstance{ 0 };
};

struct NRenderItem
{
	NRenderItem() = default;

	D3D12_PRIMITIVE_TOPOLOGY primitiveType{ D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST };
	
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferGPU{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferUploader{ nullptr };
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};

	Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferGPU{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferUploader{ nullptr };
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};

	std::unordered_map<std::string, SubRenderItem> subRenderItems{};

	int NumFramesDirty{ gFrameResourceCount };

	// We can free this memory after we finish upload to the GPU.
	void DisposeUploaders()
	{
		vertexBufferUploader.Reset();
		indexBufferUploader.Reset();
	}
};
