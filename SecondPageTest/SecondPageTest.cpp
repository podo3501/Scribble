#include "pch.h"
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <memory>
#include <unordered_map>
#include <functional>
#include <ranges>
#include "./InterfaceTest.h"
#include "../Include/RenderItem.h"
#include "../Include/RendererDefine.h"
#include "../Include/FrameResourceData.h"
#include "../Include/types.h"
#include "../SecondPage/Window.h"
#include "../SecondPage/Camera.h"
#include "../SecondPage/Model.h"
#include "../SecondPage/Mesh.h"
#include "../SecondPage/Material.h"
#include "../SecondPage/Shadow.h"
#include "../SecondPage/MainLoop.h"
#include "../SecondPage/KeyInput.h"
#include "../SecondPage/SetupData.h"
#include "../SecondPage/MockData.h"
#include "../SecondPage/Helper.h"
#include "../SecondPage/Utility.h"

using enum GraphicsPSO;
using enum ShaderType;

namespace MainLoop
{
	ShaderFileList GetShaderTestFileList()
	{
		ShaderFileList shaderFileList{};
		auto InsertShaderFile = [&shaderFileList](GraphicsPSO pso, ShaderType type, const std::wstring filename) {
			shaderFileList[pso].emplace_back(std::make_pair(type, filename)); };

		InsertShaderFile(Sky, VS, L"Cube/VS.hlsl");
		InsertShaderFile(Sky, PS, L"Cube/PS.hlsl");
		InsertShaderFile(Opaque, VS, L"Opaque/VS.hlsl");
		InsertShaderFile(Opaque, PS, L"Opaque/PS.hlsl");
		InsertShaderFile(NormalOpaque, VS, L"NormalOpaque/VS.hlsl");
		InsertShaderFile(NormalOpaque, PS, L"NormalOpaque/PS.hlsl");
		InsertShaderFile(ShadowMap, VS, L"Shadow/VS.hlsl");
		InsertShaderFile(ShadowMap, PS, L"Shadow/PS.hlsl");

		return shaderFileList;
	}

	class MainLoopClassTest : public ::testing::Test
	{
	public:
		MainLoopClassTest() {};

	protected:
		void SetUp() override
		{
			m_window = std::make_unique<CWindow>(GetModuleHandle(nullptr));
			EXPECT_TRUE(m_window->Initialize(false));

			m_renderer = CreateRenderer(
				m_resourcePath,
				m_window->GetHandle(),
				m_window->GetWidth(),
				m_window->GetHeight(),
				GetShaderTestFileList());
			EXPECT_TRUE(m_renderer != nullptr);
		}

		void TearDown() override
		{
			m_renderer.reset();
			m_window.reset();
		}

	protected:
		std::wstring m_resourcePath{ L"../Resource/" };
		std::unique_ptr<CWindow> m_window{ nullptr };
		std::unique_ptr<IRenderer> m_renderer{ nullptr };
	};

	TEST_F(MainLoopClassTest, CameraUpdate)
	{
		auto deltaTime = 0.1f;

		CKeyInput keyInput(m_window->GetHandle());
		CCamera camera;
		camera.SetPosition(0.0f, 0.0f, 0.0f);
		camera.SetSpeed(eMove::Forward, 10.0f);
		keyInput.AddKeyListener([&camera](std::vector<int> keyList) {
			camera.PressedKey(keyList); });
		//mock으로 처리 가능할지 생각해보자
		//keyInput.PressedKeyList([]() {
		//	return std::vector<int>{'W'};
		//	});
		//camera.Update(deltaTime);
		//DirectX::XMFLOAT3 pos = camera.GetPosition();
		//EXPECT_EQ(pos.z, 1.0f);
	}

	TEST_F(MainLoopClassTest, WindowMessage)
	{
		MSG msg = { 0 };
		msg.hwnd = m_window->GetHandle();
		msg.message = WM_SIZE;
		msg.lParam = MAKELONG(1024, 768);

		DispatchMessage(&msg);

		EXPECT_EQ(m_window->GetWidth(), 1024);
		EXPECT_EQ(m_window->GetHeight(), 768);
	}

	ModelProperty TestCreateMock()
	{
		MaterialList materialList;
		materialList.emplace_back(MakeMaterial("bricks0", eTextureType::Texture2D, { L"bricks.dds" }, {}, {}, 0.1f));
		materialList.emplace_back(MakeMaterial("sky", eTextureType::TextureCube, { L"grasscube1024.dds" }, {}, {}, 0.1f));
		materialList.emplace_back(MakeMaterial("bricks0", eTextureType::Texture2D, { L"bricks.dds", L"bricks2_nmap.dds" }, {}, {}, 0.1f));
		materialList.emplace_back(MakeMaterial("bricks1", eTextureType::Texture2D, { L"bricks.dds" }, {}, {}, 0.1f));
		materialList.emplace_back(MakeMaterial("bricks2", eTextureType::Texture2D, { L"bricks2.dds", L"bricks2_nmap.dds" }, {}, {}, 0.1f));

		ModelProperty  modelProp{};
		modelProp.createType = ModelProperty::CreateType::Generator;
		modelProp.meshData = nullptr;
		modelProp.cullingFrustum = false;
		modelProp.filename = L"";
		modelProp.instanceDataList = {};
		modelProp.materialList = materialList;
		
		return modelProp;
	}

	class GMockTestRenderer : public ITestRenderer
	{
	public:
		GMockTestRenderer(IRenderer* originRenderer)
			: m_originRenderer(originRenderer) {}
		virtual bool LoadTexture(const TextureList& textureList) override
		{
			EXPECT_EQ(textureList.size(), 4);
			std::vector<std::wstring> curtexIndexList;
			//m_originRenderer->LoadTexture(textureList, &curtexIndexList);

			return true;
		}
	private:
		IRenderer* m_originRenderer{ nullptr };
	};

	TEST_F(MainLoopClassTest, Material)
	{
		std::unique_ptr<CMaterial> material = std::make_unique<CMaterial>();
		std::unique_ptr<CSetupData> setupData = std::make_unique<CSetupData>();
		setupData->InsertModelProperty(Opaque, "cube1", TestCreateMock(), material.get());
		setupData->InsertModelProperty(Opaque, "cube2", TestCreateMock(), material.get());

		std::unique_ptr<IRenderer> mockRenderer = std::make_unique<GMockTestRenderer>(m_renderer.get());
		EXPECT_TRUE(material->LoadTextureIntoVRAM(mockRenderer.get()));

		//EXPECT_EQ(material->GetTextureIndex(L"bricks.dds"), 0);
		//EXPECT_EQ(material->GetTextureIndex(L"brickddddd"), -1);
		//EXPECT_EQ(material->GetTextureIndex(L"bricks2_nmap.dds"), 3);
		//EXPECT_EQ(material->GetMaterialIndex("bricks1"), 2);
		//EXPECT_EQ(material->GetMaterialIndex("bricks2"), 3);
	}
	
	class GInstanceRenderer : public ITestRenderer
	{
		virtual bool SetUploadBuffer(eBufferType bufferType, const void* bufferData, size_t dataSize) override
		{
			if (bufferType == eBufferType::Material) return true;

			InstanceBuffer* startBuf = (InstanceBuffer*)(bufferData);
			auto size = sizeof(InstanceBuffer) * dataSize;
			std::vector<InstanceBuffer> instanceBuffers(startBuf, startBuf + dataSize);

			return true;
		}
	};

	CreateModelNames MakeTestMockData()
	{
		return CreateModelNames
		{
			{Sky, { "cube" } },
			{Opaque, { "skull" } },
			{NormalOpaque, { "grid" } },
		};
	}
	TEST_F(MainLoopClassTest, Instance)
	{
		AllRenderItems allRenderItems{};
		std::unique_ptr<CModel> model = std::make_unique<CModel>();
		EXPECT_TRUE(model->Initialize(m_resourcePath, MakeTestMockData()));
		EXPECT_TRUE(model->LoadMemory(m_renderer.get(), allRenderItems));

		GInstanceRenderer renderer{};
		std::unique_ptr<CCamera> camera = std::make_unique<CCamera>();
		camera->OnResize(800, 600);
		camera->Update(0.1f);
		model->Update(&renderer, camera.get(), allRenderItems);

		SubRenderItem* subItem = GetSubRenderItem(allRenderItems, NormalOpaque, "grid");
		EXPECT_EQ(subItem->instanceCount, 1);
		EXPECT_EQ(subItem->startSubIndexInstance, 0);
	}

	TEST_F(MainLoopClassTest, Shadow)
	{
		std::unique_ptr<CShadow> shadow = std::make_unique<CShadow>();
		shadow->Update(0.1f);

		PassConstants pc;
		shadow->GetPassCB(&pc);
		EXPECT_TRUE(pc.lights[0].direction.x != 0.57735f);
	}
} //SecondPage

class GTestRenderer : public ITestRenderer
{
	virtual bool Draw(AllRenderItems& renderItem) override
	{
		EXPECT_EQ(renderItem.empty(), false);
		PostQuitMessage(0);
		return true;
	};
};

namespace A_SecondPage
{
	TEST(MainLoop, RunTest)
	{
		std::wstring resPath = L"../Resource/";

		std::unique_ptr<CWindow> window = std::make_unique<CWindow>(GetModuleHandle(nullptr));
		window->Initialize(true);
		auto renderer = CreateRenderer(
			resPath, 
			window->GetHandle(), 
			window->GetWidth(), 
			window->GetHeight(),
			GetShaderFileList());

		std::unique_ptr<CMainLoop> mainLoop = std::make_unique<CMainLoop>();
		EXPECT_TRUE(mainLoop->Initialize(resPath, window.get(), renderer.get()));

		GTestRenderer testRenderer{};
		EXPECT_TRUE(mainLoop->Run(&testRenderer));
	}
}