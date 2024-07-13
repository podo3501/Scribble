#pragma once

const int gPassCBCount = 2;	//passCB, shadowPassCB
const int gInstanceBufferCount = 200;
const int gMaterialBufferCount = 100;

//셰이더의 내용물은 늘 자료가 있다고 가정한다.
//남아서 넘치는 건 상관없지만 셰이더 데이터에 빈공간이 있으면 안된다.
//텍스춰는 빈공간이 있어도 상관없다.
constexpr UINT CubeCount{ 1u };
constexpr UINT ShadowCount{ 1u };
constexpr UINT TextureCount{ 35u };
constexpr UINT TotalHeapCount = CubeCount + ShadowCount + TextureCount;

//root signature, shader와 자료구조가 일치해야한다. 
constexpr UINT SrvShadowMapStartOffset{ 0u };
constexpr UINT SrvTextureCubeStartOffset{ 1u };
constexpr UINT SrvTexture2DStartOffset{ 2u };

constexpr UINT DsvCommon{ 0u };
constexpr UINT DsvShadowMap{ 1u };