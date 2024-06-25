#pragma once

template <class _Tp>
_Tp& RvToLv(_Tp&& __value)
{
	return __value;
}

constexpr int WrapAround(int x, int low, int high)
{
	assert(low < high);
	const int n = (x - low) % (high - low);
	return (n >= 0) ? (n + low) : (n + high);
}