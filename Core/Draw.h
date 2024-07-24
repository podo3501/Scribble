#pragma once
#include <d3d12.h>
#include <memory>
#include <map>

class CDirectx3D;
class CRenderer;
class CPipelineStateObjects;
class CFrameResources;
class CShadowMap;
class CSsaoMap;
class CDescriptorHeap;
struct ID3D12Device;
struct RenderItem;
struct ID3D12GraphicsCommandList;
enum class GraphicsPSO : int;
enum class eTextureType : int;
enum class RootSignature : int;

class CDraw
{
	using AllRenderItems = std::map<GraphicsPSO, std::unique_ptr<RenderItem>>;

public:
	CDraw();
	~CDraw();

	CDraw(const CDraw&) = delete;
	CDraw& operator=(const CDraw&) = delete;

	bool Initialize(CRenderer* renderer, CDescriptorHeap* descHeap, CPipelineStateObjects* pso);
	bool Excute(CFrameResources* frameRes, CSsaoMap* ssaoMap, AllRenderItems& renderItem);
	void OnResize(int width, int height);

private:
	void DrawSceneToShadowMap(CFrameResources* frameRes, AllRenderItems& renderItem);
	void DrawNormalsAndDepth(CFrameResources* frameRes, CSsaoMap* ssaoMap, AllRenderItems& renderItem);
	void DrawRenderItems(CFrameResources* frameRes, GraphicsPSO pso, RenderItem* renderItem);

	ID3D12RootSignature* GetRootSignature(RootSignature sigType);

private:
	CDirectx3D* m_directx3D;
	CRenderer* m_renderer;
	ID3D12Device* m_device;
	ID3D12GraphicsCommandList* m_cmdList;
	CDescriptorHeap* m_descHeap;
	CPipelineStateObjects* m_pso;
	std::unique_ptr<CShadowMap> m_shadowMap;

	D3D12_VIEWPORT m_screenViewport;
	D3D12_RECT m_scissorRect;
};
