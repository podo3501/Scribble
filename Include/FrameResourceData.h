#pragma once

#include <DirectXMath.h>
#include <wrl.h>
#include <vector>

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

enum class eBufferType : int
{
    NoType = 0,
    PassCB,
    Instance,
    Material,
    SsaoCB,
};

struct PassConstants
{
    DirectX::XMFLOAT4X4 view{};
    DirectX::XMFLOAT4X4 invView{};
    DirectX::XMFLOAT4X4 proj{};
    DirectX::XMFLOAT4X4 invProj{};
    DirectX::XMFLOAT4X4 viewProj{};
    DirectX::XMFLOAT4X4 invViewProj{};
    DirectX::XMFLOAT4X4 viewProjTex{};
    DirectX::XMFLOAT4X4 shadowTransform{};
    DirectX::XMFLOAT3 eyePosW = { 0.0f, 0.0f, 0.0f };
    float cbPerObjectPad1{ 0.0f };
    DirectX::XMFLOAT2 renderTargetSize = { 0.0f, 0.0f };
    DirectX::XMFLOAT2 invRenderTargetSize = { 0.0f, 0.0f };
    float nearZ{ 0.0f };
    float farZ{ 0.0f };
    float totalTime{ 0.0f };
    float deltaTime{ 0.0f };

    DirectX::XMFLOAT4 ambientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

    Light lights[Light::MaxLights]{};
};

struct SsaoConstants
{
    DirectX::XMFLOAT4X4 proj{};
    DirectX::XMFLOAT4X4 invProj{};
    DirectX::XMFLOAT4X4 projTex{};
    DirectX::XMFLOAT4 offsetVectors[14]{};

    DirectX::XMFLOAT4 blurWeights[3]{};

    DirectX::XMFLOAT2 invRenderTargetSize = { 0.0f, 0.0f };

    float occlusionRadius{ 0.5f };
    float occlusionFadeStart{ 0.2f };
    float occlusionFadeEnd{ 2.0f };
    float surfaceEpsilon{ 0.05f };
};

struct InstanceBuffer
{
    DirectX::XMFLOAT4X4 world{};
    DirectX::XMFLOAT4X4 texTransform{};
    UINT     materialIndex{ 0u };
    UINT     objPad0{ 0u };
    UINT     objPad1{ 0u };
    UINT     objPad2{ 0u };
};

struct MaterialBuffer
{
    DirectX::XMFLOAT4 diffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT3 fresnelR0 = { 0.01f, 0.01f, 0.01f };
    float roughness = 0.5f;

    DirectX::XMFLOAT4X4 matTransform{};

    UINT diffuseMapIndex{ 0u };
    UINT NormalMapIndex{ 0u };
    UINT materialPad1{ 0u };
    UINT materialPad2{ 0u };
};

//geometry ฐüทร struct
struct Vertex
{
    Vertex() = default;
    Vertex(
        const DirectX::XMFLOAT3& p,
        const DirectX::XMFLOAT3& n,
        const DirectX::XMFLOAT2& u,
        const DirectX::XMFLOAT4& t)
        : pos(p), normal(n), texC(u) ,tangentU(t) {}

    DirectX::XMFLOAT3 pos{};
    DirectX::XMFLOAT3 normal{};
    DirectX::XMFLOAT2 texC{};
    DirectX::XMFLOAT4 tangentU{};
};

using Vertices = std::vector<Vertex>;
using Indices = std::vector<std::int32_t>;

struct SkinnedVertex
{
    DirectX::XMFLOAT3 Pos;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT2 TexC;
    DirectX::XMFLOAT3 TangentU;
    DirectX::XMFLOAT3 BoneWeights;
    BYTE BoneIndices[4];
};

using SkinnedVertices = std::vector<SkinnedVertex>;
