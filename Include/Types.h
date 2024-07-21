#pragma once
#include <array>
#include <wrl.h>

//다른 rootSignature에 있어도 srv는 하나의 배열을 사용하기 때문에 인덱스가 중복되어서는 안된다.
//rootSignature에 같은 srv를 쓰는 데이터가 register가 달라도 이 인덱스는 하나의 배열에서의 인덱스 이기
//때문에 같을수가 없다.
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
