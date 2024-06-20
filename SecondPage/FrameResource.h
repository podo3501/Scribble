#pragma once

#include "./FrameResourceData.h"
#include <memory>
#include <vector>

class UploadBuffer;
struct ID3D12Fence;
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

		std::unique_ptr<UploadBuffer> PassCB{ nullptr };
		std::unique_ptr<UploadBuffer> MaterialBuffer{ nullptr };
		std::unique_ptr<UploadBuffer> InstanceBuffer{ nullptr };

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc{ nullptr };
	};

public:
	static const int FrameResourceCount = 3;

	CFrameResources() = default;
	CFrameResources(const CFrameResources&) = delete;
	CFrameResources& operator=(const CFrameResources&) = delete;

	bool BuildFrameResources(ID3D12Device* device,
		UINT passCount, UINT instanceCount, UINT matCount);
	bool Synchronize(ID3D12Fence* pFence);

	UploadBuffer* GetUploadBuffer(eBufferType bufferType);
	inline ID3D12CommandAllocator* GetCurrCmdListAlloc() { return m_resources[m_frameResIdx]->CmdListAlloc.Get();	}
	inline void SetFence(UINT64 fenceIdx)	{ m_fenceIdx = fenceIdx; }

private:
	std::vector<std::unique_ptr<Resource>> m_resources{};
	UINT m_frameResIdx{ 0 };
	UINT64 m_fenceIdx{ 0 };
};