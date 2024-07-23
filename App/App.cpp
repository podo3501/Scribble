#include "App.h"
#include <crtdbg.h>
#include <memory>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include "../Include/Interface.h"
#include "../SecondPage/Window.h"
#include "../SecondPage/MainLoop.h"
#include "../SecondPage/Utility.h"
#include "../SecondPage/MockData.h"

void ReportLiveObjects();

int WINAPI WinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		std::wstring resPath = L"../Resource/";

		std::unique_ptr<CWindow> window = std::make_unique<CWindow>(hInstance);
		window->Initialize(true);
		auto renderer = CreateRenderer(
			resPath, 
			window->GetHandle(), 
			window->GetWidth(), 
			window->GetHeight(),
			GetShaderFileList());
		assert(renderer);
		
		bool bResult{ true };
		std::unique_ptr<CMainLoop> mainLoop = std::make_unique<CMainLoop>();
		bResult = mainLoop->Initialize(resPath, window.get(), renderer.get());
		bResult = mainLoop->Run(renderer.get());
	}
	catch (CException e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}

#if defined(DEBUG) | defined(_DEBUG)
	ReportLiveObjects();
#endif

	//�� ���ϰ��� msg���ϰ����� �ٲ�� �Ѵ�. ���� �� �ʿ�� ������.
	return 0;
}

void ReportLiveObjects()
{
	HMODULE dxgidebugdll = GetModuleHandleW(L"dxgidebug.dll");
	decltype(&DXGIGetDebugInterface) GetDebugInterface = reinterpret_cast<decltype(&DXGIGetDebugInterface)>(GetProcAddress(dxgidebugdll, "DXGIGetDebugInterface"));

	IDXGIDebug* debug;
	GetDebugInterface(IID_PPV_ARGS(&debug));

	OutputDebugStringW(L"---------------Starting Live Direct3D Object Dump:----------------\r\n");
	debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	OutputDebugStringW(L"---------------Completed Live Direct3D Object Dump----------------\r\n");

	debug->Release();
}