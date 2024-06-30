#include "App.h"
#include <crtdbg.h>
#include <memory>
#include "../Core/d3dUtil.h"
#include "../Core/Window.h"
#include "../SecondPage/MainLoop.h"
#include "../SecondPage/Interface.h"

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
		auto renderer = CreateRenderer(resPath, window.get());

		bool bResult{ true };
		std::unique_ptr<CMainLoop> mainLoop = std::make_unique<CMainLoop>(resPath);
		bResult = mainLoop->Initialize(window.get(), renderer.get());
		bResult = mainLoop->Run(renderer.get());
	}
	catch (CoreException e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}

	//이 리턴값을 msg리턴값으로 바꿔야 한다. 딱히 뭐 필요는 없지만.
	return 0;
}