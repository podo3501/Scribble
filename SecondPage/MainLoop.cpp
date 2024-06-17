#include "MainLoop.h"
#include <WinUser.h>
#include "../Core/d3dUtil.h"
#include "../Core/Window.h"
#include "../Core/Directx3D.h"
#include "./Shader.h"
#include "./Renderer.h"
#include "./Material.h"
#include "./Model.h"
#include "./FrameResource.h"
#include "./Texture.h"
#include "./Camera.h"

LRESULT CALLBACK
CMainLoop::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

HRESULT CMainLoop::Initialize(HINSTANCE hInstance)
{
	m_directx3D = std::make_unique<CDirectx3D>(hInstance);

	//windowŬ������ �Ѵܰ� �÷��� windowproc�� add��Ű�� �����ΰ���
	static CMainLoop* gMainLoop = this;
	ReturnIfFailed(m_directx3D->Initialize([](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)->LRESULT {
		return gMainLoop->WndProc(hwnd, msg, wParam, lParam);
		}));

	m_renderer = std::make_shared<CRenderer>(m_directx3D.get());
	ReturnIfFalse(m_renderer->Initialize());

	std::unique_ptr<CCamera> camera = std::make_unique<CCamera>();
	camera->SetPosition(0.0f, 2.0f, -15.0f);

	//�����͸� �ý��� �޸𸮿� �ø���
	m_material = std::make_unique<CMaterial>();
	m_material->Build();

	m_texture = std::make_unique<CTexture>(m_resourcePath + L"Textures/");
	m_model = std::make_unique<CModel>();

	auto geo = std::make_unique<MeshGeometry>();
	m_model->Read(geo.get());
	std::string geoName = geo->Name;
	m_geometries[geoName] = std::move(geo);

	//�����Ӵ� ���̴� ������ ������ Ȯ��
	BuildFrameResource();
	BuildGraphicMemory();

	return S_OK;
}
//initialize �� �Ѵ��� Ű ������ �۾�

void CMainLoop::BuildFrameResource()
{
	const int gNumFrameResources = 3;
	for (auto i{ 0 }; i < gNumFrameResources; ++i)
	{
		auto frameRes = std::make_unique<FrameResource>(m_renderer->GetDevice(), 1,
			125, static_cast<UINT>(m_material->GetCount()));
		m_frameResources.emplace_back(std::move(frameRes));
	}
}

void CMainLoop::BuildGraphicMemory()
{
	//�ý��� �޸𸮿��� �׷��� �޸𸮿� ������ �ø���
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