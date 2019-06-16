#pragma once

#include "Mantis.h"
#include "FileStream.h"

namespace Mantis
{
    /// <summary>
    /// Describes the location of a relative path.
    /// </summary>
    enum struct PathRoot
    {
        ExeDir,
        ConfigDir,
        OutputDir,
    };

    /// <summary>
    /// The different ways a file can be accessed.
    /// </summary>
    enum struct FileMode
    {
        /// <summary>
        /// Open the file for reading. Fails if the file does not exist or cannot be found.
        /// </summary>
        Read,
        /// <summary>
        /// Opens an empty file for writing. If the file exists, its contents are destroyed.
        /// </summary>
        Overwrite,
        /// <summary>
        /// Opens for writing at the end of the file without removing the EOF marker before new data is written to the file. Creates the file if it does not exist.
        /// </summary>
        Append,
        /// <summary>
        /// Open the file for reading and writing. Fails if the file does not exist or cannot be found.
        /// </summary>
        ReadWrite,
        /// <summary>
        /// Opens an empty file for both reading and writing. If the file exists, its contents are destroyed.
        /// </summary>
        ReadOverwrite,
        /// <summary>
        /// Opens for reading and appending. The appending operation includes the removal of the EOF marker before new data is written to the file. The EOF marker is not restored after writing is completed. Creates the file if it does not exist.
        /// </summary>
        ReadAppend,
    };

    /// <summary>
    /// Manages access to the filesystem.
    /// </summary>
    class Filesystem
    {
    public:
        /// <summary>
        /// Combines paths using the path separator.
        /// </summary>
        /// <param name="path1">The first path.</param>
        /// <param name="path2">The second path.</param>
        /// <returns>The joined path.</returns>
        static String JoinPaths(const String& path1, const String& path2);

        /// <summary>
        /// Combines paths using the path separator.
        /// </summary>
        /// <param name="paths">The paths to join.</param>
        /// <returns>The joined path.</returns>
        static String JoinPaths(const eastl::vector<String>& paths);

        /// <summary>
        /// Checks if a file exists at the given path.
        /// </summary>
        /// <param name="root">The folder to get the absolute path for.</param>
        /// <param name="path">The relative file path under the folder root.</param>
        /// <returns>True if there is a file at the given path.</returns>
        static bool Exists(PathRoot root, String path);

        /// <summary>
        /// Opens a file stream.
        /// </summary>
        /// <param name="root">The folder to get the absolute path for.</param>
        /// <param name="path">The relative file path under the folder root.</param>
        /// <param name="mode">Describes how the file should be opened.</param>
        /// <returns>A stream to a file at the supplied path.</returns>
        static eastl::shared_ptr<FileStream> Open(PathRoot root, String path, FileMode mode);

    private:
        /// <summary>
        /// Gets the path to the item as a UTF-8 string.
        /// </summary>
        /// <param name="root">The folder to get the absolute path for.</param>
        /// <param name="path">The relative file path under the folder root.</param>
        static String GetPath(PathRoot root, String path);

        /// <summary>
        /// Gets the path to a folder from a UTF-8 string.
        /// </summary>
        /// <param name="path">The file path to get the directory of.</param>
        static String GetDirectoryFromPath(String path);

        /// <summary>
        /// Creates a directory on the filesystem, if it does not already exist.
        /// </summary>
        /// <param name="dirPath">The full path of the directory.</param>
        /// <returns>True if the directory was sucessfully created.</returns>
        static bool CreateDir(String dirPath);

#ifdef MANTIS_WINDOWS
        /// <summary>
        /// Get the directory containing the executable.
        /// </summary>
        static String GetExePath();

        /// <summary>
        /// Get the local appdata folder.
        /// </summary>
        static String GetAppDataPath();
#endif
    };
}
