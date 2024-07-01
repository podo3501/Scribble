#include "pch.h"
#include <algorithm>
#include <ranges>
#include <numeric>

namespace Experiment
{
	class TestRenderer
	{
	public:
		virtual int Load() = 0;
		virtual void Excute(int a) = 0;
	};

	class CRenderer : public TestRenderer
	{
		int Load() override
		{
			return 10;
		}

		void Excute(int a) override
		{
			//a를 화면에 뿌린다.
			return;
		}
	};

	class GTestRenderer : public TestRenderer
	{
	public:
		virtual int Load() override
		{
			return 0;
		}

		void Excute(int a) override
		{
			EXPECT_EQ(a, 10);
		}
	};

	class UITest
	{
	public:
		UITest(int a)
			: _a(a)
		{}

		void Load(TestRenderer& renderer)
		{
			_a = renderer.Load();
		}

		void Draw(TestRenderer& renderer)
		{
			renderer.Excute(_a);
		}

	private:
		int _a = 0;
	};

	TEST(RendererTest, Excute)
	{
		UITest uiTest(5);
		//자료는 CRenderer가 다 처리하고
		CRenderer UI;
		uiTest.Load(UI);

		//Draw 함수 테스트는 가짜 GTestRenderer가 가로채서 테스트한다.
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

	TEST(STD20, ranges)
	{
		std::vector<int> items = { 0, 7, 8, 9, 10 };
		auto find = std::ranges::find_if(items, [](int i) { return i == 8; });
		auto idx = std::distance(items.begin(), find);
		EXPECT_EQ(idx, 2);
	}

	TEST(STD20, views)
	{
		std::map<int, int> mItems = { {1, 1}, {2, 2} };
		auto tmItems = std::views::all(mItems) | std::views::filter([](auto& iter) { return iter.first == 1; });

		//filter는 참인것만 담는다.
		std::vector<int> vItems = { 1, 2, 3, 4, 5, 6 };
		auto tvItems = vItems | std::ranges::views::filter([](auto n) 
			{ 
				return n % 2 == 0; 
			});

		std::vector<int> v = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		auto rng = std::ranges::views::all(v)
			| std::ranges::views::filter([](int a) {
			return a % 2 == 0; }); // 짝수만 필터링

		auto view{ std::views::transform(v, [](int i) { return i * i; }) };
		auto sumsq {std::accumulate(view.begin(), view.end(), 0)};
	}
}
