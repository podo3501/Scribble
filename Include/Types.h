#pragma once
#include <array>

enum class eTextureType : int
{
	None = 0,
	Cube,
	Common,
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
};
