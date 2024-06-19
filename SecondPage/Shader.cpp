#include "Shader.h"
#include "../Core/Utility.h"
#include "../Core/d3dUtil.h"
#include <d3d12.h>

using Microsoft::WRL::ComPtr;

inline D3D12_SHADER_BYTECODE GetShaderBytecode(ID3DBlob* shader)
{
	return { shader->GetBufferPointer(), shader->GetBufferSize() };
}

CShader::CShader()
{}

std::string CShader::GetShaderVersion(ShaderType shaderType)
{
	switch (shaderType)
	{
	case ShaderType::VS:	return "vs_5_1";	break;
	case ShaderType::PS: return "ps_5_1";	break;
	}
	return "";
}

bool CShader::InsertShaderList(ShaderType shaderType, std::wstring&& filename)
{
	std::string shaderVersion = GetShaderVersion(shaderType);
	if (shaderVersion.empty())
		return false;
	
	ReturnIfFalse(CoreUtil::CompileShader(
		std::move(filename), nullptr, "main", std::move(shaderVersion), &m_shaderList[toUType(shaderType)]));

	return true;
}

bool CShader::SetPipelineStateDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc)
{
	ReturnIfFalse(InsertShaderList(ShaderType::VS, L"../Resource/Shaders/VertexShader.hlsl"));
	ReturnIfFalse(InsertShaderList(ShaderType::PS, L"../Resource/Shaders/PixelShader.hlsl"));

	m_inputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	inoutDesc->VS = GetShaderBytecode(m_shaderList[toUType(ShaderType::VS)].Get());
	inoutDesc->PS = GetShaderBytecode(m_shaderList[toUType(ShaderType::PS)].Get());
	inoutDesc->InputLayout = { m_inputLayout.data(), static_cast<UINT>(m_inputLayout.size()) };

	return true;
}