#include "pch.h"
#include "../FirstPage/UserInterface.h"

TEST(TestCaseName, TestName) 
{
  EXPECT_EQ(1, 1);
  EXPECT_TRUE(true);
}

TEST(MainTest, Initialize)
{
	TCHAR programpath[_MAX_PATH];
	GetCurrentDirectory(_MAX_PATH, programpath);

	HINSTANCE hInstance = GetModuleHandle(nullptr);
	InstancingAndCullingApp theApp(hInstance);
	auto result = theApp.Initialize();
	EXPECT_EQ(result, true);
}