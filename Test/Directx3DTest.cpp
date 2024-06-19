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
#include "../Core/GameTimer.h"

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
	protected:
		void SetUp() override
		{}

	public:
	};

	TEST_F(MainLoopTest, Initialize)
	{
		const std::wstring resourcePath = L"../Resource/";
		//초기화
		std::unique_ptr<CWindow> window = std::make_unique<CWindow>(GetModuleHandle(nullptr));
		EXPECT_EQ(window->Initialize(), true);

		std::unique_ptr<CDirectx3D> directx3D = std::make_unique<CDirectx3D>(window.get());
		EXPECT_EQ(directx3D->Initialize(), true);

		std::shared_ptr<CRenderer> renderer = std::make_shared<CRenderer>(directx3D.get());
		EXPECT_EQ(renderer->Initialize(), true);

		//데이터를 시스템 메모리에 올리기
		std::unique_ptr<CMaterial> material = std::make_unique<CMaterial>();
		material->Build();

		std::unique_ptr<CTexture> texture = std::make_unique<CTexture>(resourcePath + L"Textures/");
		std::unique_ptr<CModel> model = std::make_unique<CModel>();

		std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> geometries;
		auto geo = std::make_unique<MeshGeometry>();
		model->Read(geo.get());
		std::string geoName = geo->Name;
		geometries[geoName] = std::move(geo);

		//프레임당 쓰이는 데이터 공간을 확보
		const int gNumFrameResources = 3;
		std::vector<std::unique_ptr<FrameResource>> m_frameResources;
		for (auto i{ 0 }; i < gNumFrameResources; ++i)
		{
			auto frameRes = std::make_unique<FrameResource>(renderer->GetDevice(), 1,
				125, static_cast<UINT>(material->GetCount()));
			m_frameResources.emplace_back(std::move(frameRes));
		}

		//시스템 메모리에서 그래픽 메모리에 데이터 올리기
		directx3D->ResetCommandLists();

		texture->Load(renderer.get());
		Load(geometries[geoName].get(), renderer.get());

		directx3D->ExcuteCommandLists();
		directx3D->FlushCommandQueue();

		std::unique_ptr<CCamera> camera = std::make_unique<CCamera>();
		camera->SetPosition(0.0f, 2.0f, -15.0f);
	}

	TEST_F(MainLoopTest, WindowMessage)
	{
		std::unique_ptr<CWindow> window = std::make_unique<CWindow>(GetModuleHandle(nullptr));
		EXPECT_EQ(window->Initialize(), true);

		MSG msg = { 0 };
		msg.hwnd = window->GetHandle();
		msg.message = WM_SIZE;
		msg.lParam = MAKELONG(1024, 768);

		DispatchMessage(&msg);

		EXPECT_EQ(window->GetWidth(), 1024);
		EXPECT_EQ(window->GetHeight(), 768);
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

	TEST_F(MainLoopTest, RunTest)
	{
		std::unique_ptr<CMainLoop> mainLoop = std::make_unique<CMainLoop>(L"../Resource/");
		EXPECT_EQ(mainLoop->Initialize(GetModuleHandle(nullptr)), S_OK);
		mainLoop->Run();
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

			m_frameResource = std::make_unique<FrameResource>(m_directx3D->GetDevice(), 1, 125, static_cast<UINT>(m_material->GetCount()));
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
		std::unique_ptr<FrameResource> m_frameResource{ nullptr };
	};

	TEST_F(MainLoopUpdateTest, UpdateModel)
	{
		CGameTimer timer;
		timer.Reset();

		std::vector<std::unique_ptr<RenderItem>> renderItems{};
		m_model->BuildRenderItems(m_geometries.begin()->second.get(), m_material.get(), renderItems);
		DirectX::BoundingFrustum camFrustum{};

		m_model->Update(&timer, m_camera.get(), m_frameResource.get(), camFrustum, true, renderItems);
		InstanceData instanceData = renderItems.begin()->get()->Instances[2];
		EXPECT_EQ(instanceData.MaterialIndex, 2);
	}

	TEST_F(MainLoopUpdateTest, UpdateMaterial)
	{
		EXPECT_EQ(m_material->UpdateMaterialBuffer(m_frameResource.get()), true);
	}

} //SecondPage