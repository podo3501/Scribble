#ifdef _DEBUG
#pragma comment(lib, "../Lib/FirstPage_d.lib")
#else
#pragma comment(lib, "../Lib/FirstPage.lib")
#endif

#include "../FirstPage/UserInterface.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		InstancingAndCullingApp theApp(hInstance);
		if (!theApp.Initialize(nullptr))
			return 0;

		return theApp.Run();
	}
	catch (DxException e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}

	return 1;
}