#pragma once

#include <unordered_map>
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

class CModel
{
	using AllRenderItems = std::unordered_map<std::string, std::unique_ptr<RenderItem>>;
	using InstanceDataList = std::vector<std::shared_ptr<InstanceData>>;

public:
	CModel(IRenderer* m_iRenderer);
	~CModel();
	
	CModel() = delete;
	CModel(const CModel&) = delete;
	CModel& operator =(const CModel&) = delete;

	bool Initialize(const std::wstring& resPath);
	bool LoadMemory(AllRenderItems& allRenderItems);
	void UpdateRenderItems(CCamera* camera, const AllRenderItems& allRenderItems);
	void GetPassCB(PassConstants* outPc);
	void MakeMaterialBuffer();

private:
	void UpdateInstanceBuffer(const InstanceDataList& visibleInstance);

private:
	IRenderer* m_iRenderer{ nullptr };

	std::unique_ptr<CMaterial> m_material{ nullptr };
	std::unique_ptr<CMesh> m_mesh{ nullptr };
	std::unique_ptr<CSetupData> m_setupData{ nullptr };
};

