#ifndef NEPOMUK_TOOL_IO_STREAM_TRAITS_HPP_
#define NEPOMUK_TOOL_IO_STREAM_TRAITS_HPP_

#include <string>
#include <type_traits>

namespace nepomuk
{
namespace tool
{
namespace io
{

// Compile time stream concept checks.
// Usage example: static_assert(IsInputStream<T>(), "");

template <typename T, typename = void> struct IsLineInputStream : std::false_type
{
};

template <typename T>
struct IsLineInputStream<T,
                         decltype((void)std::declval<T>().getline(std::declval<std::string &>()))>
    : std::true_type
{
};

template <typename T, typename = void> struct IsLineOutputStream : std::false_type
{
};

template <typename T>
struct IsLineOutputStream<T, decltype((void)std::declval<T>().putline(""))> : std::true_type
{
};

template <typename T>
struct IsLineInputOutputStream
    : std::integral_constant<bool, IsLineInputStream<T>() && IsLineOutputStream<T>()>
{
};

template <typename T, typename = void> struct IsInputStream : std::false_type
{
};

template <typename T>
struct IsInputStream<T, decltype((void)std::declval<T>().read_bytes(nullptr, 0))> : std::true_type
{
};

template <typename T, typename = void> struct IsOutputStream : std::false_type
{
};

template <typename T>
struct IsOutputStream<T, decltype((void)std::declval<T>().write_bytes(nullptr, 0))> : std::true_type
{
};

template <typename T>
struct IsInputOutputStream : std::integral_constant<bool, IsInputStream<T>() && IsOutputStream<T>()>
{
};

} // namespace io
} // namespace tool
} // namespace nepomuk

#endif // NEPOMUK_TOOL_IO_STREAM_TRAITS_HPP_
