#include "pch.h"
#include "../FirstPage/UserInterface.h"
#include <functional>

TEST(MainTest, Initialize)
{
	HINSTANCE hInstance = GetModuleHandle(nullptr);
	InstancingAndCullingApp theApp(hInstance);
	auto result = theApp.Initialize();
	EXPECT_EQ(result, true);
}

void lambdaTest(int a, int b, std::function<void(int, int)> arg)
{
	arg(a, b);
}

class Renderer
{
public:
	virtual void Excute(int a, int b) = 0;
};

class ConsoleRenderer : public Renderer
{
public:
	void Excute(int a, int b) override
	{
		EXPECT_EQ(a, 10);
		EXPECT_EQ(b, 20);
	}
};

class UITest
{
public:
	UITest(int a, int b)
		: _a(a), _b(b)
	{}

	void Draw(Renderer& renderer)
	{
		renderer.Excute(_a, _b);
	}

private:
	int _a = 0;
	int _b = 0;
};

TEST(UserInterfaceTest, Excute)
{
	HINSTANCE hInstance = GetModuleHandle(nullptr);
	InstancingAndCullingApp userInterface(hInstance);
	
	/*lambdaTest(10, 20, [](int a, int b) {
		EXPECT_EQ(a, 10);
		EXPECT_EQ(b, 20);
		});*/

	UITest uiTest(10, 20);

	ConsoleRenderer consoleUI;
	uiTest.Draw(consoleUI);
}