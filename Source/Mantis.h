#pragma once

#include <EASTL/string.h>

// declares string as UTF-8 encoded
#define STRING(str) u8##str
typedef eastl::u8string String;

// include often used functionality
#include "Utils/Logging/Logger.h"
