#pragma once

#include <DirectXMath.h>
#include <wrl.h>

struct Light
{
    DirectX::XMFLOAT3 strength = { 0.5f, 0.5f, 0.5f };
    float falloffStart = 1.0f;                          // point/spot light only
    DirectX::XMFLOAT3 direction = { 0.0f, -1.0f, 0.0f };// directional/spot light only
    float falloffEnd = 10.0f;                           // point/spot light only
    DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };  // point/spot light only
    float spotPower = 64.0f;                            // spot light only

    static constexpr int MaxLights = 16;
};

struct PassConstants
{
    DirectX::XMFLOAT4X4 view{};
    DirectX::XMFLOAT4X4 invView{};
    DirectX::XMFLOAT4X4 proj{};
    DirectX::XMFLOAT4X4 invProj{};
    DirectX::XMFLOAT4X4 viewProj{};
    DirectX::XMFLOAT4X4 invViewProj{};
    DirectX::XMFLOAT3 eyePosW = { 0.0f, 0.0f, 0.0f };
    float cbPerObjectPad1 = 0.0f;
    DirectX::XMFLOAT2 renderTargetSize = { 0.0f, 0.0f };
    DirectX::XMFLOAT2 invRenderTargetSize = { 0.0f, 0.0f };
    float nearZ = 0.0f;
    float farZ = 0.0f;
    float totalTime = 0.0f;
    float deltaTime = 0.0f;

    DirectX::XMFLOAT4 ambientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

    Light lights[Light::MaxLights]{};
};

struct InstanceBuffer
{
    DirectX::XMFLOAT4X4 world{};
    DirectX::XMFLOAT4X4 texTransform{};
    UINT     materialIndex = 0;
    UINT     objPad0 = 0;
    UINT     objPad1 = 0;
    UINT     objPad2 = 0;
};

struct MaterialBuffer
{
    DirectX::XMFLOAT4 diffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT3 fresnelR0 = { 0.01f, 0.01f, 0.01f };
    float roughness = 64.0f;

    DirectX::XMFLOAT4X4 matTransform{};

    UINT diffuseMapIndex = 0;
    UINT materialPad0;
    UINT materialPad1;
    UINT materialPad2;
};

//geometry ���� struct
struct Vertex
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 texC;
};
