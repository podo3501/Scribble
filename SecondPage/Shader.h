#pragma once

#include <vector>
#include <wrl.h>
#include <d3dcommon.h>
#include <array>
#include <string>

struct D3D12_GRAPHICS_PIPELINE_STATE_DESC;
struct D3D12_INPUT_ELEMENT_DESC;

class CShader
{
	enum class ShaderType : int
	{
		VS,
		PS,
		Count,
	};

public:
	CShader();
	bool SetPipelineStateDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc);

	CShader(const CShader&) = delete;
	CShader& operator=(const CShader&) = delete;

private:
	std::string GetShaderVersion(ShaderType shaderType);
	bool InsertShaderList(ShaderType shaderType, std::wstring&& filename);

private:
	std::array<Microsoft::WRL::ComPtr<ID3DBlob>, static_cast<size_t>(ShaderType::Count)> m_shaderList{};
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout{};
};