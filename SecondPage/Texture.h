#pragma once

#include <string>
#include <vector>
#include <wrl.h>
#include <memory>

class CDirectx3D;
class CRenderer;
struct ID3D12Device;
struct ID3D12Resource;
struct ID3D12GraphicsCommandList;
struct ID3D12DescriptorHeap;

class CTexture
{
public:
	template<typename T>
	CTexture(T&& resPath)
		: m_resPath(std::forward<T>(resPath))
	{}

	bool LoadGraphicMemory(CDirectx3D* directx3D, CRenderer* renderer);

	CTexture() = delete;
	CTexture(const CTexture&) = delete;
	CTexture& operator=(const CTexture&) = delete;

private:
	bool Upload(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
	void CreateShaderResourceView(ID3D12Device* device, ID3D12DescriptorHeap* srvDescHeap);

private:
	struct Data
	{
		std::string Name{};// Unique material name for lookup.
		std::wstring Filename{};

		Microsoft::WRL::ComPtr<ID3D12Resource> Resource{ nullptr };
		Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap{ nullptr };
	};

private:
	std::wstring m_resPath{};
	const std::wstring m_filePath{ L"Textures/" };

	std::vector<std::unique_ptr<Data>> m_textureList{};
};

