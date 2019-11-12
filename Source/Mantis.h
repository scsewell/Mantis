#pragma once

// include precompiled header
#include "stdafx.h"

// include core stl
#include <stdint.h>
#include <errno.h>
#include <mutex>
#include <thread>

// include core eastl
#include <EASTL/string.h>
#include <EASTL/array.h>
#include <EASTL/vector.h>
#include <EASTL/queue.h>
#include <EASTL/stack.h>
#include <EASTL/map.h>
#include <EASTL/unordered_set.h>
#include <EASTL/unordered_map.h>
#include <EASTL/vector_multimap.h>
#include <EASTL/algorithm.h>
#include <EASTL/functional.h>
#include <EASTL/sort.h>
#include <EASTL/optional.h>
#include <EASTL/memory.h>
#include <EASTL/unique_ptr.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/weak_ptr.h>


// macros for macro highjinks
#define MANTIS_TOKEN_AS_TEXT(token) #token
#define MANTIS_TOKEN_VALUE_AS_TEXT(token) MANTIS_TOKEN_AS_TEXT(token)

// declares string as UTF-8 encoded
#define MANTIS_TEXT(str) u8##str

namespace Mantis
{
    using String = eastl::u8string;
}

// marcros for project properties
#ifndef MANTIS_COMPANY_NAME
#   define MANTIS_COMPANY_NAME MANTIS_TEXT("Company")
#endif
#ifndef MANTIS_PROJECT_NAME
#   define MANTIS_PROJECT_NAME MANTIS_TEXT("Project")
#endif

#ifndef MANTIS_VERSION_MAJOR
#   define MANTIS_VERSION_MAJOR 1
#endif
#ifndef MANTIS_VERSION_MINOR
#   define MANTIS_VERSION_MINOR 0
#endif
#ifndef MANTIS_VERSION_PATCH
#   define MANTIS_VERSION_PATCH 0
#endif

#define MANTIS_VERSION_TEXT (MANTIS_TOKEN_VALUE_AS_TEXT(MANTIS_VERSION_MAJOR) "." MANTIS_TOKEN_VALUE_AS_TEXT(MANTIS_VERSION_MINOR) "." MANTIS_TOKEN_VALUE_AS_TEXT(MANTIS_VERSION_PATCH))

// include often used functionality
#include "Utils/EnumFlags.h"
#include "Utils/NonCopyable.h"
#include "Utils/Delegate.h"
#include "Utils/Timer.h"

#include "Utils/Logging/Logger.h"

#include "Utils/Geometry/Vector2.h"
#include "Utils/Geometry/Vector2Int.h"
#include "Utils/Geometry/Color.h"
#include "Utils/Geometry/Rect.h"
#include "Utils/Geometry/RectInt.h"
