#pragma once

template <typename E>
constexpr auto
toUType(E enumerator) noexcept
{
	return static_cast<std::underlying_type_t<E>>(enumerator);
}

template <class _Tp>
_Tp& RvToLv(_Tp&& __value)
{
	return __value;
}