#pragma once

class CDirectx3D;
class CRootSignature;
class CPipelineStateObjects;
class CFrameResources;
class CShadowMap;
class CSsaoMap;
class CDescriptorHeap;
struct RenderItem;
enum class GraphicsPSO : int;

class CDraw
{
	using AllRenderItems = std::map<GraphicsPSO, std::unique_ptr<RenderItem>>;

public:
	CDraw(CDirectx3D* directx3D);
	~CDraw();

	CDraw() = delete;
	CDraw(const CDraw&) = delete;
	CDraw& operator=(const CDraw&) = delete;

	bool Initialize(CDescriptorHeap* descHeap, CPipelineStateObjects* pso);
	bool Excute(CRootSignature* rootSignature, CFrameResources* frameRes, CSsaoMap* ssaoMap, AllRenderItems& renderItem);
	void OnResize(int width, int height);

private:
	void DrawSceneToShadowMap(CFrameResources* frameRes, AllRenderItems& renderItem);
	void DrawNormalsAndDepth(CFrameResources* frameRes, CSsaoMap* ssaoMap, AllRenderItems& renderItem);
	void DrawRenderItems(CFrameResources* frameRes, GraphicsPSO pso, RenderItem* renderItem);

private:
	CDirectx3D* m_directx3D;
	ID3D12Device* m_device;
	ID3D12GraphicsCommandList* m_cmdList;
	CDescriptorHeap* m_descHeap;
	CPipelineStateObjects* m_pso;
	std::unique_ptr<CShadowMap> m_shadowMap;

	D3D12_VIEWPORT m_screenViewport;
	D3D12_RECT m_scissorRect;
};
