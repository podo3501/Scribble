#include "./UploadBuffer.h"
#include "./d3dUtil.h"

CUploadBuffer::CUploadBuffer(ID3D12Device* device, size_t typeSize, UINT elementCount, bool isConstantBuffer) :
    m_isConstantBuffer(isConstantBuffer)
{
    m_elementByteSize = static_cast<UINT>(typeSize);

    // Constant buffer elements need to be multiples of 256 bytes.
    // This is because the hardware can only view constant data 
    // at m*256 byte offsets and of n*256 byte lengths. 
    // typedef struct D3D12_CONSTANT_BUFFER_VIEW_DESC {
    // UINT64 OffsetInBytes; // multiple of 256
    // UINT   SizeInBytes;   // multiple of 256
    // } D3D12_CONSTANT_BUFFER_VIEW_DESC;
    if (isConstantBuffer)
        m_elementByteSize = CoreUtil::CalcConstantBufferByteSize(static_cast<UINT>(typeSize));

    CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_elementByteSize * elementCount);
    ThrowIfFailed(device->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_uploadBuffer)));

    ThrowIfFailed(m_uploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedData)));

    // We do not need to unmap until we are done with the resource.  However, we must not write to
    // the resource while it is in use by the GPU (so we must use synchronization techniques).
}

CUploadBuffer::~CUploadBuffer()
{
    if (m_uploadBuffer != nullptr)
        m_uploadBuffer->Unmap(0, nullptr);

    m_mappedData = nullptr;
}

ID3D12Resource* CUploadBuffer::Resource() const
{
    return m_uploadBuffer.Get();
}

void CUploadBuffer::CopyDataList(const void* data, size_t size)
{
    memcpy(&m_mappedData[0], data, size * m_elementByteSize);
}