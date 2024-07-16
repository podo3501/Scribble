#pragma once

const int gPassCBCount = 2;	//passCB, shadowPassCB
const int gInstanceBufferCount = 200;
const int gMaterialBufferCount = 100;

//���̴��� ���빰�� �� �ڷᰡ �ִٰ� �����Ѵ�.
//���Ƽ� ��ġ�� �� ��������� ���̴� �����Ϳ� ������� ������ �ȵȴ�.
//�ؽ���� ������� �־ �������.
constexpr UINT CubeCount{ 1u };
constexpr UINT ShadowCount{ 1u };
constexpr UINT SsaoAmbientMap0Count{ 1u };
constexpr UINT SsaoAmbientMap1Count{ 1u };
constexpr UINT SsaoNormalMapCount{ 1u };
constexpr UINT SsaoDepthMapCount{ 1u };
constexpr UINT SsaoRandomVectorCount{ 1u };
constexpr UINT TextureCount{ 35u };
constexpr UINT TotalSsaoCount = SsaoAmbientMap0Count + SsaoAmbientMap1Count + SsaoNormalMapCount + SsaoDepthMapCount + SsaoRandomVectorCount;
constexpr UINT TotalShaderResourceViewHeap = CubeCount + ShadowCount + TotalSsaoCount + TextureCount;

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