#pragma once

#include <DirectXMath.h>
#include <wrl.h>

struct Light
{
    DirectX::XMFLOAT3 Strength = { 0.5f, 0.5f, 0.5f };
    float FalloffStart = 1.0f;                          // point/spot light only
    DirectX::XMFLOAT3 Direction = { 0.0f, -1.0f, 0.0f };// directional/spot light only
    float FalloffEnd = 10.0f;                           // point/spot light only
    DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };  // point/spot light only
    float SpotPower = 64.0f;                            // spot light only

    static constexpr int MaxLights = 16;
};

struct PassConstants
{
    DirectX::XMFLOAT4X4 View{};
    DirectX::XMFLOAT4X4 InvView{};
    DirectX::XMFLOAT4X4 Proj{};
    DirectX::XMFLOAT4X4 InvProj{};
    DirectX::XMFLOAT4X4 ViewProj{};
    DirectX::XMFLOAT4X4 InvViewProj{};
    DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
    float cbPerObjectPad1 = 0.0f;
    DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
    DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
    float NearZ = 0.0f;
    float FarZ = 0.0f;
    float TotalTime = 0.0f;
    float DeltaTime = 0.0f;

    DirectX::XMFLOAT4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

    Light Lights[Light::MaxLights]{};
};

struct InstanceBuffer
{
    DirectX::XMFLOAT4X4 World{};
    DirectX::XMFLOAT4X4 TexTransform{};
    UINT     MaterialIndex = 0;
    UINT     ObjPad0 = 0;
    UINT     ObjPad1 = 0;
    UINT     ObjPad2 = 0;
};

struct MaterialBuffer
{
    DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
    float Roughness = 64.0f;

    DirectX::XMFLOAT4X4 MatTransform{};

    UINT DiffuseMapIndex = 0;
    UINT MaterialPad0;
    UINT MaterialPad1;
    UINT MaterialPad2;
};

//geometry ฐüทร struct
struct Vertex
{
    DirectX::XMFLOAT3 Pos;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT2 TexC;
};
