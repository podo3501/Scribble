#pragma once

template <class _Tp>
_Tp& RvToLv(_Tp&& __value)
{
	return __value;
}