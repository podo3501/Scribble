#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <wrl.h>
#include <memory>

class CDescriptorHeap;
struct ID3D12Device;
struct ID3D12Resource;
struct ID3D12GraphicsCommandList;
struct ID3D12DescriptorHeap;

enum class SrvOffset : int;

class CTexture
{
	using TextureList = std::vector<std::pair<SrvOffset, std::wstring>>;

public:
	CTexture(std::wstring resPath);
	~CTexture();

	CTexture() = delete;
	CTexture(const CTexture&) = delete;
	CTexture& operator=(const CTexture&) = delete;

	void CreateShaderResourceView(CDescriptorHeap* descHeap);
	bool Upload(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const TextureList& textureList);
	inline std::vector<std::wstring> GetListSrvTexture2D();
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

	std::vector<std::pair<SrvOffset, std::unique_ptr<TextureMemory>>> m_textureMemories;
	std::vector<std::wstring> m_srvTexture2DFilename{};
};

inline std::vector<std::wstring> CTexture::GetListSrvTexture2D() {	return m_srvTexture2DFilename; }

