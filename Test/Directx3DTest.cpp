#include "pch.h"
#include "../Core/Directx3D.h"
#include "../Core/Window.h"
#include "../SecondPage/Camera.h"
#include "../SecondPage/Texture.h"
#include "../SecondPage/Renderer.h"
#include "../SecondPage/Shader.h"
#include "../SecondPage/Model.h"
#include "../SecondPage/FrameResource.h"
#include "../SecondPage/Material.h"
#include "../Core/d3dUtil.h"
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <memory>
#include <unordered_map>

LRESULT CALLBACK
TestWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

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

namespace MainLoop
{
	TEST(CoreClass, Initialize)
	{
		const std::wstring resourcePath = L"../Resource/";
		//초기화
		std::unique_ptr<CDirectx3D> directx3D = std::make_unique<CDirectx3D>(GetModuleHandle(nullptr));
		EXPECT_EQ(directx3D->Initialize(TestWndProc), true);

		std::shared_ptr<CRenderer> renderer = std::make_shared<CRenderer>(directx3D.get());
		EXPECT_EQ(renderer->Initialize(), true);

		//데이터를 시스템 메모리에 올리기
		std::unique_ptr<CMaterial> material = std::make_unique<CMaterial>();
		material->Build();

		std::unique_ptr<CTexture> texture = std::make_unique<CTexture>(resourcePath +L"Textures/");
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

	class CMainLoop
	{
	public:
		CMainLoop() = default;
		~CMainLoop() = default;

		CMainLoop(const CMainLoop&) = delete;
		CMainLoop& operator=(const CMainLoop&) = delete;

		template<typename T>
		HRESULT Initialize(HINSTANCE hInstance, T&& resourcePath);

	private:
		std::wstring m_resourcePath{};
	};

	template<typename T>
	HRESULT CMainLoop::Initialize(HINSTANCE hInstance, T&& resourcePath)
	{
		m_resourcePath = std::forward<T>(resourcePath);
		
		std::unique_ptr<CDirectx3D> directx3D = std::make_unique<CDirectx3D>(GetModuleHandle(nullptr));
		ReturnIfFailed(directx3D->Initialize(TestWndProc));

		std::shared_ptr<CRenderer> renderer = std::make_shared<CRenderer>(directx3D.get());
		ReturnIfFalse(renderer->Initialize());

		return S_OK;
	}

	TEST(MainLoop, Initialize)
	{
		std::unique_ptr<CMainLoop> mainLoop = std::make_unique<CMainLoop>();
		EXPECT_EQ(mainLoop->Initialize(GetModuleHandle(nullptr), L"../Resource/"), S_OK);
	}
}

