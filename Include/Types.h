#pragma once
#include <array>
#include <wrl.h>

//�ٸ� rootSignature�� �־ srv�� �ϳ��� �迭�� ����ϱ� ������ �ε����� �ߺ��Ǿ�� �ȵȴ�.
//rootSignature�� ���� srv�� ���� �����Ͱ� register�� �޶� �� �ε����� �ϳ��� �迭������ �ε��� �̱�
//������ �������� ����.
enum class eTextureType : int
{
	ShadowMap,
	SsaoAmbientMap0,
	SsaoAmbientMap1,
	SsaoNormalMap,
	SsaoDepthMap,
	SsaoRandomVectorMap,
	TextureCube,
	Texture2D,
};

enum class ShaderType : int
{
	VS,
	PS,
};

enum class GraphicsPSO : int
{
	Sky,
	Opaque,
	NormalOpaque,
	SkinnedOpaque,
	SkinnedShadowOpaque,
	SkinnedDrawNormals,
	ShadowMap,
	SsaoDrawNormals,
	SsaoMap,
	SsaoBlur,
	Debug,
};
