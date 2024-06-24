#pragma once
#include <array>

const int gFrameResourceCount = 3;

enum class GraphicsPSO : int
{
	Opaque,
	Count
};

static constexpr std::array<GraphicsPSO, static_cast<size_t>(GraphicsPSO::Count)> GraphicsPSO_ALL
{
	GraphicsPSO::Opaque,
};