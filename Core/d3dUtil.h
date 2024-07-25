//***************************************************************************************
// d3dUtil.h by Frank Luna (C) 2015 All Rights Reserved.
//
// General helper code.
//***************************************************************************************

#pragma once

#include <windows.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <string>
#include <memory>
#include <algorithm>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cassert>
#include "./d3dx12.h"
#include "./DDSTextureLoader.h"
//#include "./MathHelper.h"

extern const int gNumFrameResources;

inline void d3dSetDebugName(IDXGIObject* obj, const char* name)
{
    if(obj)
    {
        obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
    }
}
inline void d3dSetDebugName(ID3D12Device* obj, const char* name)
{
    if(obj)
    {
        obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
    }
}
inline void d3dSetDebugName(ID3D12DeviceChild* obj, const char* name)
{
    if(obj)
    {
        obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
    }
}

inline std::wstring AnsiToWString(const std::string& str)
{
    WCHAR buffer[512];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
    return std::wstring(buffer);
}

/*
#if defined(_DEBUG)
    #ifndef Assert
    #define Assert(x, description)                                  \
    {                                                               \
        static bool ignoreAssert = false;                           \
        if(!ignoreAssert && !(x))                                   \
        {                                                           \
            Debug::AssertResult result = Debug::ShowAssertDialog(   \
            (L#x), description, AnsiToWString(__FILE__), __LINE__); \
        if(result == Debug::AssertIgnore)                           \
        {                                                           \
            ignoreAssert = true;                                    \
        }                                                           \
                    else if(result == Debug::AssertBreak)           \
        {                                                           \
            __debugbreak();                                         \
        }                                                           \
        }                                                           \
    }
    #endif
#else
    #ifndef Assert
    #define Assert(x, description) 
    #endif
#endif 		
    */

class CoreUtil
{
public:
    static void LogAdapters(IDXGIFactory4* dxgiFactory, DXGI_FORMAT backBufferFormat);

    static bool IsKeyDown(int vkeyCode);
    static std::string ToString(HRESULT hr);
    static UINT CalcConstantBufferByteSize(UINT byteSize)
    {
        // Constant buffers must be a multiple of the minimum hardware
        // allocation size (usually 256 bytes).  So round up to nearest
        // multiple of 256.  We do this by adding 255 and then masking off
        // the lower 2 bytes which store all bits < 256.
        // Example: Suppose byteSize = 300.
        // (300 + 255) & ~255
        // 555 & ~255
        // 0x022B & ~0x00ff
        // 0x022B & 0xff00
        // 0x0200
        // 512
        return (byteSize + 255) & ~255;
    }

    static Microsoft::WRL::ComPtr<ID3DBlob> LoadBinary(const std::wstring& filename);

    static bool CreateDefaultBuffer(
        ID3D12Device* device,
        ID3D12GraphicsCommandList* cmdList,
        const void* initData,
        UINT64 byteSize,
        Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer,
        Microsoft::WRL::ComPtr<ID3D12Resource>* outDefaultBuffer);

	static bool CompileShader(
		const std::wstring& filename,
		const D3D_SHADER_MACRO* defines,
		const std::string& entrypoint,
		const std::string& target,
        Microsoft::WRL::ComPtr<ID3DBlob>* outBlob);

    static HRESULT LoadTextureFromFile(
        ID3D12Device* device,
        ID3D12GraphicsCommandList* cmdList,
        std::wstring&& filename,
        Microsoft::WRL::ComPtr<ID3D12Resource>& texture,
        Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeaps);

    static std::vector<D3D12_STATIC_SAMPLER_DESC> GetStaticSamplers();
    static std::vector<D3D12_STATIC_SAMPLER_DESC> GetSsaoSamplers();
};

struct MaterialConstants
{
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = 0.25f;

	// Used in texture mapping.
    DirectX::XMFLOAT4X4 MatTransform = {};
};

template <typename E>
constexpr auto
EtoV(E enumerator) noexcept
{
    return static_cast<std::underlying_type_t<E>>(enumerator);
}

template <class _Tp>
_Tp& RvToLv(_Tp&& __value)
{
    return __value;
}

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw CoreException(hr__, L#x, wfn, __LINE__); } \
}
#endif

#ifndef ReturnIfFailed
#define ReturnIfFailed(x)                                              \
{                                                                     \
    if(FAILED(x))                                            \
        return false;                                                \
}
#endif

#ifndef ReturnIfFalse
#define ReturnIfFalse(x)                       \
{                                                               \
    if(!x)                                                      \
    {                                               \
        assert(false);                          \
        return false;                          \
    }                                                       \
}
#endif

#ifndef ReleaseCom
#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }
#endif