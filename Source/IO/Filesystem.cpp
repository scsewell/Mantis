#include "stdafx.h"
#include "Filesystem.h"

#include <filesystem>

using namespace std::filesystem;


namespace Mantis
{
    const char Filesystem::Separator = '/';

    String Filesystem::GetExePath()
    {
#ifdef MANTIS_WINDOWS
        wchar_t wBuf[MAX_PATH];
        DWORD pathLen = GetModuleFileName(NULL, wBuf, sizeof(wBuf));

        if (pathLen == 0 || pathLen == sizeof(wBuf))
        {
            PrintLastError();
        }

        return WideCharToString(wBuf);
#endif
    }

#ifdef MANTIS_WINDOWS
    void Filesystem::PrintLastError()
    {
        DWORD error = GetLastError();
        if (error != 0)
        {
            LPSTR messageBuffer = nullptr;
            DWORD size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

            String message(messageBuffer, size);
            LocalFree(messageBuffer);

            Logger::Error("Filesystem", message);
        }
    }

    String Filesystem::WideCharToString(const wchar_t* wStr)
    {
        // create output string
        String path = String();

        // determine size as UTF-8 string
        int sizeRequired = WideCharToMultiByte(CP_UTF8, 0, wStr, -1, NULL, 0, NULL, NULL);

        // error handling
        if (sizeRequired == 0)
        {
            PrintLastError();
            return path;
        }

        path.set_capacity(sizeRequired);

        // convert the path
        int bytesConverted = WideCharToMultiByte(CP_UTF8, 0, wStr, -1, &path[0], sizeRequired, NULL, NULL);

        // error handling
        if (bytesConverted == 0)
        {
            PrintLastError();
            return path;
        }

        return path;
    }
#endif
}
