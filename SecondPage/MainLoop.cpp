#include "MainLoop.h"
#include <sstream>
#include "../Include/RendererDefine.h"
#include "../Include/RenderItem.h"
#include "../Include/interface.h"
#include "../Include/FrameResourceData.h"
#include "../Include/Types.h"
#include "./Window.h"
#include "./GameTimer.h"
#include "./Model.h"
#include "./Camera.h"
#include "./KeyInput.h"
#include "./Shadow.h"
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

CMainLoop::CMainLoop()
	: m_keyInput{ nullptr }
	, m_camera{ nullptr }
	, m_timer{ nullptr }
	, m_shadow{ nullptr }
	, m_model{ nullptr }
	, m_AllRenderItems{}
{}
CMainLoop::~CMainLoop() = default;

bool CMainLoop::Initialize(const std::wstring& resourcePath, CWindow* window, IRenderer* renderer)
{
	m_window = window;
	m_iRenderer = renderer;

	ReturnIfFalse(InitializeClass(resourcePath));	//초기화

	AddKeyListener();	//키 리스너 등록

	//local에서 작업할 것들
	ReturnIfFalse(OnResize(m_window->GetWidth(), m_window->GetHeight()));
	ReturnIfFalse(m_model->LoadMemory(m_iRenderer, m_AllRenderItems));	//데이터를 ram, vram에 올리기

	return true;
}

bool CMainLoop::InitializeClass(const std::wstring& resourcePath)
{
	m_keyInput = std::make_unique<CKeyInput>(m_window->GetHandle());
	m_camera = std::make_unique<CCamera>();
	m_timer = std::make_unique<CGameTimer>();
	m_shadow = std::make_unique<CShadow>();
	m_model = std::make_unique<CModel>();

	ReturnIfFalse(m_model->Initialize(resourcePath, MakeMockData()));

	return true;
}

void CMainLoop::AddKeyListener()
{
	m_window->AddWndProcListener([this](HWND wnd, UINT msg, WPARAM wp, LPARAM lp, LRESULT& lr)->bool {
		return MsgProc(wnd, msg, wp, lp, lr); });
	m_window->AddWndProcListener([&keyInput = m_keyInput](HWND wnd, UINT msg, WPARAM wp, LPARAM lp, LRESULT& lr)->bool {
		return keyInput->MsgProc(wnd, msg, wp, lp, lr); });
	m_window->AddOnResizeListener([this](int width, int height)->bool {
		return OnResize(width, height); });
	m_window->AddAppPauseListener([this](bool pause)->void {
		return SetAppPause(pause); });

	m_keyInput->AddKeyListener([&cam = m_camera](std::vector<int> keyList) {
		cam->PressedKey(keyList); });
	m_keyInput->AddMouseListener([&cam = m_camera](float dx, float dy) {
		cam->Move(dx, dy);	 });
}

void CMainLoop::UpdatePassCB()
{	
	std::vector<PassConstants> passCBList{};
	passCBList.emplace_back(UpdateMainPassCB());
	passCBList.emplace_back(m_shadow->UpdatePassCB());

	m_iRenderer->SetUploadBuffer(eBufferType::PassCB, passCBList.data(), passCBList.size());
}

PassConstants CMainLoop::UpdateMainPassCB()
{
	PassConstants pc;
	m_camera->GetPassCB(&pc);
	m_timer->GetPassCB(&pc);
	m_shadow->GetPassCB(&pc);

	float width = (float)m_window->GetWidth();
	float height = (float)m_window->GetHeight();
	pc.renderTargetSize = { width, height };
	pc.invRenderTargetSize = { 1.0f / width, 1.0f / height };

	return pc;
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
				SubRenderItem* renderItem = GetSubRenderItem(m_AllRenderItems, GraphicsPSO::Opaque, "skull");
				if (renderItem != nullptr)
				{
					std::wstring caption = SetWindowCaption(renderItem->instanceCount, renderItem->instanceDataList.size());
					m_window->SetText(caption + fps);
				}
			}
			
			m_keyInput->CheckInput();
			ReturnIfFalse(m_iRenderer->PrepareFrame());

			m_camera->Update(m_timer->DeltaTime());
			m_model->Update(m_iRenderer, m_camera.get(), m_AllRenderItems);
			m_shadow->Update(m_timer->DeltaTime());
			
			UpdatePassCB();

			ReturnIfFalse(renderer->Draw(m_AllRenderItems));
		}
	}

	return true;
}