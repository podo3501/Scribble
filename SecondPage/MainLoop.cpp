#include "MainLoop.h"
#include <ranges>
#include <algorithm>
#include <sstream>
#include "../Include/RendererDefine.h"
#include "../Include/RenderItem.h"
#include "../Include/interface.h"
#include "../Include/FrameResourceData.h"
#include "./Window.h"
#include "./GameTimer.h"
#include "./Material.h"
#include "./Model.h"
#include "./Camera.h"
#include "./KeyInputManager.h"
#include "./SetupData.h"
#include "./Utility.h"
#include "./Helper.h"
#include "./MockData.h"

using namespace DirectX;

bool g4xMsaaState{ false };
bool CMainLoop::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& lr)
{
	switch (msg)
	{
	case WM_KEYUP:
		if ((int)wParam == VK_F2)
		{
			m_iRenderer->Set4xMsaaState(
				m_window->GetHandle(), m_window->GetWidth(), m_window->GetHeight(), !g4xMsaaState);
			g4xMsaaState = !g4xMsaaState;
		}
		return true;
	}

	return false;
}

RenderItem::RenderItem()
	: NumFramesDirty{ gFrameResourceCount }
{}

CMainLoop::CMainLoop() = default;
CMainLoop::~CMainLoop() = default;

bool CMainLoop::Initialize(const std::wstring& resourcePath, CWindow* window, IRenderer* renderer)
{
	m_window = window;
	m_iRenderer = renderer;

	ReturnIfFalse(InitializeClass(resourcePath));	//초기화

	AddKeyListener();	//키 리스너 등록

	//local에서 작업할 것들
	ReturnIfFalse(OnResize(m_window->GetWidth(), m_window->GetHeight()));
	ReturnIfFalse(LoadMemory());	//데이터를 ram, vram에 올리기

	return true;
}

bool CMainLoop::InitializeClass(const std::wstring& resourcePath)
{
	m_keyInputManager = std::make_unique<CKeyInputManager>(m_window->GetHandle());
	m_camera = std::make_unique<CCamera>();
	m_timer = std::make_unique<CGameTimer>();
	m_material = std::make_unique<CMaterial>();
	m_setupData = std::make_unique<CSetupData>();
	m_model = std::make_unique<CModel>(resourcePath);

	MakeMockData(m_setupData.get(), m_material.get());

	return true;
}

void CMainLoop::AddKeyListener()
{
	m_window->AddWndProcListener([this](HWND wnd, UINT msg, WPARAM wp, LPARAM lp, LRESULT& lr)->bool {
		return MsgProc(wnd, msg, wp, lp, lr); });
	m_window->AddWndProcListener([&keyMng = m_keyInputManager](HWND wnd, UINT msg, WPARAM wp, LPARAM lp, LRESULT& lr)->bool {
		return keyMng->MsgProc(wnd, msg, wp, lp, lr); });
	m_window->AddOnResizeListener([this](int width, int height)->bool {
		return OnResize(width, height); });
	m_window->AddAppPauseListener([this](bool pause)->void {
		return SetAppPause(pause); });

	m_keyInputManager->AddKeyListener([&cam = m_camera](std::vector<int> keyList) {
		cam->PressedKey(keyList); });
	m_keyInputManager->AddMouseListener([&cam = m_camera](float dx, float dy) {
		cam->Move(dx, dy);	 });
}

bool CMainLoop::LoadMemory()
{
	ReturnIfFalse(m_material->LoadTextureIntoVRAM(m_iRenderer));
	ReturnIfFalse(m_setupData->LoadModel(m_model.get(), &m_AllRenderItems));
	ReturnIfFalse(m_model->LoadModelIntoVRAM(m_iRenderer, &m_AllRenderItems));

	return true;
}

void CMainLoop::UpdateRenderItems()
{
	//처리 안할것을 먼저 골라낸다.
	InstanceDataList totalVisibleInstance{};
	int instanceStartIndex{ 0 };
	for (auto& e : m_AllRenderItems)
	{
		InstanceDataList visibleInstance{};
		auto renderItem = e.second.get();
		renderItem->startIndexInstance = instanceStartIndex;
		//보여지는 서브 아이템을 찾아낸다.
		m_camera->FindVisibleSubRenderItems(renderItem->subRenderItems, visibleInstance);
		instanceStartIndex += static_cast<int>(visibleInstance.size());

		std::ranges::move(visibleInstance, std::back_inserter(totalVisibleInstance));
	}
	UpdateInstanceBuffer(totalVisibleInstance);
}

void CMainLoop::UpdateInstanceBuffer(const InstanceDataList& visibleInstance)
{
	std::vector<InstanceBuffer> instanceBufferDatas{};
	std::ranges::transform(visibleInstance, std::back_inserter(instanceBufferDatas), [this](auto& visibleData) {
		InstanceBuffer curInsBuf{};
		XMStoreFloat4x4(&curInsBuf.world, XMMatrixTranspose(visibleData->world));
		XMStoreFloat4x4(&curInsBuf.texTransform, XMMatrixTranspose(visibleData->texTransform));
		curInsBuf.materialIndex = m_material->GetMaterialIndex(visibleData->matName);
		return std::move(curInsBuf); });

	m_iRenderer->SetUploadBuffer(eBufferType::Instance, instanceBufferDatas.data(), instanceBufferDatas.size());
}

void CMainLoop::UpdateMainPassCB()
{	
	PassConstants pc;
	m_camera->GetPassCB(&pc);
	m_timer->GetPassCB(&pc);
	m_setupData->GetPassCB(&pc);

	float width = (float)m_window->GetWidth();
	float height = (float)m_window->GetHeight();
	pc.renderTargetSize = { width, height };
	pc.invRenderTargetSize = { 1.0f / width, 1.0f / height };

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
	m_camera->OnResize(width, height);

	return true;
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
				SubRenderItem* renderItem = GetSubRenderItem(m_AllRenderItems, "things", "skull");
				std::wstring caption = SetWindowCaption(renderItem->instanceCount, renderItem->instanceDataList.size());
				m_window->SetText(caption + fps);
			}
			
			m_keyInputManager->CheckInput();
			ReturnIfFalse(m_iRenderer->PrepareFrame());

			m_camera->Update(m_timer->DeltaTime());
			m_material->MakeMaterialBuffer(m_iRenderer);
			UpdateRenderItems();
			UpdateMainPassCB();

			ReturnIfFalse(renderer->Draw(m_AllRenderItems));
		}
	}

	return true;
}