#pragma once

#include "Mantis.h"

#ifdef MANTIS_WINDOWS

namespace Mantis
{
    using WString = eastl::wstring;

    /// <summary>
    /// Takes a UTF-8 character string and encodes it as wide characters.
    /// </summary>
    /// <param name="str">A UTF-8 encoded string.</param>
    /// <returns>A wide character string.</returns>
    WString StringToWideChar(const String str);

    /// <summary>
    /// Takes a wide character string and encodes it as UTF-8.
    /// </summary>
    /// <param name="wStr">A null terminated wide character string.</param>
    /// <returns>A UTF-8 encoded string.</returns>
    String WideCharToString(const wchar_t* wStr);

    /// <summary>
    /// Gets the formatted message for the last windows error.
    /// </summary>
    String GetLastWindowsError();
}

#endif // MANTIS_WINDOWS
