#pragma once

#include <wrl.h>
#include <memory>
#include <vector>

class CRenderer;
class CUploadBuffer;
struct ID3D12Device;
struct ID3D12CommandAllocator;
struct ID3D12Resource;
enum class eBufferType;

class CFrameResources
{
	struct Resource
	{
		Resource();
		~Resource();
		bool CreateUpdateBuffer(ID3D12Device* device, UINT passCount, UINT maxInstanceCount, UINT materialCount);

		std::unique_ptr<CUploadBuffer> passCB;
		std::unique_ptr<CUploadBuffer> ssaoCB;
		std::unique_ptr<CUploadBuffer> skinnedCB;
		std::unique_ptr<CUploadBuffer> instanceBuffer;
		std::unique_ptr<CUploadBuffer> materialBuffer;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdListAlloc;
	};

public:
	CFrameResources();
	~CFrameResources();

	CFrameResources(const CFrameResources&) = delete;
	CFrameResources& operator=(const CFrameResources&) = delete;

	bool BuildFrameResources(ID3D12Device* device,
		UINT passCount, UINT instanceCount, UINT matCount);
	bool PrepareFrame(CRenderer* renderer);
	bool SetUploadBuffer(eBufferType bufferType, const void* bufferData, size_t dataSize);

	inline ID3D12CommandAllocator* GetCurrCmdListAlloc() { return m_resources[m_frameResIdx]->cmdListAlloc.Get();	}
	inline void SetFence(UINT64 fenceIdx)	{ m_fenceCount = fenceIdx; }
	ID3D12Resource* GetResource(eBufferType bufferType);
	UINT GetBufferSize(eBufferType bufferType);

private:
	CUploadBuffer* GetUploadBuffer(eBufferType bufferType);

private:
	std::vector<std::unique_ptr<Resource>> m_resources;
	UINT m_frameResIdx{ 0 };
	UINT64 m_fenceCount{ 0 };
};

