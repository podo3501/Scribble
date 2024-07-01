#include "pch.h"
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <memory>
#include <unordered_map>
#include <functional>
#include "../Core/Directx3D.h"
#include "../Core/Window.h"
#include "../Core/d3dUtil.h"
#include "../Include/interface.h"
#include "../Include/RenderItem.h"
#include "../Include/FrameResourceData.h"
#include "../SecondPage/GameTimer.h"
#include "../SecondPage/Camera.h"
#include "../SecondPage/Texture.h"
#include "../SecondPage/Model.h"
#include "../SecondPage/Material.h"
#include "../SecondPage/MainLoop.h"
#include "../SecondPage/KeyInputManager.h"
#include "../SecondPage/Instance.h"

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

			m_renderer = CreateRenderer(m_resourcePath, m_window.get());
			EXPECT_EQ(m_renderer != nullptr, true);
		}

		void TearDown() override
		{
			m_window.reset();
			m_renderer.reset();
		}
		
	protected:
		std::wstring m_resourcePath{ L"../Resource/" };
		std::unique_ptr<CWindow> m_window{ nullptr };
		std::unique_ptr<IRenderer> m_renderer{ nullptr };
	};

	TEST_F(MainLoopClassTest, ModelTest)
	{
		//std::unique_ptr<CMaterial> material = std::make_unique<CMaterial>();
		//material->Build();

		//std::unique_ptr<CInstance> instance = std::make_unique<CInstance>();
		//instance->CreateInstanceData(nullptr, "nature", "cube");
		//instance->CreateInstanceData(material.get(), "things", "skull");

		//std::unique_ptr<CModel> model = std::make_unique<CModel>(m_resourcePath);

		//ModelTypeList modelTypeList
		//{
		//	ModelType(CreateType::Generator, "nature", "cube"), 
		//	ModelType(CreateType::ReadFile, "things", "skull", L"skull.txt")
		//};

		//std::unordered_map<std::string, std::unique_ptr<RenderItem>> renderItems{};
		//EXPECT_EQ(model->LoadGeometryList(modelTypeList), true);
		//EXPECT_EQ(model->LoadGraphicMemory(m_renderer.get(), &renderItems), true);
		//EXPECT_EQ(renderItems["things"]->vertexBufferGPU != nullptr, true );

		//EXPECT_EQ(instance->FillRenderItems(renderItems), true);
	}

	TEST_F(MainLoopClassTest, CameraUpdate)
	{
		auto deltaTime = 0.1f;

		CKeyInputManager keyInput;
		CCamera camera;
		camera.SetPosition(0.0f, 0.0f, 0.0f);
		camera.SetSpeed(eMove::Forward, 10.0f);
		keyInput.AddListener([&camera](std::vector<int> keyList) {
			camera.PressedKey(keyList); });
		keyInput.PressedKeyList([]() {
			return std::vector<int>{'W'};
			});
		camera.Update(deltaTime);
		DirectX::XMFLOAT3 pos = camera.GetPosition();
		EXPECT_EQ(pos.z, 1.0f);
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

	TEST_F(MainLoopClassTest, Instance)
	{
		std::unique_ptr<CModel> model = std::make_unique<CModel>(m_resourcePath);
		std::unique_ptr<CInstance> m_instance = std::make_unique<CInstance>();
		AllRenderItems renderItems{};
		EXPECT_EQ(m_instance->CreateMockData(), true);
		EXPECT_EQ(m_instance->LoadModel(model.get()), true);
		
		EXPECT_EQ(model->LoadGraphicMemory(m_renderer.get(), &renderItems), true);
		EXPECT_EQ(m_instance->FillRenderItems(renderItems), true);
		EXPECT_EQ(renderItems.empty(), false);
	}
} //SecondPage

class GTestRenderer : public IRenderer
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
		auto renderer = CreateRenderer(resPath, window.get());

		std::unique_ptr<CMainLoop> mainLoop = std::make_unique<CMainLoop>(resPath);
		EXPECT_EQ(mainLoop->Initialize(window.get(), renderer.get()), true);

		GTestRenderer testRenderer;
		EXPECT_EQ(mainLoop->Run(&testRenderer), true);
	}
}