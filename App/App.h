#pragma once

#ifdef _DEBUG
#pragma comment(lib, "../Lib/Core_d.lib")
#pragma comment(lib, "../Lib/SecondPage_d.lib")
#else
#pragma comment(lib, "../Lib/Core.lib")
#pragma comment(lib, "../Lib/SecondPage.lib")
#endif