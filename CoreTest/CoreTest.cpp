#include "pch.h"
#include "../Include/Types.h"
#include "../Core/Shader.h"
#include "../Core/d3dUtil.h"

namespace Core
{
	TEST(CShader, Load)
	{
		using enum GraphicsPSO;
		using enum ShaderType;

		using ShaderFileList = std::map<GraphicsPSO, std::vector<std::pair<ShaderType, std::wstring>>>;

		ShaderFileList shaderFileList{};
		auto InsertShaderFile = [&shaderFileList](GraphicsPSO pso, ShaderType type, const std::wstring filename) {
			shaderFileList[pso].emplace_back(std::make_pair(type, filename)); };

		InsertShaderFile(NormalOpaque, VS, L"NormalOpaque/VS.hlsl");
		InsertShaderFile(NormalOpaque, PS, L"NormalOpaque/PS.hlsl");

		std::unique_ptr<CShader> shader = std::make_unique<CShader>(L"../Resource/", shaderFileList);

		EXPECT_EQ(shader->GetPSOList().size(), 1);

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
		EXPECT_TRUE(shader->SetPipelineStateDesc(NormalOpaque, &psoDesc));
	}
}
