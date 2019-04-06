#pragma once

#include "Mantis.h"

namespace Mantis
{
    /// <summary>
    /// Describes the location of a relative path.
    /// </summary>
    enum struct PathRoot
    {
        ExeDir,
        LogDir,
        ConfigDir,
    };

    /// <summary>
    /// Manages access to the filesystem.
    /// </summary>
    class Filesystem
    {
    public:
        /// <summary>
        /// The path separator character.
        /// </summary>
        static const char Separator;

        static String GetExePath();

    private:
#ifdef MANTIS_WINDOWS
        /// <summary>
        /// Prints the last windows error.
        /// </summary>
        static void PrintLastError();

        /// <summary>
        /// Takes a wide character string and encodes it as UTF-8.
        /// </summary>
        /// <param name="wStr">A null terminated wide character string.</param>
        /// <returns>A UTF-8 encoded string.</returns>
        static String WideCharToString(const wchar_t* wStr);
#endif
    };
}
