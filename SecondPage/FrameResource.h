#pragma once

#include <wrl.h>
#include <memory>
#include <vector>

class CUploadBuffer;
class CDirectx3D;
struct ID3D12Device;
struct ID3D12CommandAllocator;

enum class eBufferType : int
{
	NoType = 0,
	PassCB,
	Material,
	Instance,
	Count,
};

class CFrameResources
{
	struct Resource
	{
		bool CreateUpdateBuffer(ID3D12Device* device, UINT passCount, UINT maxInstanceCount, UINT materialCount);

		std::unique_ptr<CUploadBuffer> passCB{ nullptr };
		std::unique_ptr<CUploadBuffer> materialBuffer{ nullptr };
		std::unique_ptr<CUploadBuffer> instanceBuffer{ nullptr };

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdListAlloc{ nullptr };
	};

public:
	CFrameResources() = default;
	CFrameResources(const CFrameResources&) = delete;
	CFrameResources& operator=(const CFrameResources&) = delete;

	bool BuildFrameResources(ID3D12Device* device,
		UINT passCount, UINT instanceCount, UINT matCount);
	bool PrepareFrame(CDirectx3D* directx3D);

	CUploadBuffer* GetUploadBuffer(eBufferType bufferType);
	inline ID3D12CommandAllocator* GetCurrCmdListAlloc() { return m_resources[m_frameResIdx]->cmdListAlloc.Get();	}
	inline void SetFence(UINT64 fenceIdx)	{ m_fenceCount = fenceIdx; }

private:
	std::vector<std::unique_ptr<Resource>> m_resources{};
	UINT m_frameResIdx{ 0 };
	UINT64 m_fenceCount{ 0 };
};