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

void Load(MeshGeometry* meshGeo, CRenderer* renderer)
{
	meshGeo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(
		renderer->GetDevice(), renderer->GetCommandList(), 
		meshGeo->VertexBufferCPU->GetBufferPointer(), 
		meshGeo->VertexBufferByteSize,
		meshGeo->VertexBufferUploader);
	
	meshGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(
		renderer->GetDevice(), renderer->GetCommandList(), 
		meshGeo->IndexBufferCPU->GetBufferPointer(),
		meshGeo->IndexBufferByteSize,
		meshGeo->IndexBufferUploader);

	return;
}

namespace SecondPage 
{
	class MainLoopTest : public ::testing::Test
	{
	public:
		MainLoopTest()
		{}

	protected:
		void SetUp() override
		{
			//초기화
			m_window = std::make_unique<CWindow>(GetModuleHandle(nullptr));
			EXPECT_EQ(m_window->Initialize(), true);

			m_directx3D = std::make_unique<CDirectx3D>(m_window.get());
			EXPECT_EQ(m_directx3D->Initialize(), true);

			m_renderer = std::make_unique<CRenderer>(m_directx3D.get());
			EXPECT_EQ(m_renderer->Initialize(), true);

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
		frameResources->Synchronize(m_directx3D->GetFence());
		EXPECT_EQ(frameResources->GetUploadBuffer(eBufferType::PassCB) != nullptr, true);
	}

	TEST_F(MainLoopTest, Initialize)
	{
		//데이터를 시스템 메모리에 올리기
		std::unique_ptr<CTexture> texture = std::make_unique<CTexture>(m_resourcePath + L"Textures/");
		std::unique_ptr<CModel> model = std::make_unique<CModel>();

		std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> geometries;
		auto geo = std::make_unique<MeshGeometry>();
		model->Read(geo.get());
		std::string geoName = geo->Name;
		geometries[geoName] = std::move(geo);

		//프레임당 쓰이는 데이터 공간을 확보
		std::unique_ptr<CFrameResources> m_frameResources = std::make_unique<CFrameResources>();
		m_frameResources->BuildFrameResources(
			m_directx3D->GetDevice(), 1, 125, static_cast<UINT>(m_material->GetCount()));

		//시스템 메모리에서 그래픽 메모리에 데이터 올리기
		m_directx3D->ResetCommandLists();

		texture->Load(m_renderer.get());
		Load(geometries[geoName].get(), m_renderer.get());

		m_directx3D->ExcuteCommandLists();
		m_directx3D->FlushCommandQueue();

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

	class MainLoopUpdateTest : public ::testing::Test
	{
	public:
		MainLoopUpdateTest()
		{
			m_window = std::make_unique<CWindow>(GetModuleHandle(nullptr));
			m_window->Initialize();

			m_directx3D = std::make_unique<CDirectx3D>(m_window.get());
			m_directx3D->Initialize();

			m_material = std::make_unique<CMaterial>();
			m_material->Build();

			m_model = std::make_unique<CModel>();

			auto geo = std::make_unique<MeshGeometry>();
			m_model->Read(geo.get());
			std::string geoName = geo->Name;
			m_geometries[geoName] = std::move(geo);

			m_camera = std::make_unique<CCamera>();
			m_camera->UpdateViewMatrix();

			m_frameResources = std::make_unique<CFrameResources>();
			m_frameResources->BuildFrameResources(
				m_directx3D->GetDevice(), 1, 125, static_cast<UINT>(m_material->GetCount()));

			m_frameResources->Synchronize(m_directx3D->GetFence());
		}

	protected:
		void SetUp() override
		{}

	protected:
		std::unique_ptr<CWindow> m_window{ nullptr };
		std::unique_ptr<CDirectx3D> m_directx3D{ nullptr };
		std::unique_ptr<CMaterial> m_material{ nullptr };
		std::unique_ptr<CModel> m_model{ nullptr };
		std::unique_ptr<CCamera> m_camera{ nullptr };
		std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> m_geometries{};
		std::unique_ptr<CFrameResources> m_frameResources{ nullptr };
	};
	TEST_F(MainLoopUpdateTest, UpdateModel)
	{
		CGameTimer timer;
		timer.Reset();

		std::vector<std::unique_ptr<RenderItem>> renderItems{};
		m_model->BuildRenderItems(m_geometries.begin()->second.get(), m_material.get(), renderItems);
		DirectX::BoundingFrustum camFrustum{};

		m_model->Update(&timer, m_camera.get(), m_frameResources->GetUploadBuffer(eBufferType::Instance),
			camFrustum, true, renderItems);
		InstanceData instanceData = renderItems.begin()->get()->Instances[2];
		EXPECT_EQ(instanceData.MaterialIndex, 2);
	}

	TEST_F(MainLoopUpdateTest, UpdateMaterial)
	{
		EXPECT_EQ(m_material->UpdateMaterialBuffer(
			m_frameResources->GetUploadBuffer(eBufferType::Material)), true);
	}

	TEST(MainLoop, RunTest)
	{
		std::unique_ptr<CMainLoop> mainLoop = std::make_unique<CMainLoop>(L"../Resource/");
		EXPECT_EQ(mainLoop->Initialize(GetModuleHandle(nullptr)), S_OK);
		mainLoop->Run();
	}
} //SecondPage