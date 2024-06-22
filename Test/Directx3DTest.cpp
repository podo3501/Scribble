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
#include "../Core/UploadBuffer.h"
#include "../Core/GameTimer.h"
#include "../SecondPage/Camera.h"
#include "../SecondPage/Texture.h"
#include "../SecondPage/Renderer.h"
#include "../SecondPage/Shader.h"
#include "../SecondPage/Model.h"
#include "../SecondPage/FrameResource.h"
#include "../SecondPage/Material.h"
#include "../SecondPage/MainLoop.h"
#include "../SecondPage/RendererData.h"
#include "../SecondPage/KeyInputManager.h"
#include "../SecondPage/Geometry.h"

namespace STDTest
{
	TEST(std, all_of)
	{
		std::vector<int> a = { 1, 2, 3, 4, 5 };
		std::vector<int> b{};
		bool result = std::all_of(a.begin(), a.end(), [&b](auto& iter) {
			if (iter == 3)
				return false;
			b.emplace_back(iter);
			return true;
			});

		EXPECT_EQ(static_cast<int>(b.size()), 2);
		EXPECT_EQ(result, false);
	}
}

namespace MainLoop 
{
	class MainLoopTest : public ::testing::Test
	{
	public:
		MainLoopTest()
		{}

	protected:
		void SetUp() override
		{
			//�ʱ�ȭ
			m_window = std::make_unique<CWindow>(GetModuleHandle(nullptr));
			EXPECT_EQ(m_window->Initialize(false), true);

			m_directx3D = std::make_unique<CDirectx3D>(m_window.get());
			EXPECT_EQ(m_directx3D->Initialize(), true);

			m_renderer = std::make_unique<CRenderer>(m_resourcePath);
			EXPECT_EQ(m_renderer->Initialize(m_directx3D.get()), true);

			m_material = std::make_unique<CMaterial>();
			m_material->Build();
		}

		void TearDown() override
		{
			m_material.reset();
			m_renderer.reset();
			m_directx3D.reset();
			m_window.reset();
		}

	protected:
		const std::wstring m_resourcePath = L"../Resource/";

		std::unique_ptr<CWindow> m_window{ nullptr };
		std::unique_ptr<CDirectx3D> m_directx3D{ nullptr };
		std::unique_ptr<CRenderer> m_renderer{ nullptr };
		std::unique_ptr<CMaterial> m_material{ nullptr };
	};

	TEST_F(MainLoopTest, FrameResourceTest)
	{
		std::unique_ptr<CFrameResources> frameResources = std::make_unique<CFrameResources>();
		EXPECT_EQ(frameResources->BuildFrameResources(
			m_renderer->GetDevice(), 1, 125, static_cast<UINT>(m_material->GetCount())), true );
		EXPECT_EQ(frameResources->Synchronize(m_directx3D.get()), true);
		EXPECT_EQ(frameResources->GetUploadBuffer(eBufferType::PassCB) != nullptr, true);
	}

	TEST_F(MainLoopTest, Initialize)
	{
		//�����͸� �ý��� �޸𸮿� �ø���
		std::unique_ptr<CTexture> texture = std::make_unique<CTexture>(m_resourcePath);
		std::unique_ptr<CGeometry> geometry = std::make_unique<CGeometry>();
		std::unique_ptr<CModel> model = std::make_unique<CModel>(m_resourcePath);

		EXPECT_EQ(model->Read(), true);

		//�����Ӵ� ���̴� ������ ������ Ȯ��
		std::unique_ptr<CFrameResources> m_frameResources = std::make_unique<CFrameResources>();
		EXPECT_EQ(m_frameResources->BuildFrameResources(
			m_directx3D->GetDevice(), 1, 125, static_cast<UINT>(m_material->GetCount())), true);

		EXPECT_EQ(model->Convert(geometry.get()), true);
		//�ý��� �޸𸮿��� �׷��� �޸𸮿� ������ �ø���
		EXPECT_EQ(geometry->LoadGraphicMemory(m_directx3D.get()), true);
		EXPECT_EQ(texture->LoadGraphicMemory(m_directx3D.get(), m_renderer.get()), true);

		std::unique_ptr<CCamera> camera = std::make_unique<CCamera>();
		camera->SetPosition(0.0f, 2.0f, -15.0f);
	}

	TEST_F(MainLoopTest, WindowMessage)
	{
		MSG msg = { 0 };
		msg.hwnd = m_window->GetHandle();
		msg.message = WM_SIZE;
		msg.lParam = MAKELONG(1024, 768);

		DispatchMessage(&msg);

		EXPECT_EQ(m_window->GetWidth(), 1024);
		EXPECT_EQ(m_window->GetHeight(), 768);
	}

	TEST_F(MainLoopTest, CameraUpdate)
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
		DirectX::XMFLOAT3 pos = camera.GetPosition3f();
		EXPECT_EQ(pos.z, 1.0f);
	}

	TEST(MainLoop, Initialize)
	{
		std::unique_ptr<CMainLoop> mainLoop = std::make_unique<CMainLoop>(L"../Resource/");
		EXPECT_EQ(mainLoop->Initialize(GetModuleHandle(nullptr), false), true);
	}
	//for�� std::�˰��� ���� �ٲٱ�
} //SecondPage

namespace A_SecondPage
{
	TEST(MainLoop, RunTest)
	{
		std::unique_ptr<CMainLoop> mainLoop = std::make_unique<CMainLoop>(L"../Resource/");
		EXPECT_EQ(mainLoop->Initialize(GetModuleHandle(nullptr)), true);
		EXPECT_EQ(mainLoop->Run(), true);
	}
}