#pragma once
#include <vector>
#include <memory>
#include <DirectXCollision.h>

class CMaterial;
class CGameTimer;
class CCamera;
class UploadBuffer;
struct FrameResource;
struct MeshGeometry;
struct RenderItem;

class CModel
{
public:
	CModel();

	CModel(const CModel&) = delete;
	CModel& operator=(const CModel&) = delete;

	bool Read(MeshGeometry* meshGeo);
	void BuildRenderItems(MeshGeometry* pGeo, CMaterial* pMaterial, std::vector<std::unique_ptr<RenderItem>>& renderItems);

	void Update(const CGameTimer* gt,
		const CCamera* camera,
		UploadBuffer* instanceBuffer,
		DirectX::BoundingFrustum& camFrustum,
		bool frustumCullingEnabled,
		std::vector<std::unique_ptr<RenderItem>>& renderItems);

private:
};
