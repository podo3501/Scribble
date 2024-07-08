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
	Count,
};

enum class GraphicsPSO : int
{
	Sky,
	Opaque,
	Count
};

static constexpr std::array<GraphicsPSO, static_cast<size_t>(GraphicsPSO::Count)> GraphicsPSO_ALL
{
	GraphicsPSO::Sky,
	GraphicsPSO::Opaque,
};
