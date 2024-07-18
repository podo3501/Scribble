#include "./UploadBuffer.h"
#include "./d3dUtil.h"
#include <ranges>

CUploadBuffer::CUploadBuffer(size_t typeSize, UINT elementCount, bool isConstantBuffer) 
    : m_uploadBuffer{ nullptr }
    , m_isConstantBuffer(isConstantBuffer)
    , m_elementByteSize{ static_cast<UINT>(typeSize) }
    , m_typeSize{ static_cast<UINT>(typeSize) }
    , m_elementCount{ elementCount }
{
    // Constant buffer elements need to be multiples of 256 bytes.
    // This is because the hardware can only view constant data 
    // at m*256 byte offsets and of n*256 byte lengths. 
    // typedef struct D3D12_CONSTANT_BUFFER_VIEW_DESC {
    // UINT64 OffsetInBytes; // multiple of 256
    // UINT   SizeInBytes;   // multiple of 256
    // } D3D12_CONSTANT_BUFFER_VIEW_DESC;
    if (isConstantBuffer)
        m_elementByteSize = CoreUtil::CalcConstantBufferByteSize(static_cast<UINT>(typeSize));

    // We do not need to unmap until we are done with the resource.  However, we must not write to
    // the resource while it is in use by the GPU (so we must use synchronization techniques).
}

CUploadBuffer::~CUploadBuffer()
{
    if (m_uploadBuffer != nullptr)
        m_uploadBuffer->Unmap(0, nullptr);

    m_mappedData = nullptr;
}

bool CUploadBuffer::Initialize(ID3D12Device* device)
{
    CD3DX12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_elementByteSize * m_elementCount);
    ReturnIfFailed(device->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_uploadBuffer)));

    ReturnIfFailed(m_uploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedData)));

    return true;
}

ID3D12Resource* CUploadBuffer::Resource() const
{
    return m_uploadBuffer.Get();
}

void CUploadBuffer::CopyDataList(const void* data, size_t size)
{
    if (m_elementByteSize == m_typeSize)
        memcpy(&m_mappedData[0], data, size * m_elementByteSize);

    //|-------------|--------------|-------|        데이터 1, 데이터2, 남는 공간
    //|-----------------|------------------|        데이터 1(계산되어서 빈공간이 합쳐있는), 데이터2
    //Constant buffer와 같은 경우는 데이터가 밑에처럼 들어가야 하는데 
    // vector형식으로 넣으면 위에것 처럼 들어온다. 그래서 하나씩 자료를 돌면서 크기를 맞추어주면서 넣어준다.
    for (auto i : std::views::iota(0, static_cast<int>(size)))
    {
        unsigned char* curData = (unsigned char*)data;
        curData += m_typeSize * i;
        memcpy(&m_mappedData[m_elementByteSize * i], (void*)curData, m_typeSize);
    }
}