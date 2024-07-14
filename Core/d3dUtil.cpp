#include "./d3dUtil.h"
#include <comdef.h>
#include <fstream>
#include <stack>
#include "../Core/DirectXTK12Inc/WICTextureLoader.h"

using Microsoft::WRL::ComPtr;

bool CoreUtil::IsKeyDown(int vkeyCode)
{
    return (GetAsyncKeyState(vkeyCode) & 0x8000) != 0;
}

ComPtr<ID3DBlob> CoreUtil::LoadBinary(const std::wstring& filename)
{
    std::ifstream fin(filename, std::ios::binary);

    fin.seekg(0, std::ios_base::end);
    std::ifstream::pos_type size = (int)fin.tellg();
    fin.seekg(0, std::ios_base::beg);

    ComPtr<ID3DBlob> blob{ nullptr };
    HRESULT hResult = D3DCreateBlob(static_cast<SIZE_T>(size), blob.GetAddressOf());
    if (FAILED(hResult)) return nullptr;

    fin.read((char*)blob->GetBufferPointer(), size);
    fin.close();

    return blob;
}

bool CoreUtil::CreateDefaultBuffer(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* cmdList,
    const void* initData,
    UINT64 byteSize,
    Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer,
    Microsoft::WRL::ComPtr<ID3D12Resource>* outDefaultBuffer)
{
    ComPtr<ID3D12Resource> defaultBuffer{ nullptr };

    // 실제 기본 버퍼 자원을 생성한다.
    // 정적 기하구조를 그릴 때에는 최적의 성능을 위해 기본힙(D3D12_HEAP_TYPE_DEFAULT)에 넣는다.
    ReturnIfFailed(device->CreateCommittedResource(
        &RvToLv(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
        D3D12_HEAP_FLAG_NONE,
        &RvToLv(CD3DX12_RESOURCE_DESC(CD3DX12_RESOURCE_DESC::Buffer(byteSize))),
		D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(defaultBuffer.GetAddressOf())));

    // cpu메모리(ram)의 자료를 기본 버퍼(vram)에 복사하려면
    // 임시 업로드 힙을 만들어야 한다. 기본버퍼는 cpu에서 접근이 가능하지 않기 때문이다. 
    ReturnIfFailed(device->CreateCommittedResource(
        &RvToLv(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)),
		D3D12_HEAP_FLAG_NONE,
        &RvToLv(CD3DX12_RESOURCE_DESC(CD3DX12_RESOURCE_DESC::Buffer(byteSize))),
		D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(uploadBuffer.GetAddressOf())));


    // 기본버퍼에 복사할 자료이다.
    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = initData;
    subResourceData.RowPitch = static_cast<LONG_PTR>(byteSize);
    subResourceData.SlicePitch = subResourceData.RowPitch;

    //기본 버퍼 자원으로의 자료 복사를 '스케쥴'한다(지금 실행되는 것이 아님)
    //개략적으로 말하자면, subResourceData를 임시 업로드 힙에 복사하고, 
    //ID3D12CommandList::CopySubresourceRegion을 이용해서 임시 업로드 힙의 자료를 defaultBuffer에 복사한다.
    CD3DX12_RESOURCE_BARRIER barrierDest(
        CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
            D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	cmdList->ResourceBarrier(1, &barrierDest);

    UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);

    CD3DX12_RESOURCE_BARRIER barrierRead(
        CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
	cmdList->ResourceBarrier(1, &barrierRead);

    // Note: uploadBuffer has to be kept alive after the above function calls because
    // the command list has not been executed yet that performs the actual copy.
    // The caller can Release the uploadBuffer after it knows the copy has been executed.

    (*outDefaultBuffer) = defaultBuffer;

    return true;
}

std::wstring GetWstring(LPCSTR pFileName)
{
    std::string str = std::string(pFileName);
    return std::wstring().assign(str.begin(), str.end());
}

std::wstring SplitPath(const std::wstring& fullFilename)
{
    std::wstring fullname{ fullFilename };
    auto find = fullname.rfind(L"/");
    return fullname.substr(0, find + 1);
}

class IncludeProcessor : public ID3DInclude
{
public:
    IncludeProcessor(const std::wstring path)
    {
        m_stackPath.push(path);
    }

    ~IncludeProcessor()
    {
        m_stackPath.pop();
        assert(m_stackPath.empty());
    }

private:
    std::stack<std::wstring> m_stackPath{};

public:
    HRESULT STDMETHODCALLTYPE Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) noexcept override
    {
        std::wstring path = m_stackPath.top();
        std::wstring fullFilename = path + GetWstring(pFileName);
        std::wstring parentPath = SplitPath(fullFilename);
        m_stackPath.push(parentPath);

        std::ifstream ifs(fullFilename);
        if (ifs.fail()) return E_FAIL;

        std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        ifs.close();

        UINT numBytes = static_cast<UINT>(str.size());

        char* pData;
        pData = new char[numBytes + 1]; // (char*)malloc( numBytes+1 ); // Add 1 for the null terminator that is appended by strncpy_s.
        strncpy_s(pData, numBytes + 1, str.data(), str.size());

        *ppData = (LPCVOID)pData;
        *pBytes = numBytes;

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Close(LPCVOID pData) noexcept override
    {
        delete[](char*)pData;
        m_stackPath.pop();
        return S_OK;
    }
};

bool CoreUtil::CompileShader(
	const std::wstring& filename,
	const D3D_SHADER_MACRO* defines,
	const std::string& entrypoint,
	const std::string& target,
    ComPtr<ID3DBlob>* outBlob)
{
	UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = S_OK;
    
    IncludeProcessor includeProcessor(SplitPath(filename));
	ComPtr<ID3DBlob> byteCode{ nullptr };
    ComPtr<ID3DBlob> errors{ nullptr };
	hr = D3DCompileFromFile(filename.c_str(), defines, &includeProcessor,
		entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

	if(errors != nullptr)
		OutputDebugStringA((char*)errors->GetBufferPointer());

	ReturnIfFailed(hr);
    (*outBlob) = byteCode;

    return true;
}

HRESULT CoreUtil::LoadTextureFromFile(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* cmdList,
    std::wstring&& filename,
    Microsoft::WRL::ComPtr<ID3D12Resource>& texture,
    Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeaps)
{
    std::unique_ptr<uint8_t[]> decodedData;
    D3D12_SUBRESOURCE_DATA subresource;
    HRESULT hr = DirectX::LoadWICTextureFromFile(device, filename.c_str(), texture.ReleaseAndGetAddressOf(),
        decodedData, subresource);
    if (FAILED(hr))
    {
        return hr;
    }

    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, 1);

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);

    auto desc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

    // Create the GPU upload buffer.
    hr = device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(uploadHeaps.GetAddressOf()));
    if (FAILED(hr))
    {
        return hr;
    }

    UpdateSubresources(cmdList, texture.Get(), uploadHeaps.Get(),
        0, 0, 1, &subresource);

    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    cmdList->ResourceBarrier(1, &barrier);  

    return S_OK;
}

D3D12_STATIC_SAMPLER_DESC ShadowSampler(UINT registerIdx)
{
    return CD3DX12_STATIC_SAMPLER_DESC(
        registerIdx, // shaderRegister
        D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
        0.0f,                               // mipLODBias
        16,                                 // maxAnisotropy
        D3D12_COMPARISON_FUNC_LESS_EQUAL,
        D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);
}

std::vector<D3D12_STATIC_SAMPLER_DESC> CoreUtil::GetStaticSamplers()
{
    std::vector<D3D12_STATIC_SAMPLER_DESC> samplers;

    UINT idx{ 0 };
    auto MakeSampler = [&samplers, &idx](D3D12_FILTER filter,
        D3D12_TEXTURE_ADDRESS_MODE addressUVW, FLOAT mipLODBias = 0.0f, UINT maxAnisotropy = 16)
        {
            samplers.emplace_back(CD3DX12_STATIC_SAMPLER_DESC
                {
                    idx++, filter,
                    addressUVW, addressUVW, addressUVW,
                    mipLODBias,
                    maxAnisotropy
                });
        };

    MakeSampler(D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
    MakeSampler(D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
    MakeSampler(D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
    MakeSampler(D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
    MakeSampler(D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0.0f, 8);
    MakeSampler(D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0.0f, 8);
    samplers.emplace_back(ShadowSampler(idx++));

    return samplers;
}

std::vector<D3D12_STATIC_SAMPLER_DESC> CoreUtil::GetSsaoSamplers()
{
    const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
        0, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
        1, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC depthMapSam(
        2, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
        0.0f,
        0,
        D3D12_COMPARISON_FUNC_LESS_EQUAL,
        D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE);

    const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
        3, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

    std::vector<D3D12_STATIC_SAMPLER_DESC> samplers 
    {
        pointClamp, linearClamp, depthMapSam, linearWrap  
    };

    return samplers;
}



