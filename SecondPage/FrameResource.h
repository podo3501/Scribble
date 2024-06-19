﻿#pragma once

#include "./FrameResourceData.h"
#include <memory>
#include <vector>

class UploadBuffer;
struct ID3D12Fence;
struct ID3D12Device;
struct ID3D12CommandAllocator;

// Stores the resources needed for the CPU to build the command lists
// for a frame.  
struct FrameResource
{
public:
    
    FrameResource(ID3D12Device* device, UINT passCount, UINT maxInstanceCount, UINT materialCount);
    FrameResource(const FrameResource& rhs) = delete;
    FrameResource& operator=(const FrameResource& rhs) = delete;
    ~FrameResource();

    // We cannot reset the allocator until the GPU is done processing the commands.
    // So each frame needs their own allocator.
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

    // We cannot update a cbuffer until the GPU is done processing the commands
    // that reference it.  So each frame needs their own cbuffers.
    std::unique_ptr<UploadBuffer> PassCB{ nullptr };
    std::unique_ptr<UploadBuffer> MaterialBuffer{ nullptr };
    std::unique_ptr<UploadBuffer> InstanceBuffer{ nullptr };

    // Fence value to mark commands up to this fence point.  This lets us
    // check if these frame resources are still in use by the GPU.
    UINT64 Fence = 0;
};

enum class eBufferType : int
{
	NoType = 0,
	PassCB,
	Material,
	Instance,
	Count,
};

class CFrameResource
{
	static const int FrameResourceCount = 3;

public:
	CFrameResource() = default;

	CFrameResource(const CFrameResource&) = delete;
	CFrameResource& operator=(const CFrameResource&) = delete;

	bool BuildFrameResource(ID3D12Device* device,
		UINT passCount, UINT instanceCount, UINT matCount);
	void Synchronize(ID3D12Fence* pFence);
	UploadBuffer* GetUploadBuffer(eBufferType bufferType);

private:
	std::vector<std::unique_ptr<FrameResource>> m_resources{};
	FrameResource* m_curFrameRes{ nullptr };
	UINT m_frameResIdx{ 0 };
};