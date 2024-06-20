#include "FrameResource.h"
#include <d3d12.h>
#include "../Core/d3dUtil.h"
#include "../Core/UploadBuffer.h"

bool CFrameResources::Resource::CreateUpdateBuffer(
	ID3D12Device* device, UINT passCount, UINT maxInstanceCount, UINT materialCount)
{
	ReturnIfFailed(device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(CmdListAlloc.GetAddressOf())));

	PassCB = std::make_unique<UploadBuffer>(device, sizeof(PassConstants), passCount, true);
	MaterialBuffer = std::make_unique<UploadBuffer>(device, sizeof(MaterialData), materialCount, false);
	InstanceBuffer = std::make_unique<UploadBuffer>(device, sizeof(InstanceData), maxInstanceCount, false);

	return true;
}

bool CFrameResources::BuildFrameResources(ID3D12Device* device,
	UINT passCount, UINT instanceCount, UINT matCount)
{
	for (auto i{ 0 }; i < FrameResourceCount; ++i)
	{
		auto frameRes = std::make_unique<Resource>();
		ReturnIfFalse(frameRes->CreateUpdateBuffer(device, passCount, instanceCount, matCount));
		m_resources.emplace_back(std::move(frameRes));
	}

	return true;
}

bool CFrameResources::Synchronize(ID3D12Fence* pFence)
{
	m_frameResIdx = (m_frameResIdx + 1) % gNumFrameResources;
	if (m_fenceIdx != 0 && pFence->GetCompletedValue() < m_fenceIdx)
	{
		HANDLE hEvent = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		ReturnIfFailed(pFence->SetEventOnCompletion(m_fenceIdx, hEvent));
		WaitForSingleObject(hEvent, INFINITE);
		CloseHandle(hEvent);
	}

	return true;
}

UploadBuffer* CFrameResources::GetUploadBuffer(eBufferType bufferType)
{
	Resource* resource = m_resources[m_frameResIdx].get();
	if (resource == nullptr) return nullptr;

	switch (bufferType)
	{
	case eBufferType::PassCB:			return resource->PassCB.get();
	case eBufferType::Material:		return resource->MaterialBuffer.get();
	case eBufferType::Instance:		return resource->InstanceBuffer.get();
	}

	return nullptr;
}