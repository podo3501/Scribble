#pragma once

#include<wrl.h>

struct ID3D12Device;
struct ID3D12Resource;

class CUploadBuffer
{
public:
    CUploadBuffer(ID3D12Device* device, size_t typeSize, UINT elementCount, bool isConstantBuffer);

    CUploadBuffer(const CUploadBuffer& rhs) = delete;
    CUploadBuffer& operator=(const CUploadBuffer& rhs) = delete;

    ~CUploadBuffer();
    ID3D12Resource* Resource()const;

    template<typename T>
    void CopyData(int elementIndex, const T& data)
    {
        memcpy(&mMappedData[elementIndex * mElementByteSize], &data, sizeof(T));
    }
    
private:
    Microsoft::WRL::ComPtr<ID3D12Resource> mUploadBuffer{ nullptr };
    BYTE* mMappedData{ nullptr };

    UINT mElementByteSize{ 0 };
    bool mIsConstantBuffer{ false };
};