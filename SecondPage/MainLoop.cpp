#include "MainLoop.h"
#include <WinUser.h>
#include "../Core/d3dUtil.h"
#include "../Core/Utility.h"
#include "../Core/Window.h"
#include "../Include/RendererDefine.h"
#include "../Include/RenderItem.h"
#include "../Include/interface.h"
#include "../Include/FrameResourceData.h"
#include "./GameTimer.h"
#include "./Material.h"
#include "./Model.h"
#include "./Camera.h"
#include "./KeyInputManager.h"
#include "./Instance.h"
#include "./Helper.h"

using namespace DirectX;

RenderItem::RenderItem()
	: NumFramesDirty{ gFrameResourceCount }
{}

CMainLoop::~CMainLoop() = default;

CMainLoop::CMainLoop(std::wstring resourcePath)
{
	m_resourcePath = resourcePath;
}

bool CMainLoop::Initialize(CWindow* window, IRenderer* renderer)
{
	m_window = window;
	m_iRenderer = renderer;

	ReturnIfFalse(InitializeClass());	//초기화

	AddKeyListener();	//키 리스너 등록

	//local에서 작업할 것들
	ReturnIfFalse(OnResize(m_window->GetWidth(), m_window->GetHeight()));
	ReturnIfFalse(LoadMemory());	//데이터를 ram, vram에 올리기

	return true;
}

bool CMainLoop::InitializeClass()
{
	m_keyInputManager = std::make_unique<CKeyInputManager>(m_window->GetHandle());
	m_camera = std::make_unique<CCamera>();
	m_timer = std::make_unique<CGameTimer>();
	m_material = std::make_unique<CMaterial>();
	m_instance = std::make_unique<CInstance>();
	m_model = std::make_unique<CModel>(m_resourcePath);

	ReturnIfFalse(m_instance->CreateMockData());

	return true;
}

void CMainLoop::AddKeyListener()
{
	m_window->AddWndProcListener([&keyMng = m_keyInputManager](HWND wnd, UINT msg, WPARAM wp, LPARAM lp, LRESULT& lr)->bool {
		return keyMng->MsgProc(wnd, msg, wp, lp, lr); });
	m_window->AddOnResizeListener([this](int width, int height)->bool {
		return OnResize(width, height); });
	m_window->AddAppPauseListener([this](bool pause)->void {
		return SetAppPause(pause); });

	m_keyInputManager->AddKeyListener([&cam = m_camera](std::vector<int> keyList) {
		cam->PressedKey(keyList); });
	m_keyInputManager->AddKeyListener([this](std::vector<int> keyList) {
		PressedKey(keyList); });
	m_keyInputManager->AddMouseListener([&cam = m_camera](float dx, float dy) {
		cam->Move(dx, dy);	 });
}

bool CMainLoop::LoadMemory()
{
	ReturnIfFalse(m_instance->LoadModel(m_model.get(), &m_AllRenderItems));
	ReturnIfFalse(m_model->LoadModelIntoVRAM(m_iRenderer, &m_AllRenderItems));
	ReturnIfFalse(m_instance->LoadTextureIntoVRAM(m_iRenderer, m_material.get()));

	return true;
}

bool CMainLoop::IsInsideFrustum(const DirectX::BoundingSphere& bSphere, const XMMATRIX& invView, const XMMATRIX& world)
{
	BoundingFrustum frustum{};
	XMMATRIX invWorld = XMMatrixInverse(&RvToLv(XMMatrixDeterminant(world)), world);
	XMMATRIX viewToLocal = XMMatrixMultiply(invView, invWorld);

	m_camera->GetFrustum().Transform(frustum, viewToLocal);

	const bool isInside = (frustum.Contains(bSphere) != DirectX::DISJOINT);
	return (isInside || !m_frustumCullingEnabled);
}

std::wstring SetWindowCaption(std::size_t visibleCount, std::size_t totalCount)
{
	std::wostringstream outs;
	outs.precision(6);
	outs << L"Instancing and Culling Demo" <<
		L"    " << visibleCount <<
		L" objects visible out of " << totalCount;
	return outs.str();
}

void CMainLoop::FindVisibleSubRenderItems( SubRenderItems& subRenderItems, InstanceDataList& visibleInstance)
{
	XMMATRIX view = m_camera->GetView();
	XMMATRIX invView = XMMatrixInverse(&RvToLv(XMMatrixDeterminant(view)), view);

	for (auto& iterSubItem : subRenderItems)
	{
		auto& subRenderItem = iterSubItem.second;
		auto& instanceList = subRenderItem.instanceDataList;
		if (subRenderItem.cullingFrustum)
		{
			std::ranges::copy_if(instanceList, std::back_inserter(visibleInstance),
				[this, &invView, &subRenderItem](auto& instance) {
					return IsInsideFrustum(subRenderItem.subItem.boundingSphere, invView, instance->world);
				});
			m_windowCaption = SetWindowCaption(visibleInstance.size(), instanceList.size());
		}
		else
			std::ranges::copy(instanceList, std::back_inserter(visibleInstance));

		subRenderItem.instanceCount = static_cast<UINT>(visibleInstance.size());
	}
}

void CMainLoop::UpdateRenderItems()
{
	//처리 안할것을 먼저 골라낸다.
	InstanceDataList visibleInstance{};
	int instanceStartIndex{ 0 };
	for (auto& e : m_AllRenderItems)
	{
		auto renderItem = e.second.get();
		renderItem->startIndexInstance = instanceStartIndex;
		//보여지는 서브 아이템을 찾아낸다.
		FindVisibleSubRenderItems(renderItem->subRenderItems, visibleInstance);
		instanceStartIndex += static_cast<int>(visibleInstance.size());
	}
	UpdateInstanceBuffer(visibleInstance);
}

void CMainLoop::UpdateInstanceBuffer(const InstanceDataList& visibleInstance)
{
	std::vector<InstanceBuffer> instanceBufferDatas{};
	std::ranges::transform(visibleInstance, std::back_inserter(instanceBufferDatas), [](auto& visibleData) {
		InstanceBuffer curInsBuf{};
		XMStoreFloat4x4(&curInsBuf.world, XMMatrixTranspose(visibleData->world));
		XMStoreFloat4x4(&curInsBuf.texTransform, XMMatrixTranspose(visibleData->texTransform));
		curInsBuf.materialIndex = visibleData->matIndex;
		return std::move(curInsBuf); });

	m_iRenderer->SetUploadBuffer(eBufferType::Instance, instanceBufferDatas.data(), instanceBufferDatas.size());
}

void CMainLoop::UpdateMainPassCB()
{	
	PassConstants pc;
	m_camera->GetPassCB(&pc);
	m_timer->GetPassCB(&pc);

	float width = (float)m_window->GetWidth();
	float height = (float)m_window->GetHeight();
	pc.renderTargetSize = { width, height };
	pc.invRenderTargetSize = { 1.0f / width, 1.0f / height };

	pc.nearZ = 1.0f;
	pc.farZ = 1000.0f;
	pc.ambientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	pc.lights[0].direction = { 0.57735f, -0.57735f, 0.57735f };
	pc.lights[0].strength = { 0.8f, 0.8f, 0.8f };
	pc.lights[1].direction = { -0.57735f, -0.57735f, 0.57735f };
	pc.lights[1].strength = { 0.4f, 0.4f, 0.4f };
	pc.lights[2].direction = { 0.0f, -0.707f, -0.707f };
	pc.lights[2].strength = { 0.2f, 0.2f, 0.2f };

	m_iRenderer->SetUploadBuffer(eBufferType::PassCB, &pc, 1);
}

void CMainLoop::SetAppPause(bool pause)
{
	pause ? m_timer->Stop() : m_timer->Start();
}

bool CMainLoop::OnResize(int width, int height)
{
	if (m_iRenderer->IsInitialize() == false) 
		return true;

	ReturnIfFalse(m_iRenderer->OnResize(width, height));
	auto aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	m_camera->SetLens(0.25f * MathHelper::Pi, aspectRatio, 1.0f, 1000.f);

	return true;
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

bool CMainLoop::Run(IRenderer* renderer)
{
	m_timer->Reset();
	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			m_timer->Tick();

			if (m_timer->IsStop())
				Sleep(100);
			
			std::wstring fps = CalculateFrameStats(m_timer.get());
			if (!fps.empty())
			{
				m_windowCaption += fps;
				m_window->SetText(m_windowCaption);
			}
			
			m_keyInputManager->CheckInput();
			m_camera->Update(m_timer->DeltaTime());
			ReturnIfFalse(m_iRenderer->PrepareFrame());

			m_material->MakeMaterialBuffer(m_iRenderer);
			UpdateRenderItems();
			UpdateMainPassCB();

			ReturnIfFalse(renderer->Draw(m_AllRenderItems));
		}
	}

	return true;
}