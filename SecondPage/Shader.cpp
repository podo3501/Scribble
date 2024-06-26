#include "Shader.h"
#include "../Core/d3dUtil.h"
#include "./RendererDefine.h"

using Microsoft::WRL::ComPtr;

CShader::CShader(std::wstring& resPath)
	: m_resPath(std::move(resPath))
{
	m_shaderList.resize(EtoV(GraphicsPSO::Count));
}

bool CShader::InsertShaderList(GraphicsPSO psoType, ShaderType shaderType, std::wstring&& filename)
{
	ReturnIfFalse(CoreUtil::CompileShader(
		std::move(filename), nullptr, "main", m_shaderVersion[EtoV(shaderType)],
		&m_shaderList[EtoV(psoType)][EtoV(shaderType)]));

	return true;
}

inline D3D12_SHADER_BYTECODE CShader::GetShaderBytecode(GraphicsPSO psoType, ShaderType shaderType) const
{
	auto shader = m_shaderList[EtoV(psoType)][EtoV(shaderType)].Get();
	return { shader->GetBufferPointer(), shader->GetBufferSize() };
}

std::wstring CShader::GetShaderFilename(GraphicsPSO psoType, ShaderType shaderType)
{
	static std::wstring shaderFilename[EtoV(GraphicsPSO::Count)][EtoV(ShaderType::Count)] =
	{
		{ L"SkyVS.hlsl", L"SkyPS.hlsl"},
		{ L"VertexShader.hlsl", L"PixelShader.hlsl"},
	};
	return m_resPath + m_filePath + shaderFilename[EtoV(psoType)][EtoV(shaderType)];
}

bool CShader::SetPipelineStateDesc(GraphicsPSO psoType, D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc)
{
	ReturnIfFalse(InsertShaderList(psoType, ShaderType::VS, GetShaderFilename(psoType, ShaderType::VS)));
	ReturnIfFalse(InsertShaderList(psoType, ShaderType::PS, GetShaderFilename(psoType, ShaderType::PS)));

	m_inputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	inoutDesc->VS = GetShaderBytecode(psoType, ShaderType::VS);
	inoutDesc->PS = GetShaderBytecode(psoType, ShaderType::PS);
	inoutDesc->InputLayout = { m_inputLayout.data(), static_cast<UINT>(m_inputLayout.size()) };

	return true;
}