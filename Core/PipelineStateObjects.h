#pragma once

#include <map>
#include <vector>
#include <wrl.h>

class CShader;
class CRenderer;
struct ID3D12Device;
struct D3D12_GRAPHICS_PIPELINE_STATE_DESC;
struct ID3D12PipelineState;
enum class GraphicsPSO : int;

class CPipelineStateObjects
{
public:
	CPipelineStateObjects(CRenderer* renderer);
	~CPipelineStateObjects();

	CPipelineStateObjects() = delete;
	CPipelineStateObjects(const CPipelineStateObjects&) = delete;
	CPipelineStateObjects& operator=(const CPipelineStateObjects&) = delete;

	bool Build(CShader* shader);

	ID3D12PipelineState* GetPso(GraphicsPSO type) noexcept;

private:
	void MakePSOPipelineState(GraphicsPSO psoType, D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc) noexcept;
	bool CreatePipelineState(CShader* shader, GraphicsPSO psoType);

	void MakeBasicDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc) noexcept;
	void MakeSkyDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc) noexcept;
	void MakeOpaqueDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc) noexcept;
	void MakeNormalOpaqueDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc) noexcept;
	void MakeSkinnedOpaqueDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc) noexcept;
	void MakeSkinnedShadowOpaqueDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc) noexcept;
	void MakeSkinnedDrawNormals(D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc) noexcept;
	void MakeShadowDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc) noexcept;
	void MakeDrawNormals(D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc) noexcept;
	void MakeSsaoDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc) noexcept;
	void MakeSsaoBlurDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc) noexcept;
	void MakeDebugDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc) noexcept;

private:
	CRenderer* m_renderer;
	std::map<GraphicsPSO, Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_psoList;
};
