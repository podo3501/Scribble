#pragma once

#include <d3d12.h>
#include <vector>
#include <map>
#include <wrl.h>
#include <d3dcommon.h>
#include <array>
#include <string>
#include "../Core/headerUtility.h"

struct D3D12_GRAPHICS_PIPELINE_STATE_DESC;
struct D3D12_INPUT_ELEMENT_DESC;
enum class GraphicsPSO : int;
enum class ShaderType : int;

class CShader
{
	using ShaderFileList = std::map<GraphicsPSO, std::vector<std::pair<ShaderType, std::wstring>>>;

public:
	CShader(const std::wstring& resPath, const ShaderFileList& shaderFileList);
	~CShader();

	CShader() = delete;
	CShader(const CShader&) = delete;
	CShader& operator=(const CShader&) = delete;

	std::vector<GraphicsPSO> GetPSOList();
	bool SetPipelineStateDesc(GraphicsPSO psoType, D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc);

private:
	bool InsertShaderList(GraphicsPSO psoType, ShaderType shaderType, std::wstring&& filename);
	inline D3D12_SHADER_BYTECODE GetShaderBytecode(GraphicsPSO psoType, ShaderType shaderType);
	std::wstring GetShaderFilename(GraphicsPSO psoType, ShaderType shaderType);

private:
	using ShaderList = std::map<ShaderType, Microsoft::WRL::ComPtr<ID3DBlob>>;

	std::wstring m_resPath{};
	std::wstring m_filePath{ L"Shaders/" };

	ShaderFileList m_shaderFileList;
	std::map<GraphicsPSO, ShaderList> m_shaderList;
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;
};