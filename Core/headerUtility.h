//����� ��Ŭ��� ��ų �� �ֵ��� ������ ���� ���� �ʵ��� �Ѵ�.
#pragma once

template <typename E>
constexpr auto
EtoV(E enumerator) noexcept
{
	return static_cast<std::underlying_type_t<E>>(enumerator);
}

