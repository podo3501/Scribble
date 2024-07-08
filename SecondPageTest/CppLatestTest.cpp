#include "pch.h"

#include <vector>
#include<ranges>

//����� ���� ���׵� �ְ�, ������ ��� �Ƕ��� ����� ���� ����.
//����� ũ�Ⱑ Ŀ���� partition���� ������ �ϴµ� �̰� ���� �ҿ����ؼ� ���⿡�� �ñ�����ΰ� ����.

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