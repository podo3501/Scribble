#pragma once

#include <d3d12.h>
#include <vector>
#include <wrl.h>
#include <d3dcommon.h>
#include <array>
#include <string>
#include "../Core/headerUtility.h"

struct D3D12_GRAPHICS_PIPELINE_STATE_DESC;
struct D3D12_INPUT_ELEMENT_DESC;
enum class GraphicsPSO;

class CShader
{
	enum class ShaderType : int
	{
		VS,
		PS,
		Count,
	};

	inline static std::string m_shaderVersion[EtoV(ShaderType::Count)] =
	{
		"vs_5_1",
		"ps_5_1",
	};

public:
	CShader(std::wstring resPath);
	~CShader();

	CShader() = delete;
	CShader(const CShader&) = delete;
	CShader& operator=(const CShader&) = delete;

	bool SetPipelineStateDesc(GraphicsPSO psoType, D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc);

private:
	bool InsertShaderList(GraphicsPSO psoType, ShaderType shaderType, std::wstring&& filename);
	inline D3D12_SHADER_BYTECODE GetShaderBytecode(GraphicsPSO psoType, ShaderType shaderType) const;
	std::wstring GetShaderFilename(GraphicsPSO psoType, ShaderType shaderType);

private:
	std::wstring m_resPath{};
	std::wstring m_filePath{ L"Shaders/" };

	using ShaderList = std::array<Microsoft::WRL::ComPtr<ID3DBlob>, EtoV(ShaderType::Count)>;
	std::vector<ShaderList> m_shaderList{};

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout{};
};