//헤더에 인클루드 시킬 수 있도록 변경을 자주 하지 않도록 한다.
#pragma once

template <typename E>
constexpr auto
EtoV(E enumerator) noexcept
{
	return static_cast<std::underlying_type_t<E>>(enumerator);
}

