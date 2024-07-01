#pragma once

#include <wrl.h>
#include <vector>

struct ID3D12Device;
struct ID3D12Resource;

class CUploadBuffer
{
public:
    CUploadBuffer(ID3D12Device* device, size_t typeSize, UINT elementCount, bool isConstantBuffer);
    ~CUploadBuffer();

    CUploadBuffer() = delete;
    CUploadBuffer(const CUploadBuffer& rhs) = delete;
    CUploadBuffer& operator=(const CUploadBuffer& rhs) = delete;

    ID3D12Resource* Resource()const;
    void CopyDataList(const void* data, size_t size);

    template<typename T>
    void CopyData(int elementIndex, const T& data)
    {
        memcpy(&m_mappedData[elementIndex * m_elementByteSize], &data, sizeof(T));
    }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadBuffer{ nullptr };
    BYTE* m_mappedData{ nullptr };

    UINT m_elementByteSize{ 0 };
    bool m_isConstantBuffer{ false };
};