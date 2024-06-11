#include "pch.h"
#include "../FirstPage/UserInterface.h"
#include "../FirstPage/FrameResource.h"
#include "../../Scribble/Common/Camera.h"
#include "../../Scribble/Common/Interface.h"
#include <functional>

TEST(MainTest, Initialize)
{
	HINSTANCE hInstance = GetModuleHandle(nullptr);
	InstancingAndCullingApp theApp(hInstance);
	auto result = theApp.Initialize(nullptr);
	EXPECT_EQ(result, true);

	theApp.Run();
}

void lambdaTest(int a, int b, std::function<void(int, int)> arg)
{
	arg(a, b);
}

class TestRenderer
{
public:
	virtual void Excute(int a, int b) = 0;
};

class GTestRenderer : public TestRenderer
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

	void Draw(TestRenderer& renderer)
	{
		renderer.Excute(_a, _b);
	}

private:
	int _a = 0;
	int _b = 0;
};

TEST(RendererTest, Excute)
{
	HINSTANCE hInstance = GetModuleHandle(nullptr);
	InstancingAndCullingApp userInterface(hInstance);
	
	/*lambdaTest(10, 20, [](int a, int b) {
		EXPECT_EQ(a, 10);
		EXPECT_EQ(b, 20);
		});*/

	UITest uiTest(10, 20);

	GTestRenderer testUI;
	uiTest.Draw(testUI);
}

TEST(UserInterfaceTest, model)
{
	HINSTANCE hInstance = GetModuleHandle(nullptr);
	MainLoop mainLoop(hInstance);
	mainLoop.Run();
}

class KeyInput
{
public:
	KeyInput();
	void PressedKeyList(std::function<std::vector<int>()> keyList)
	{
		m_keyList = keyList();
	}

	void AddListener(KeyInputListener* keyListener)
	{
		m_listenerList.emplace_back(keyListener);
	}

	void Flush()
	{
		if (m_keyList.empty() == true)
			return;

		for (auto listener : m_listenerList)
		{
			listener->PressedKey(m_keyList);
		}

		m_keyList.clear();
	}

private:
	std::vector<int> m_keyList;
	std::vector<KeyInputListener*> m_listenerList;
};

KeyInput::KeyInput()
	: m_keyList()
{}

TEST(CameraTest, listener)
{
	KeyInput keyInput;
	Camera camera;
	keyInput.AddListener(&camera);
	keyInput.PressedKeyList([]() {
		return std::vector<int>{'W'};
		});
	keyInput.Flush();
}