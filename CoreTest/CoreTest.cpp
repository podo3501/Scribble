#include "pch.h"
#include <ranges>
#include <algorithm>
#include "../Include/Types.h"
#include "../Core/Shader.h"
#include "../Core/d3dUtil.h"
#include "../Core/Renderer.h"
#include "../SecondPage/Window.h"

namespace Core
{
	using enum GraphicsPSO;
	using enum ShaderType;

	ShaderFileList GetShaderShadowTestFileList()
	{
		ShaderFileList shaderFileList{};
		auto InsertShaderFile = [&shaderFileList](GraphicsPSO pso, ShaderType type, const std::wstring filename) {
			shaderFileList[pso].emplace_back(std::make_pair(type, filename)); };

		InsertShaderFile(ShadowMap, VS, L"Shadow/VS.hlsl");
		InsertShaderFile(ShadowMap, PS, L"Shadow/PS.hlsl");

		return shaderFileList;
	}

	TEST(CRenderer, CreateShadowSRV)
	{
		std::wstring resPath = L"../Resource/";

		std::unique_ptr<CWindow> window = std::make_unique<CWindow>(GetModuleHandle(nullptr));
		window->Initialize(false);

		std::unique_ptr<CRenderer> renderer = std::make_unique<CRenderer>();
		EXPECT_TRUE(renderer->Initialize(
			resPath, 
			window->GetHandle(),
			window->GetWidth(),
			window->GetHeight(),
			GetShaderShadowTestFileList()));

		std::vector<std::wstring> srvFilename{};
		renderer->LoadTexture({}, &srvFilename);
		EXPECT_TRUE(std::ranges::find(srvFilename, L"SHADOWMAP") == srvFilename.end());
	}
	TEST(CShader, Load)
	{
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
