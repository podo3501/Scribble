#pragma once
#include <vector>
#include <memory>

class CMaterial;
struct MeshGeometry;
struct RenderItem;

class CModel
{
public:
	CModel();
	bool Read(MeshGeometry* meshGeo);
	void BuildRenderItems(MeshGeometry* pGeo, CMaterial* pMaterial, std::vector<std::unique_ptr<RenderItem>>& renderItems);

	CModel(const CModel&) = delete;
	CModel& operator=(const CModel&) = delete;

private:
};
