#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <wrl.h>
#include <memory>

interface IRenderer;
class CRenderer;
struct ID3D12Device;
struct ID3D12Resource;
struct ID3D12GraphicsCommandList;
struct ID3D12DescriptorHeap;

enum class eTextureType : int;

class CTexture
{
	using TextureList = std::vector<std::pair<eTextureType, std::wstring>>;

public:
	CTexture(std::wstring resPath);
	~CTexture();

	CTexture() = delete;
	CTexture(const CTexture&) = delete;
	CTexture& operator=(const CTexture&) = delete;

	void CreateShaderResourceView(CRenderer* renderer);
	bool Upload(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const TextureList& textureList);

private:
	struct TextureMemory
	{
		TextureMemory();
		~TextureMemory();

		//std::string name{};// Unique material name for lookup.
		std::wstring path{};
		std::wstring filename{};

		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		Microsoft::WRL::ComPtr<ID3D12Resource> uploadHeap;
	};

private:
	std::wstring m_resPath{};
	const std::wstring m_filePath;

	std::vector<std::pair<eTextureType, std::unique_ptr<TextureMemory>>> m_textureMemories;
};

