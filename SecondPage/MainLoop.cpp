#include "MainLoop.h"
#include <WinUser.h>
#include "../Core/d3dUtil.h"
#include "../Core/Window.h"
#include "../Core/Directx3D.h"
#include "./Shader.h"
#include "./Renderer.h"
#include "./RendererDefine.h"
#include "./Material.h"
#include "./Model.h"
#include "./FrameResource.h"
#include "./Texture.h"
#include "./Camera.h"
#include "./KeyInputManager.h"

void CMainLoop::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return;
}

HRESULT CMainLoop::Initialize(HINSTANCE hInstance)
{
	static CMainLoop* gMainLoop = this;
	m_window = std::make_unique<CWindow>(hInstance);
	ReturnIfFailed(m_window->Initialize());
	m_window->AddWndProcListener([mainLoop = this](HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
		mainLoop->WndProc(wnd, msg, wp, lp); });

	m_directx3D = std::make_unique<CDirectx3D>(m_window.get());
	ReturnIfFailed(m_directx3D->Initialize());;

	m_renderer = std::make_shared<CRenderer>(m_directx3D.get());
	ReturnIfFalse(m_renderer->Initialize());

	m_camera = std::make_unique<CCamera>();
	m_camera->SetPosition(0.0f, 2.0f, -15.0f);

	//데이터를 시스템 메모리에 올리기
	m_material = std::make_unique<CMaterial>();
	m_material->Build();

	m_texture = std::make_unique<CTexture>(m_resourcePath + L"Textures/");
	m_model = std::make_unique<CModel>();

	auto geo = std::make_unique<MeshGeometry>();
	m_model->Read(geo.get());
	m_model->BuildRenderItems(geo.get(), m_material.get(), m_renderItems);
	std::string geoName = geo->Name;
	m_geometries[geoName] = std::move(geo);

	BuildFrameResource();			//프레임당 쓰이는 데이터 공간을 확보
	BuildGraphicMemory();			//시스템 메모리에서 그래픽 메모리에 데이터 올리기
	AddKeyListener();

	return S_OK;
}

void CMainLoop::BuildFrameResource()
{
	for (auto i{ 0 }; i < gFrameResourceCount; ++i)
	{
		auto frameRes = std::make_unique<FrameResource>(m_renderer->GetDevice(), 1,
			125, static_cast<UINT>(m_material->GetCount()));
		m_frameResources.emplace_back(std::move(frameRes));
	}
}

void CMainLoop::BuildGraphicMemory()
{
	//시스템 메모리에서 그래픽 메모리에 데이터 올리기
	m_directx3D->ResetCommandLists();

	m_texture->Load(m_renderer.get());
	for_each(m_geometries.begin(), m_geometries.end(), [mainLoop = this](auto& geometry) {
		auto& meshGeo = geometry.second;
		mainLoop->Load(meshGeo.get()); });

	m_directx3D->ExcuteCommandLists();
	m_directx3D->FlushCommandQueue();
}

void CMainLoop::Load(MeshGeometry* meshGeo)
{
	meshGeo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(
		m_renderer->GetDevice(), m_renderer->GetCommandList(),
		meshGeo->VertexBufferCPU->GetBufferPointer(),
		meshGeo->VertexBufferByteSize,
		meshGeo->VertexBufferUploader);

	meshGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(
		m_renderer->GetDevice(), m_renderer->GetCommandList(),
		meshGeo->IndexBufferCPU->GetBufferPointer(),
		meshGeo->IndexBufferByteSize,
		meshGeo->IndexBufferUploader);

	return;
}

void CMainLoop::AddKeyListener()
{
	m_keyInputManager = std::make_unique<CKeyInputManager>();
	m_keyInputManager->AddListener([&cam = m_camera](std::vector<int> keyList) {
		cam->PressedKey(keyList); });
	m_keyInputManager->AddListener([mainLoop = this](std::vector<int> keyList) {
		mainLoop->PressedKey(keyList); });
}

void CMainLoop::PressedKey(std::vector<int> keyList)
{
	for (auto vKey : keyList)
	{
		switch (vKey)
		{
		case '1':		m_frustumCullingEnabled = true;			break;
		case '2':		m_frustumCullingEnabled = false;		break;
		}
	}
}