#pragma once

// Get platform
#if defined(_WIN32) || defined(_WIN64)
#   define MANTIS_WINDOWS
#endif
#if defined(__linux__)
#   define MANTIS_LINUX
#endif
#if defined(__APPLE__)
#   define MANTIS_MACOS
#endif

// Get compiler
#if defined(_MSC_VER)
#   define MANTIS_MSCV
#endif
#if defined(__GNUC__)
#   define MANTIS_GCC
#endif
#if defined(__clang__)
#   define MANTIS_CLANG
#endif

// Get architecture
#if defined(_M_IX86) || defined(__i386__)
#   define MANTIS_32
#endif
#if defined(_M_X64) || defined(__x86_64__)
#   define MANTIS_64
#endif

// Get configuration
#if !defined(NDEBUG)
#   define MANTIS_DEBUG
#endif

// Recommend that the compiler inline this function.
#define MANTIS_INLINE inline

// Strongly recommend that the compiler inline this function.
#if defined(MANTIS_MSCV)
#   define MANTIS_FORCE_INLINE __forceinline
#elif defined(MANTIS_GCC)
#   define MANTIS_FORCE_INLINE attribute((always_inline))
#else
#   define MANTIS_FORCE_INLINE
#endif

// Alignment marco
#if defined(MANTIS_MSCV)
#   define MANTIS_ALIGN(x) __declspec(align(x))
#elif defined(MANTIS_GCC)
#   define MANTIS_ALIGN(x) __attribute__(align(x))
#else
#   define MANTIS_ALIGN(x) alignas(x)
#endif

// restricted pointer macro
#if defined(MANTIS_MSCV)
#   define MANTIS_RESTRICT __restrict
#elif defined(MANTIS_GCC)
#   define MANTIS_RESTRICT __restrict
#elif defined(MANTIS_CLANG)
#   define MANTIS_RESTRICT __restrict
#else
#   define MANTIS_RESTRICT
#endif