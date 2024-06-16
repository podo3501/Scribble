#include "FrameResource.h"
#include <d3d12.h>
#include "../Core/d3dUtil.h"

FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT maxInstanceCount, UINT materialCount)
{
    ThrowIfFailed(device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(CmdListAlloc.GetAddressOf())));

    PassCB = std::make_unique<UploadBuffer>(device, sizeof(PassConstants), passCount, true);
    MaterialBuffer = std::make_unique<UploadBuffer>(device, sizeof(MaterialData), materialCount, false);
    InstanceBuffer = std::make_unique<UploadBuffer>(device, sizeof(InstanceData), maxInstanceCount, false);
}

FrameResource::~FrameResource()
{

}