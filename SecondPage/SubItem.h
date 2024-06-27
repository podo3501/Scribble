#pragma once

#include <wrl.h>
#include <DirectXCollision.h>

struct SubItem
{
	UINT indexCount{ 0u };
	UINT startIndexLocation{ 0u };
	INT baseVertexLocation{ 0 };

	DirectX::BoundingBox boundingBox{};
	DirectX::BoundingSphere boundingSphere{};
};