#include "pch.h"
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <memory>
#include <unordered_map>
#include <functional>
#include "../Core/Directx3D.h"
#include "../Core/d3dUtil.h"
#include "../Include/interface.h"
#include "../Include/RenderItem.h"
#include "../Include/FrameResourceData.h"
#include "../SecondPage/Window.h"
#include "../SecondPage/GameTimer.h"
#include "../SecondPage/Camera.h"
#include "../SecondPage/Model.h"
#include "../SecondPage/Material.h"
#include "../SecondPage/MainLoop.h"
#include "../SecondPage/KeyInputManager.h"
#include "../SecondPage/SetupData.h"

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
			m_setupData = std::make_unique<CSetupData>();
			EXPECT_EQ(m_setupData->CreateMockData(), true);
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

	TEST_F(MainLoopClassTest, Instance)
	{
		AllRenderItems renderItems{};
		std::unique_ptr<CModel> model = std::make_unique<CModel>(m_resourcePath);
		EXPECT_EQ(m_setupData->LoadModel(model.get(), &renderItems), true);
		EXPECT_EQ(model->LoadModelIntoVRAM(m_renderer.get(), &renderItems), true);
		EXPECT_EQ(renderItems.empty(), false);
	}

	TEST_F(MainLoopClassTest, Texture)
	{
		std::unique_ptr<CMaterial> material = std::make_unique<CMaterial>();
		EXPECT_EQ(m_setupData->LoadTextureIntoVRAM(m_renderer.get(), material.get()), true);
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
		auto renderer = CreateRenderer(resPath, window->GetHandle(), window->GetWidth(), window->GetHeight());

		std::unique_ptr<CMainLoop> mainLoop = std::make_unique<CMainLoop>();
		EXPECT_EQ(mainLoop->Initialize(resPath, window.get(), renderer.get()), true);

		GTestRenderer testRenderer;
		EXPECT_EQ(mainLoop->Run(&testRenderer), true);
	}
}