﻿#include "FrameResource.h"
#include <d3d12.h>
#include "../Core/d3dUtil.h"
#include "./Interface.h"
#include "./UploadBuffer.h"
#include "./RendererDefine.h"
#include "./FrameResourceData.h"

CFrameResources::CFrameResources() = default;
CFrameResources::~CFrameResources() = default;

bool CFrameResources::Resource::CreateUpdateBuffer(
	ID3D12Device* device, UINT passCount, UINT maxInstanceCount, UINT materialCount)
{
	ReturnIfFailed(device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(cmdListAlloc.GetAddressOf())));

	passCB = std::make_unique<CUploadBuffer>(device, sizeof(PassConstants), passCount, true);
	instanceBuffer = std::make_unique<CUploadBuffer>(device, sizeof(InstanceBuffer), maxInstanceCount, false);
	materialBuffer = std::make_unique<CUploadBuffer>(device, sizeof(MaterialBuffer), materialCount, false);

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

bool CFrameResources::PrepareFrame(IRenderer* renderer)
{
	m_frameResIdx = (m_frameResIdx + 1) % gFrameResourceCount;
	if (m_fenceCount == 0)
		return true;
	ReturnIfFalse(renderer->WaitUntilGpuFinished(m_fenceCount));

	return true;
}

CUploadBuffer* CFrameResources::GetUploadBuffer(eBufferType bufferType)
{
	Resource* resource = m_resources[m_frameResIdx].get();
	if (resource == nullptr) return nullptr;

	switch (bufferType)
	{
	case eBufferType::PassCB:			return resource->passCB.get();
	case eBufferType::Instance:		return resource->instanceBuffer.get();
	case eBufferType::Material:		return resource->materialBuffer.get();
	}

	return nullptr;
}