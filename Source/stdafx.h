#pragma once

// include platform defines
#include "Platform.h"

// configure windows
#ifdef MANTIS_WINDOWS
#   define NTDDI_VERSION 0x06010000    // target windows 7
#   define WINVER 0x0601
#   define _WIN32_WINNT 0x0601
#   define WIN32_LEAN_AND_MEAN         // exclude rarely-used stuff from Windows headers
#   include <Windows.h>
#endif

// configure the windowing library
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// configure EASTL library
#define _SILENCE_CXX17_NEGATORS_DEPRECATION_WARNING
#define EASTL_USER_CONFIG_HEADER "EASTLConfig.h"

#include <EABase/eabase.h>

// configure the math libary
#define EIGEN_UNALIGNED_VECTORIZE 1
#define EIGEN_FAST_MATH  1
#include <Eigen/Core>
#include <Eigen/Geometry>
