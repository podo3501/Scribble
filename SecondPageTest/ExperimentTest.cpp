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

namespace STDTest
{
	TEST(STD, all_of)
	{
		std::vector<int> a = { 1, 2, 3, 4, 5 };
		std::vector<int> b{};
		bool result = std::all_of(a.begin(), a.end(), [&b](auto& iter) {
			if (iter == 3)
				return false;
			b.emplace_back(iter);
			return true;
			});

		EXPECT_EQ(static_cast<int>(b.size()), 2);
		EXPECT_EQ(result, false);

		std::vector<int> vecTest{};
		vecTest.resize(3);
		vecTest[2] = 1;

		std::map<int, int> intMap;
		for_each(intMap.begin(), intMap.end(), [](auto& iter) {
			int test = iter.second;
			});
	}
}
