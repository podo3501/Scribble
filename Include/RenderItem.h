#pragma once

#include <wrl.h>
#include <d3dcommon.h>
#include <DirectXCollision.h>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <string>
#include <d3d12.h>

struct Material;
struct Geometry;
struct InstanceBuffer;
struct SubmeshGeometry;
enum class GraphicsPSO : int;

struct InstanceData
{
	DirectX::XMMATRIX world{};
	DirectX::XMMATRIX texTransform{};
	std::string matName{};
};

using InstanceDataList = std::vector<std::shared_ptr<InstanceData>>;

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

	InstanceDataList instanceDataList{};		//위치나 머터리얼 같은 정보들
	bool cullingFrustum{ false };		//카메라 컬링여부
	UINT instanceCount{ 0 };			//총 인스턴스
	int startSubIndexInstance{ 0 };	//서브안에서 얼마나 떨어져 있는지 
};

using SubRenderItems = std::unordered_map<std::string, SubRenderItem>;

struct RenderItem
{
	RenderItem();

	D3D12_PRIMITIVE_TOPOLOGY primitiveType{ D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST };
	
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferGPU{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferUploader{ nullptr };
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};

	Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferGPU{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferUploader{ nullptr };
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};

	int startIndexInstance{ 0 };

	SubRenderItems subRenderItems{};

	int NumFramesDirty{ 0 };

	// We can free this memory after we finish upload to the GPU.
	void DisposeUploaders()
	{
		vertexBufferUploader.Reset();
		indexBufferUploader.Reset();
	}
};

using AllRenderItems = std::map<GraphicsPSO, std::unique_ptr<RenderItem>>;
