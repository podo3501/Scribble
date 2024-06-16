#pragma once

#include<wrl.h>

struct ID3D12Device;
struct ID3D12Resource;

class UploadBuffer
{
public:
    UploadBuffer(ID3D12Device* device, size_t typeSize, UINT elementCount, bool isConstantBuffer);

    UploadBuffer(const UploadBuffer& rhs) = delete;
    UploadBuffer& operator=(const UploadBuffer& rhs) = delete;

    ~UploadBuffer();
    ID3D12Resource* Resource()const;

    template<typename T>
    void CopyData(int elementIndex, const T& data)
    {
        memcpy(&mMappedData[elementIndex * mElementByteSize], &data, sizeof(T));
    }
    
private:
    Microsoft::WRL::ComPtr<ID3D12Resource> mUploadBuffer;
    BYTE* mMappedData = nullptr;

    UINT mElementByteSize = 0;
    bool mIsConstantBuffer = false;
};