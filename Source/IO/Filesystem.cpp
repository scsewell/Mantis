#include "stdafx.h"
#include "Filesystem.h"
#include "Utils/Platform/WindowsUtils.h"

#ifdef MANTIS_WINDOWS
#include <ShlObj.h>
#endif

#define LOG_TAG MANTIS_TEXT("Filesystem")

namespace Mantis
{
    String Filesystem::JoinPaths(const String& path1, const String& path2)
    {
        String path;
        path.append(path1);
        path.append("/");
        path.append(path2);
        return path;
    }

    String Filesystem::JoinPaths(const eastl::vector<String>& paths)
    {
        String path;
        auto it = paths.begin();
        while (it != paths.end())
        {
            path.append(*it);
            path.append("/");
            ++it;
        }
        path.append(*it);
        return path;
    }

    bool Filesystem::Exists(PathRoot root, String path)
    {
        // get the full path
        String fullPath = GetPath(root, path).c_str();

#ifdef MANTIS_WINDOWS
        DWORD attrib = GetFileAttributesW(StringToWideChar(fullPath).c_str());

        if (attrib != INVALID_FILE_ATTRIBUTES)
        {
            return true;
        }

        // check that the file really doesn't exist
        switch (GetLastError())
        {
            case ERROR_PATH_NOT_FOUND:
            case ERROR_FILE_NOT_FOUND:
            case ERROR_INVALID_NAME:
            case ERROR_BAD_NETPATH:
                return false;
        }

        Logger::ErrorTF(LOG_TAG, "Failed to get file attributes: %s", GetLastWindowsError().c_str());
        return false;
#endif
    }

    eastl::shared_ptr<FileStream> Filesystem::Open(PathRoot root, String path, FileMode mode)
    {
        FILE* handle = nullptr;
        const char* modeStr;

        switch (mode)
        {
#ifdef MANTIS_WINDOWS
            case FileMode::Read:            modeStr = "rb, ccs=UTF-8"; break;
            case FileMode::Overwrite:       modeStr = "wb, ccs=UTF-8"; break;
            case FileMode::Append:          modeStr = "ab, ccs=UTF-8"; break;
            case FileMode::ReadWrite:       modeStr = "r+b, ccs=UTF-8"; break;
            case FileMode::ReadOverwrite:   modeStr = "w+b, ccs=UTF-8"; break;
            case FileMode::ReadAppend:      modeStr = "a+b, ccs=UTF-8"; break;
#endif
            default:
                Logger::ErrorTF(LOG_TAG, "Unsupported file mode: %i", static_cast<std::underlying_type<FileMode>::type>(mode));
                return nullptr;
        }

        // get the full path
        String fullPath = GetPath(root, path).c_str();

        // ensure the directory exists if writing to a file
        switch (mode)
        {
            case FileMode::Overwrite:
            case FileMode::Append:
            case FileMode::ReadOverwrite:
            case FileMode::ReadAppend:
                CreateDir(GetDirectoryFromPath(fullPath));
                break;
        }

        // try to open the file
        handle = fopen(fullPath.c_str(), modeStr);

        // check if successful
        if (handle == nullptr)
        {
            Logger::ErrorTF(LOG_TAG, "Failed to open file \"%s\": %s", fullPath.c_str(), strerror(errno));
            return nullptr;
        }
        
        // return the stream
        return eastl::make_shared<FileStream>(handle, fullPath);
    }

    String Filesystem::GetPath(PathRoot root, String path)
    {
        String fullPath;

        // get the path root
        switch (root)
        {
#ifdef MANTIS_WINDOWS
            case PathRoot::ExeDir:
                fullPath = GetExePath();
                break;
            case PathRoot::ConfigDir:
            case PathRoot::OutputDir:
                fullPath = GetAppDataPath() + String(MANTIS_COMPANY_NAME "/" MANTIS_PROJECT_NAME "/");
                break;
#endif
            default:
                Logger::ErrorTF(LOG_TAG, "Unsupported path root: %i", static_cast<std::underlying_type<PathRoot>::type>(root));
                return String();
        }

        // add on the relative path
        fullPath.append(path);

        // convert the path to a format accepted by the platform
#ifdef MANTIS_WINDOWS
        eastl::replace(fullPath.begin(), fullPath.end(), '/', '\\');
#endif

        return fullPath;
    }

    String Filesystem::GetDirectoryFromPath(String path)
    {
        // get the path to the directory
#ifdef MANTIS_WINDOWS
        const char separator = '\\';
#else
        const char separator = '/';
#endif

        return String(path.c_str(), path.find_last_of(separator, path.size()) + 1);
    }

    bool Filesystem::CreateDir(String dirPath)
    {
#ifdef MANTIS_WINDOWS
        if (SHCreateDirectoryExW(NULL, StringToWideChar(dirPath).c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError())
        {
            return true;
        }
        else
        {
            Logger::ErrorTF(LOG_TAG, "Failed to create directory: %s", GetLastWindowsError().c_str());
            return false;
        }
#endif
    }

#ifdef MANTIS_WINDOWS
    String Filesystem::GetExePath()
    {
        wchar_t wBuf[MAX_PATH];
        DWORD pathLen = GetModuleFileNameW(NULL, wBuf, sizeof(wBuf));

        if (pathLen == 0 || pathLen == sizeof(wBuf))
        {
            Logger::ErrorTF(LOG_TAG, "Failed to get exe path: ", GetLastWindowsError().c_str());
        }

        // exclude the file from path
        for (int i = pathLen - 1; i >= 0; i--)
        {
            if (wBuf[i] == L'\\')
            {
                wBuf[i + 1] = L'\0';
                break;
            }
        }

        // convert to a UTF-8 string
        return WideCharToString(wBuf);
    }

    String Filesystem::GetAppDataPath()
    {
        PWSTR path = nullptr;
        HRESULT result = SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, NULL, &path);
        if (FAILED(result))
        {
            Logger::ErrorTF(LOG_TAG, "Failed to get known folder: %#x", result);
        }

        // get the length of the folder, without the terminator
        int length = lstrlenW(path);

        // copy the path to a larger buffer and free the OS provided buffer
        wchar_t wBuf[MAX_PATH];
        lstrcpynW(wBuf, path, MAX_PATH);
        CoTaskMemFree(path);

        // append a path separator
        wBuf[length] = L'\\';
        wBuf[length + 1] = L'\0';

        // convert to a UTF-8 string
        return WideCharToString(wBuf);
    }
#endif
}
