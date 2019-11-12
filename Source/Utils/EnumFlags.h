#pragma once

#include <type_traits>
#include "Platform.h"

/// <summary>
/// Allows an enum to use bitwise operators.
/// </summary>
#define ENUM_IS_FLAGS(enumType)             \
    template<>                              \
    struct EnableBitMaskOperators<enumType> \
    {                                       \
        static const bool enable = true;    \
    };

#define HAS_FLAGS(flags, value) ((flags & (value)) == (value))
#define HAS_ANY_FLAG(flags, value) ((flags & (value)) != 0)
#define HAS_NO_FLAG(flags, value) ((flags & (value)) == 0)

template<typename TEnum>
struct EnableBitMaskOperators
{
    static const bool enable = false;
};

template<typename TEnum>
typename std::enable_if<EnableBitMaskOperators<TEnum>::enable, TEnum>::type
constexpr MANTIS_FORCE_INLINE operator |(TEnum lhs, TEnum rhs)
{
    using underlying = typename std::underlying_type<TEnum>::type;
    return static_cast<TEnum>(
        static_cast<underlying>(lhs) |
        static_cast<underlying>(rhs)
    );
}

template<typename TEnum>
typename std::enable_if<EnableBitMaskOperators<TEnum>::enable, TEnum>::type
constexpr MANTIS_FORCE_INLINE operator &(TEnum lhs, TEnum rhs)
{
    using underlying = typename std::underlying_type<TEnum>::type;
    return static_cast<TEnum>(
        static_cast<underlying>(lhs) &
        static_cast<underlying>(rhs)
    );
}

template<typename TEnum>
typename std::enable_if<EnableBitMaskOperators<TEnum>::enable, TEnum>::type
constexpr MANTIS_FORCE_INLINE operator ^(TEnum lhs, TEnum rhs)
{
    using underlying = typename std::underlying_type<TEnum>::type;
    return static_cast<TEnum>(
        static_cast<underlying>(lhs) ^
        static_cast<underlying>(rhs)
    );
}

template<typename TEnum>
typename std::enable_if<EnableBitMaskOperators<TEnum>::enable, TEnum>::type
constexpr MANTIS_FORCE_INLINE operator ~(TEnum lhs)
{
    using underlying = typename std::underlying_type<TEnum>::type;
    return static_cast<TEnum>(
        ~static_cast<underlying>(lhs)
    );
}

template<typename TEnum>
typename std::enable_if<EnableBitMaskOperators<TEnum>::enable, TEnum>::type
constexpr MANTIS_FORCE_INLINE &operator |=(TEnum &lhs, TEnum rhs)
{
    using underlying = typename std::underlying_type<TEnum>::type;
    lhs = static_cast<TEnum>(
        static_cast<underlying>(lhs) |
        static_cast<underlying>(rhs)
    );
    return lhs;
}

template<typename TEnum>
typename std::enable_if<EnableBitMaskOperators<TEnum>::enable, TEnum>::type
constexpr MANTIS_FORCE_INLINE&operator &=(TEnum &lhs, TEnum rhs)
{
    using underlying = typename std::underlying_type<TEnum>::type;
    lhs = static_cast<TEnum>(
        static_cast<underlying>(lhs) &
        static_cast<underlying>(rhs)
    );
    return lhs;
}

template<typename TEnum>
typename std::enable_if<EnableBitMaskOperators<TEnum>::enable, TEnum>::type
constexpr MANTIS_FORCE_INLINE&operator ^=(TEnum &lhs, TEnum rhs)
{
    using underlying = typename std::underlying_type<TEnum>::type;
    lhs = static_cast<TEnum>(
        static_cast<underlying>(lhs) ^
        static_cast<underlying>(rhs)
    );
    return lhs;
}
