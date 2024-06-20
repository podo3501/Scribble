#pragma once

#include<string>
#include<vector>
#include<memory>

class CDirectx3D;
class CRenderer;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct ID3D12DescriptorHeap;
struct Texture;

class CTexture
{
public:
	template<typename T>
	CTexture(T&& filePath)
		: m_filePath(std::forward<T>(filePath))
	{}

	bool LoadGraphicMemory(CDirectx3D* directx3D, CRenderer* renderer);

	CTexture() = delete;
	CTexture(const CTexture&) = delete;
	CTexture& operator=(const CTexture&) = delete;

private:
	bool Upload(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
	void CreateShaderResourceView(ID3D12Device* device, ID3D12DescriptorHeap* srvDescHeap);

private:
	std::wstring m_filePath{};

	std::vector<std::unique_ptr<Texture>> m_textureList{};
};

