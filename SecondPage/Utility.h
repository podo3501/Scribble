#pragma once

class CException
{
public:
    CException() = default;
    CException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

    std::wstring ToString()const;

    HRESULT ErrorCode = S_OK;
    std::wstring FunctionName;
    std::wstring Filename;
    int LineNumber = -1;
};

constexpr int WrapAround(int x, int low, int high)
{
    assert(low < high);
    const int n = (x - low) % (high - low);
    return (n >= 0) ? (n + low) : (n + high);
}

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw CoreException(hr__, L#x, wfn, __LINE__); } \
}
#endif

#ifndef ReturnIfFailed
#define ReturnIfFailed(x)                                              \
{                                                                     \
    if(FAILED(x))                                            \
        return false;                                                \
}
#endif

#ifndef ReturnIfFalse
#define ReturnIfFalse(x)                       \
{                                                             \
    if(!x) return false;                          \
}
#endif

template <typename E>
constexpr auto
EtoV(E enumerator) noexcept
{
    return static_cast<std::underlying_type_t<E>>(enumerator);
}

template <class _Tp>
_Tp& RvToLv(_Tp&& __value)
{
    return __value;
}
