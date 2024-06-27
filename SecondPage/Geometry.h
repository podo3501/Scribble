#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <d3dcommon.h>
#include <DirectXCollision.h>
#include <memory>
#include <unordered_map>
#include <string>

class CDirectx3D;
struct SubItem;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct ID3D12Resource;

struct Geometry
{
	std::string name{};

	Microsoft::WRL::ComPtr<ID3DBlob> vertexBufferCPU{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferGPU{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferUploader{ nullptr };

	Microsoft::WRL::ComPtr<ID3DBlob> indexBufferCPU{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferGPU{ nullptr };
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferUploader{ nullptr };

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};

	std::unordered_map<std::string, std::shared_ptr<SubItem>> drawArgs{};

	// We can free this memory after we finish upload to the GPU.
	void DisposeUploaders()
	{
		vertexBufferUploader = nullptr;
		indexBufferUploader = nullptr;
	}
};

class CGeometry
{
public:
	CGeometry();

	CGeometry(const CGeometry&) = delete;
	CGeometry& operator=(const CGeometry&) = delete;

	bool LoadGraphicMemory(CDirectx3D* directx3D);
	Geometry* GetGeometry(const std::string& geoName);

private:
	bool Load(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, Geometry* meshGeo);

private:
	std::unordered_map<std::string, std::unique_ptr<Geometry>> m_geometries{};
};