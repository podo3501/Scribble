#pragma once
#include <array>
#include <wrl.h>

enum class eTextureType : int
{
	None = 0,
	ShadowMap,
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
	ShadowMap,
	SsaoMap,
	SsaoBlur,
};
