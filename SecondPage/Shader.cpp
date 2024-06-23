#include "Shader.h"
//#include "../Core/Utility.h"
#include "../Core/d3dUtil.h"
#include <d3d12.h>

using Microsoft::WRL::ComPtr;

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

inline D3D12_SHADER_BYTECODE GetShaderBytecode(ID3DBlob* shader)
{
	return { shader->GetBufferPointer(), shader->GetBufferSize() };
}

bool CShader::SetPipelineStateDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc)
{
	std::wstring filePath = m_resPath + m_filePath;
	ReturnIfFalse(InsertShaderList(ShaderType::VS, filePath + L"VertexShader.hlsl"));
	ReturnIfFalse(InsertShaderList(ShaderType::PS, filePath + L"PixelShader.hlsl"));

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