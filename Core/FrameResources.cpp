#include "./FrameResources.h"
#include <d3d12.h>
#include <ranges>
#include "./d3dUtil.h"
#include "./UploadBuffer.h"
#include "./Renderer.h"
#include "../Include/RendererDefine.h"
#include "../Include/FrameResourceData.h"

CFrameResources::Resource::Resource()
	: passCB{ nullptr }
	, ssaoCB{ nullptr }
	, skinnedCB{ nullptr }
	, instanceBuffer{ nullptr }
	, materialBuffer{ nullptr }
	, cmdListAlloc{ nullptr }
{}
CFrameResources::Resource::~Resource() = default;

CFrameResources::CFrameResources()
	: m_resources{}
{}
CFrameResources::~CFrameResources() = default;

bool CFrameResources::Resource::CreateUpdateBuffer(
	ID3D12Device* device, UINT passCount, UINT maxInstanceCount, UINT materialCount)
{
	ReturnIfFailed(device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(cmdListAlloc.GetAddressOf())));

	passCB = std::make_unique<CUploadBuffer>(sizeof(PassConstants), passCount, true);
	ssaoCB = std::make_unique<CUploadBuffer>(sizeof(SsaoConstants), 1, true);
	skinnedCB = std::make_unique<CUploadBuffer>(sizeof(SkinnedConstants), 1, true);
	instanceBuffer = std::make_unique<CUploadBuffer>(sizeof(InstanceBuffer), maxInstanceCount, false);
	materialBuffer = std::make_unique<CUploadBuffer>(sizeof(MaterialBuffer), materialCount, false);

	ReturnIfFalse(passCB->Initialize(device));
	ReturnIfFalse(ssaoCB->Initialize(device));
	ReturnIfFalse(skinnedCB->Initialize(device));
	ReturnIfFalse(instanceBuffer->Initialize(device));
	ReturnIfFalse(materialBuffer->Initialize(device));

	return true;
}

bool CFrameResources::Build(ID3D12Device* device,
	UINT passCount, UINT instanceCount, UINT matCount)
{
	for (auto i : std::views::iota(0, gFrameResourceCount))
	{
		auto frameRes = std::make_unique<Resource>();
		ReturnIfFalse(frameRes->CreateUpdateBuffer(device, passCount, instanceCount, matCount));
		m_resources.emplace_back(std::move(frameRes));
	}

	return true;
}

bool CFrameResources::SetUploadBuffer(eBufferType bufferType, const void* bufferData, size_t dataSize)
{
	if (dataSize == 0)
		return false;

	CUploadBuffer* uploadBuffer = GetUploadBuffer(bufferType);
	uploadBuffer->CopyDataList(bufferData, dataSize);

	return true;
}

bool CFrameResources::PrepareFrame(CRenderer* renderer)
{
	m_frameResIdx = (m_frameResIdx + 1) % gFrameResourceCount;
	if (m_fenceCount == 0)
		return true;
	ReturnIfFalse(renderer->WaitUntilGpuFinished(m_fenceCount));

	return true;
}

UINT64 CFrameResources::ForwardFrame()
{
	m_frameResIdx = (m_frameResIdx + 1) % gFrameResourceCount;
	return m_fenceCount;
}

ID3D12Resource* CFrameResources::GetResource(eBufferType bufferType)
{
	return GetUploadBuffer(bufferType)->Resource();
}

UINT CFrameResources::GetBufferSize(eBufferType bufferType)
{
	CUploadBuffer* buffer = GetUploadBuffer(bufferType);
	if (buffer == nullptr) return 0;

	return buffer->GetByteSize();
}

CUploadBuffer* CFrameResources::GetUploadBuffer(eBufferType bufferType)
{
	Resource* resource = m_resources[m_frameResIdx].get();
	if (resource == nullptr) return nullptr;
	
	switch (bufferType)
	{
	case eBufferType::PassCB:			return resource->passCB.get();
	case eBufferType::SsaoCB:			return resource->ssaoCB.get();
	case eBufferType::SkinnedCB:	return resource->skinnedCB.get();
	case eBufferType::Instance:		return resource->instanceBuffer.get();
	case eBufferType::Material:		return resource->materialBuffer.get();
	}

	return nullptr;
}