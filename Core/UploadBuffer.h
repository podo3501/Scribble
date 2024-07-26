#pragma once

class CUploadBuffer
{
public:
    CUploadBuffer(size_t typeSize, UINT elementCount, bool isConstantBuffer);
    ~CUploadBuffer();

    CUploadBuffer() = delete;
    CUploadBuffer(const CUploadBuffer& rhs) = delete;
    CUploadBuffer& operator=(const CUploadBuffer& rhs) = delete;

    bool Initialize(ID3D12Device* device);
    ID3D12Resource* Resource()const;
    void CopyDataList(const void* data, size_t size);
    inline UINT GetByteSize() { return m_elementByteSize; }; 

    template<typename T>
    void CopyData(int elementIndex, const T& data)
    {
        memcpy(&m_mappedData[elementIndex * m_elementByteSize], &data, sizeof(T));
    }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadBuffer;
    BYTE* m_mappedData{ nullptr };

    bool m_isConstantBuffer{ false };
    UINT m_elementByteSize{ 0u };
    UINT m_typeSize{ 0u };
    UINT m_elementCount{ 0u };
};