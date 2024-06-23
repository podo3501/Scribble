#include "pch.h"

namespace Experiment
{
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
		UITest uiTest(10, 20);

		GTestRenderer testUI;
		uiTest.Draw(testUI);
	}
}
