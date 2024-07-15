#pragma once

const int gPassCBCount = 2;	//passCB, shadowPassCB
const int gInstanceBufferCount = 200;
const int gMaterialBufferCount = 100;

//셰이더의 내용물은 늘 자료가 있다고 가정한다.
//남아서 넘치는 건 상관없지만 셰이더 데이터에 빈공간이 있으면 안된다.
//텍스춰는 빈공간이 있어도 상관없다.
constexpr UINT CubeCount{ 1u };
constexpr UINT ShadowCount{ 1u };
constexpr UINT SsaoCount{ 1u };
constexpr UINT TextureCount{ 35u };
constexpr UINT TotalShaderResourceViewHeap = CubeCount + ShadowCount + SsaoCount + TextureCount;

constexpr UINT SwapChainBufferCount{ 2u };
constexpr UINT SsaoScreenNormalMap{ 1u };
constexpr UINT SsaoAmbientMap{ 2u };
constexpr UINT TotalRenderTargetViewHeap = SwapChainBufferCount + SsaoScreenNormalMap + SsaoAmbientMap;

constexpr UINT DsvCommon{ 0u };
constexpr UINT DsvShadowMap{ 1u };

constexpr UINT DsvCommonCount{ 1u };
constexpr UINT DsvShadowMapCount{ 1u };
constexpr UINT TotalDepthStencilView = DsvCommonCount + DsvShadowMapCount;

enum class RtvOffset : int
{
	SwapChain0,
	SwapChain1,
	NormalMap,
	AmbientMap0,
	AmbientMap1,
};