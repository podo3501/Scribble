#include "Shader.h"
#include "../Core/Utility.h"
#include "../Core/d3dUtil.h"
#include <d3d12.h>

inline D3D12_SHADER_BYTECODE GetShaderBytecode(ID3DBlob* shader)
{
	return { shader->GetBufferPointer(), shader->GetBufferSize() };
}

CShader::CShader()
{}

void CShader::SetPipelineStateDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc)
{
	m_shaderList[toUType(ShaderType::VS)] = d3dUtil::CompileShader(L"../Resource/Shaders/VertexShader.hlsl", nullptr, "main", "vs_5_1");
	m_shaderList[toUType(ShaderType::PS)] = d3dUtil::CompileShader(L"../Resource/Shaders/PixelShader.hlsl", nullptr, "main", "ps_5_1");

	m_inputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	inoutDesc->VS = GetShaderBytecode(m_shaderList[toUType(ShaderType::VS)].Get());
	inoutDesc->PS = GetShaderBytecode(m_shaderList[toUType(ShaderType::PS)].Get());
	inoutDesc->InputLayout = { m_inputLayout.data(), static_cast<UINT>(m_inputLayout.size()) };
}