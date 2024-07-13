#pragma once

const int gPassCBCount = 2;	//passCB, shadowPassCB
const int gInstanceBufferCount = 200;
const int gMaterialBufferCount = 100;

//���̴��� ���빰�� �� �ڷᰡ �ִٰ� �����Ѵ�.
//���Ƽ� ��ġ�� �� ��������� ���̴� �����Ϳ� ������� ������ �ȵȴ�.
//�ؽ���� ������� �־ �������.
constexpr UINT CubeCount{ 1u };
constexpr UINT ShadowCount{ 1u };
constexpr UINT TextureCount{ 35u };
constexpr UINT TotalHeapCount = CubeCount + ShadowCount + TextureCount;

//root signature, shader�� �ڷᱸ���� ��ġ�ؾ��Ѵ�. 
constexpr UINT SrvShadowMapStartOffset{ 0u };
constexpr UINT SrvTextureCubeStartOffset{ 1u };
constexpr UINT SrvTexture2DStartOffset{ 2u };

constexpr UINT DsvCommon{ 0u };
constexpr UINT DsvShadowMap{ 1u };