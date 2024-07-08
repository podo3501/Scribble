#include "pch.h"

#include <vector>
#include<ranges>

//모듈은 아직 버그도 있고, 지원도 없어서 맨땅에 헤딩할 일이 많다.
//모듈의 크기가 커지며 partition으로 나눠야 하는데 이게 아직 불완전해서 쓰기에는 시기상조인거 같다.

namespace Z_CPPLatest
{
	TEST(modules, classTest)
	{
		//std::unique_ptr<CModuleTest> test = std::make_unique<CModuleTest>();
		//EXPECT_EQ(test->IsTrue(), true);
	}
	TEST(modules, stdTest)
	{
		std::vector<int> modulesVector{ 1, 2, 3, 4 };
		EXPECT_EQ(modulesVector.size(), 4);

		auto find = std::ranges::find(modulesVector, 3);
		EXPECT_EQ(find != modulesVector.end(), true);

		auto filter = modulesVector | std::ranges::views::filter([](auto& n) {
			return n % 2 == 0; });

		std::ranges::for_each(filter, [](auto& n) {
			EXPECT_EQ(n % 2, 0); });
	}
}