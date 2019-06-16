#include "stdafx.h"
#include "WindowsUtils.h"

#ifdef MANTIS_WINDOWS

#define LOG_TAG MANTIS_TEXT("WinUtils")

namespace Mantis
{
    WString StringToWideChar(const String str)
    {
        // determine size as wide character string
        int sizeRequired = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);

        // error handling
        if (sizeRequired <= 0)
        {
            Logger::ErrorT(LOG_TAG, GetLastWindowsError());
            return nullptr;
        }

        // create output string, don't create room for the null terminator, that is automatic
        WString wstr(sizeRequired - 1, ' ');

        // convert the path
        int bytesConverted = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], sizeRequired);

        // error handling
        if (bytesConverted <= 0)
        {
            Logger::ErrorT(LOG_TAG, GetLastWindowsError());
        }

        return wstr;
    }

    String WideCharToString(const wchar_t* wStr)
    {
        // determine size as UTF-8 string
        int sizeRequired = WideCharToMultiByte(CP_UTF8, 0, wStr, -1, NULL, 0, NULL, NULL);

        // error handling
        if (sizeRequired <= 0)
        {
            Logger::ErrorT(LOG_TAG, GetLastWindowsError());
            return String();
        }

        // create output string, don't create room for the null terminator, that is automatic
        String str(sizeRequired - 1, ' ');

        // convert the string
        int bytesConverted = WideCharToMultiByte(CP_UTF8, 0, wStr, -1, &str[0], sizeRequired, NULL, NULL);

        // error handling
        if (bytesConverted <= 0)
        {
            Logger::ErrorT(LOG_TAG, GetLastWindowsError());
        }

        return str;
    }

    String GetLastWindowsError()
    {
        DWORD error = GetLastError();
        if (error != 0)
        {
            LPSTR messageBuffer = nullptr;
            DWORD size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)& messageBuffer, 0, NULL) + 1;

            String message(messageBuffer, size);
            LocalFree(messageBuffer);
            return message;
        }
        return String();
    }
}

#endif // MANTIS_WINDOWS
