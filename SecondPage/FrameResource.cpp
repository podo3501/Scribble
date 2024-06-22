#include "FrameResource.h"
#include <d3d12.h>
#include "../Core/d3dUtil.h"
#include "../Core/UploadBuffer.h"
#include "../Core/Directx3D.h"
#include "./RendererDefine.h"

bool CFrameResources::Resource::CreateUpdateBuffer(
	ID3D12Device* device, UINT passCount, UINT maxInstanceCount, UINT materialCount)
{
	ReturnIfFailed(device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(cmdListAlloc.GetAddressOf())));

	passCB = std::make_unique<CUploadBuffer>(device, sizeof(PassConstants), passCount, true);
	materialBuffer = std::make_unique<CUploadBuffer>(device, sizeof(MaterialBuffer), materialCount, false);
	instanceBuffer = std::make_unique<CUploadBuffer>(device, sizeof(InstanceBuffer), maxInstanceCount, false);

	return true;
}

bool CFrameResources::BuildFrameResources(ID3D12Device* device,
	UINT passCount, UINT instanceCount, UINT matCount)
{
	for (auto i{ 0 }; i < gFrameResourceCount; ++i)
	{
		auto frameRes = std::make_unique<Resource>();
		ReturnIfFalse(frameRes->CreateUpdateBuffer(device, passCount, instanceCount, matCount));
		m_resources.emplace_back(std::move(frameRes));
	}

	return true;
}

bool CFrameResources::PrepareFrame(CDirectx3D* directx3D)
{
	m_frameResIdx = (m_frameResIdx + 1) % gFrameResourceCount;
	if (m_fenceCount == 0)
		return true;
	ReturnIfFalse(directx3D->WaitUntilGpuFinished(m_fenceCount));

	return true;
}

CUploadBuffer* CFrameResources::GetUploadBuffer(eBufferType bufferType)
{
	Resource* resource = m_resources[m_frameResIdx].get();
	if (resource == nullptr) return nullptr;

	switch (bufferType)
	{
	case eBufferType::PassCB:			return resource->passCB.get();
	case eBufferType::Material:		return resource->materialBuffer.get();
	case eBufferType::Instance:		return resource->instanceBuffer.get();
	}

	return nullptr;
}