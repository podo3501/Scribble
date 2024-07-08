#include "pch.h"
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <memory>
#include <unordered_map>
#include <functional>
#include "./InterfaceTest.h"
#include "../Include/RenderItem.h"
#include "../Include/FrameResourceData.h"
#include "../Include/types.h"
#include "../SecondPage/Window.h"
#include "../SecondPage/Camera.h"
#include "../SecondPage/Model.h"
#include "../SecondPage/Mesh.h"
#include "../SecondPage/Material.h"
#include "../SecondPage/MainLoop.h"
#include "../SecondPage/KeyInputManager.h"
#include "../SecondPage/SetupData.h"
#include "../SecondPage/MockData.h"
#include "../SecondPage/Helper.h"

namespace MainLoop
{
	class MainLoopClassTest : public ::testing::Test
	{
	public:
		MainLoopClassTest() {};

	protected:
		void SetUp() override
		{
			m_window = std::make_unique<CWindow>(GetModuleHandle(nullptr));
			EXPECT_EQ(m_window->Initialize(false), true);
			m_renderer = CreateRenderer(m_resourcePath, m_window->GetHandle(), m_window->GetWidth(), m_window->GetHeight());
			EXPECT_EQ(m_renderer != nullptr, true);

			m_material = std::make_unique<CMaterial>();
			m_setupData = std::make_unique<CSetupData>();
			EXPECT_EQ(m_setupData->InsertModelProperty("nature", "cube", CreateMock("cube"), m_material.get()), true);
		}

		void TearDown() override
		{
			m_setupData.reset();
			m_renderer.reset();
			m_window.reset();
		}
		
	protected:
		std::wstring m_resourcePath{ L"../Resource/" };
		std::unique_ptr<CWindow> m_window{ nullptr };
		std::unique_ptr<IRenderer> m_renderer{ nullptr };
		std::unique_ptr<CMaterial> m_material{ nullptr };
		std::unique_ptr<CSetupData> m_setupData{ nullptr };
	};

	TEST_F(MainLoopClassTest, CameraUpdate)
	{
		auto deltaTime = 0.1f;

		CKeyInputManager keyInput(m_window->GetHandle());
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
		materialList.emplace_back(MakeMaterial("bricks0", eTextureType::Common, L"bricks.dds", {}, {}, 0.1f));
		materialList.emplace_back(MakeMaterial("sky", eTextureType::Cube, L"grasscube1024.dds", {}, {}, 0.1f));
		materialList.emplace_back(MakeMaterial("bricks0", eTextureType::Common, L"bricks.dds", {}, {}, 0.1f));
		materialList.emplace_back(MakeMaterial("bricks1", eTextureType::Common, L"bricks.dds", {}, {}, 0.1f));
		materialList.emplace_back(MakeMaterial("bricks2", eTextureType::Common, L"bricks2.dds", {}, {}, 0.1f));

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
		virtual bool LoadTexture(const TextureList& textureList) override
		{
			EXPECT_EQ(textureList.size(), 3);

			return true;
		}
	};

	TEST_F(MainLoopClassTest, MockData)
	{
		std::unique_ptr<CMaterial> material = std::make_unique<CMaterial>();
		std::unique_ptr<CSetupData> setupData = std::make_unique<CSetupData>();
		setupData->InsertModelProperty("nature", "cube1", TestCreateMock(), material.get());
		setupData->InsertModelProperty("nature", "cube2", TestCreateMock(), material.get());

		std::unique_ptr<IRenderer> mockRenderer = std::make_unique<GMockTestRenderer>();
		EXPECT_EQ(material->LoadTextureIntoVRAM(mockRenderer.get()), true);
		EXPECT_EQ(material->GetDiffuseIndex(L"bricks.dds"), 0);
		EXPECT_EQ(material->GetDiffuseIndex(L"brickddddd"), -1);
		EXPECT_EQ(material->GetMaterialIndex("bricks1"), 2);
		EXPECT_EQ(material->GetMaterialIndex("bricks2"), 3);
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
	//데이터 넣는 부분을 테스트용으로 교체하자.
	TEST_F(MainLoopClassTest, Instance)
	{
		AllRenderItems allRenderItems{};
		std::unique_ptr<CModel> model = std::make_unique<CModel>();
		EXPECT_EQ(model->Initialize(m_resourcePath), true);
		EXPECT_EQ(model->LoadMemory(m_renderer.get(), allRenderItems), true);

		GInstanceRenderer renderer{};
		std::unique_ptr<CCamera> camera = std::make_unique<CCamera>();
		camera->OnResize(800, 600);
		camera->Update(0.1f);
		model->Update(&renderer, camera.get(), allRenderItems);

		SubRenderItem* subItem = GetSubRenderItem(allRenderItems, "things", "grid");
		EXPECT_EQ(subItem->instanceCount, 1);
		EXPECT_EQ(subItem->startSubIndexInstance, 115);
	}
} //SecondPage

class GTestRenderer : public ITestRenderer
{
	virtual bool Draw(std::unordered_map<std::string, std::unique_ptr<RenderItem>>& renderItem) override
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
		auto renderer = CreateRenderer(resPath, window->GetHandle(), window->GetWidth(), window->GetHeight());

		std::unique_ptr<CMainLoop> mainLoop = std::make_unique<CMainLoop>();
		EXPECT_EQ(mainLoop->Initialize(resPath, window.get(), renderer.get()), true);

		GTestRenderer testRenderer{};
		EXPECT_EQ(mainLoop->Run(&testRenderer), true);
	}
}