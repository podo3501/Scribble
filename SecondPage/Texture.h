#pragma once

#include <string>
#include <vector>
#include <map>
#include <wrl.h>
#include <memory>

interface IRenderer;
struct ID3D12Device;
struct ID3D12Resource;
struct ID3D12GraphicsCommandList;
struct ID3D12DescriptorHeap;

enum class eTextureType : int
{
	None = 0,
	Cube,
	Common,
};

class CTexture
{
public:
	CTexture(std::wstring resPath);
	~CTexture();

	bool LoadTexture(IRenderer* renderer, eTextureType type, std::vector<std::wstring>& filenames);

	CTexture() = delete;
	CTexture(const CTexture&) = delete;
	CTexture& operator=(const CTexture&) = delete;

private:
	bool Upload(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, eTextureType type, std::vector<std::wstring>& filenames);
	void CreateShaderResourceView(IRenderer* renderer, eTextureType type);

private:
	struct TextureMemory
	{
		std::string name{};// Unique material name for lookup.
		std::wstring filename{};

		Microsoft::WRL::ComPtr<ID3D12Resource> resource{ nullptr };
		Microsoft::WRL::ComPtr<ID3D12Resource> uploadHeap{ nullptr };
	};

private:
	std::wstring m_resPath{};
	const std::wstring m_filePath{ L"Textures/" };

	std::map<eTextureType, std::vector<std::unique_ptr<TextureMemory>>> m_texMemories{};
	int m_skyTexHeapIndex{ 0 };
	int m_offsetIndex{ 0 };
};

