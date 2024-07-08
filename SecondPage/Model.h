#pragma once

#include <map>
#include <functional>
#include <vector>
#include <string>
#include <memory>
#include <Windows.h>

interface IRenderer;
class CMaterial;
class CMesh;
class CSetupData;
class CCamera;
struct RenderItem;
struct InstanceData;
struct PassConstants;
enum class GraphicsPSO : int;

class CModel
{
	using AllRenderItems = std::map<GraphicsPSO, std::unique_ptr<RenderItem>>;
	using InstanceDataList = std::vector<std::shared_ptr<InstanceData>>;

public:
	CModel();
	~CModel();
	
	CModel(const CModel&) = delete;
	CModel& operator =(const CModel&) = delete;

	bool Initialize(const std::wstring& resPath, std::function<bool(CSetupData*, CMaterial*)> data);
	bool LoadMemory(IRenderer* renderer, AllRenderItems& allRenderItems);
	void Update(IRenderer* renderer, CCamera* camera, AllRenderItems& allRenderItems);

private:
	void UpdateRenderItems(IRenderer* renderer, CCamera* camera, AllRenderItems& allRenderItems);
	void UpdateInstanceBuffer(IRenderer* renderer, const InstanceDataList& visibleInstance);

private:
	std::unique_ptr<CMaterial> m_material;
	std::unique_ptr<CMesh> m_mesh;
	std::unique_ptr<CSetupData> m_setupData;
};

