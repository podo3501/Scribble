#include "Shader.h"
#include <ranges>
#include <algorithm>
#include "../Core/d3dUtil.h"
#include "../Include/RendererDefine.h"
#include "../Include/Types.h"

using Microsoft::WRL::ComPtr;
using enum ShaderType;

CShader::CShader(const std::wstring& resPath, const ShaderFileList& shaderFileList)
	: m_resPath(resPath)
	, m_shaderFileList(shaderFileList)
	, m_shaderList{}
	, m_inputLayout{}
{}
CShader::~CShader() = default;

bool CShader::IsShadowMap()
{
	return m_shaderFileList.contains(GraphicsPSO::ShadowMap);
}

std::vector<GraphicsPSO> CShader::GetPSOList()
{
	std::vector<GraphicsPSO> psoList{};
	std::ranges::copy(m_shaderFileList | std::views::keys, std::back_inserter(psoList));
	
	return psoList;
}

std::string GetShaderVersion(ShaderType shaderType)
{
	switch (shaderType)
	{
	case VS: return "vs_5_1";
	case PS: return "ps_5_1";
	}

	return "";
}

bool CShader::InsertShaderList(GraphicsPSO psoType, ShaderType shaderType, std::wstring&& filename)
{
	Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob{ nullptr };

	ReturnIfFalse(CoreUtil::CompileShader(
		std::move(filename), nullptr, "main", GetShaderVersion(shaderType),
		&shaderBlob));

	m_shaderList[psoType][shaderType] = std::move(shaderBlob);

	return true;
}

inline D3D12_SHADER_BYTECODE CShader::GetShaderBytecode(GraphicsPSO psoType, ShaderType shaderType) 
{
	const auto& shader = m_shaderList[psoType][shaderType];
	return { shader->GetBufferPointer(), shader->GetBufferSize() };
}

std::wstring CShader::GetShaderFilename(GraphicsPSO psoType, ShaderType shaderType)
{
	const auto& psoFileList = m_shaderFileList[psoType];
	auto find = std::ranges::find_if(psoFileList, [shaderType](auto& file) {
		return file.first == shaderType; });
	if (find == psoFileList.end()) return {};
	
	return m_resPath + m_filePath + find->second;
}

std::vector<D3D12_INPUT_ELEMENT_DESC> GetLayout(GraphicsPSO psoType)
{
	std::vector<D3D12_INPUT_ELEMENT_DESC> layout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	return layout; 
}

bool CShader::SetPipelineStateDesc(GraphicsPSO psoType, D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc)
{
	auto count = std::ranges::count_if(m_shaderFileList[psoType], [](auto& fileList) {
		auto isVS = fileList.first == VS && !fileList.second.empty();
		auto isPS = fileList.first == PS && !fileList.second.empty();
		return isVS || isPS; });

	if (count < 2) return false;	//vs, ps가 없는 경우는 pipeline을 만들지 않는다.

	ReturnIfFalse(InsertShaderList(psoType, VS, GetShaderFilename(psoType, VS)));
	ReturnIfFalse(InsertShaderList(psoType, PS, GetShaderFilename(psoType, PS)));

	m_inputLayout = GetLayout(psoType);

	inoutDesc->VS = GetShaderBytecode(psoType, VS);
	inoutDesc->PS = GetShaderBytecode(psoType, PS);
	inoutDesc->InputLayout = { m_inputLayout.data(), static_cast<UINT>(m_inputLayout.size()) };

	return true;
}