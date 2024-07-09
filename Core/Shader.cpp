#include "Shader.h"
#include <ranges>
#include <algorithm>
#include "../Core/d3dUtil.h"
#include "../Include/RendererDefine.h"
#include "../Include/Types.h"

using Microsoft::WRL::ComPtr;

CShader::CShader(const std::wstring& resPath, const ShaderFileList& shaderFileList)
	: m_resPath(std::move(resPath))
	, m_shaderFileList(shaderFileList)
	, m_shaderList{}
	, m_inputLayout{}
{
	m_shaderList.resize(EtoV(GraphicsPSO::Count));
	std::ranges::for_each(m_shaderList, [](auto& shader) {
		shader.resize(EtoV(ShaderType::Count)); });
}
CShader::~CShader() = default;

inline static std::string m_shaderVersion[EtoV(ShaderType::Count)] =
{
	"vs_5_1",
	"ps_5_1",
};

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
	const auto& psoFileList = m_shaderFileList[psoType];
	auto find = std::ranges::find_if(psoFileList, [shaderType](auto& file) {
		return file.first == shaderType; });
	if (find == psoFileList.end()) return {};
	
	return m_resPath + m_filePath + find->second;
}

bool CShader::SetPipelineStateDesc(GraphicsPSO psoType, D3D12_GRAPHICS_PIPELINE_STATE_DESC* inoutDesc)
{
	auto count = std::ranges::count_if(m_shaderFileList[psoType], [](auto& fileList) {
		auto isVS = fileList.first == ShaderType::VS && !fileList.second.empty();
		auto isPS = fileList.first == ShaderType::PS && !fileList.second.empty();
		return isVS || isPS; });

	if (count < 2) return true;	//vs, ps가 없는 경우는 pipeline을 만들지 않는다.

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