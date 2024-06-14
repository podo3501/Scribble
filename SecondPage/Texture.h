#pragma once

#include<string>
#include<vector>
#include<memory>

class CRenderer;
struct ID3D12Device;
struct ID3D12DescriptorHeap;
struct Texture;

class CTexture
{
public:
	CTexture(const std::wstring& filePath);

	void Load(CRenderer* renderer);

	CTexture(const CTexture&) = delete;
	CTexture& operator=(const CTexture&) = delete;

private:
	void CreateShaderResourceView(ID3D12Device* device, ID3D12DescriptorHeap* srvDescHeap);

private:
	std::wstring m_filePath{};

	std::vector<std::unique_ptr<Texture>> m_textureList{};
};