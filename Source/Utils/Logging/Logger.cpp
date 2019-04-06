#include "stdafx.h"
#include "Logger.h"

namespace Mantis
{
    void Logger::Debug(const String& message)
    {
#ifdef MANTIS_DEBUG
        Write(Append("Debug", "", message));
#endif
    }

    template<typename... Args>
    void Logger::Debug(const String& format, Args&& ... args)
    {
#ifdef MANTIS_DEBUG
        Write(Append("Debug", "", format, std::forward<Args>(args)...));
#endif
    }

    void Logger::Debug(const String& tag, const String& message)
    {
#ifdef MANTIS_DEBUG
        Write(Append("Debug", tag, message));
#endif
    }

    template<typename... Args>
    void Logger::Debug(const String& tag, const String& format, Args&& ... args)
    {
#ifdef MANTIS_DEBUG
        Write(Append("Debug", tag, format, std::forward<Args>(args)...));
#endif
    }

    void Logger::Info(const String& message)
    {
        Write(Append("Info", "", message));
    }

    template<typename... Args>
    void Logger::Info(const String& format, Args&& ... args)
    {
        Write(Append("Info", "", format, std::forward<Args>(args)...));
    }

    void Logger::Info(const String& tag, const String& message)
    {
        Write(Append("Info", tag, message));
    }

    template<typename... Args>
    void Logger::Info(const String& tag, const String& format, Args&& ... args)
    {
        Write(Append("Info", tag, format, std::forward<Args>(args)...));
    }

    void Logger::Warning(const String& message)
    {
        Write(Append("Warning", "", message));
    }

    template<typename... Args>
    void Logger::Warning(const String& format, Args&& ... args)
    {
        Write(Append("Warning", "", format, std::forward<Args>(args)...));
    }

    void Logger::Warning(const String& tag, const String& message)
    {
        Write(Append("Warning", tag, message));
    }

    template<typename... Args>
    void Logger::Warning(const String& tag, const String& format, Args&& ... args)
    {
        Write(Append("Warning", tag, format, std::forward<Args>(args)...));
    }

    void Logger::Error(const String& message)
    {
        Write(Append("Error", "", message));
    }

    template<typename... Args>
    void Logger::Error(const String& format, Args&& ... args)
    {
        Write(Append("Error", "", format, std::forward<Args>(args)...));
    }

    void Logger::Error(const String& tag, const String& message)
    {
        Write(Append("Error", tag, message));
    }

    template<typename... Args>
    void Logger::Error(const String& tag, const String& format, Args&& ... args)
    {
        Write(Append("Error", tag, format, std::forward<Args>(args)...));
    }

    String Logger::Append(const String& level, const String& tag, const String& message)
    {
        String str = String();
        str.append_sprintf("%8s|%20s| ", level, tag);
        str.append(message);
        return str;
    }

    template<typename... Args>
    String Logger::Append(const String& level, const String& tag, const String& format, Args&& ... args)
    {
        String str = String();
        str.append_sprintf("%8s|%20s| ", level, tag);
        str.append_sprintf_va_list(format.c_str(), args);
        return str;
    }

    void Logger::Write(const String& message)
    {
#if defined(MANTIS_WINDOWS) && defined(MANTIS_DEBUG)
        OutputDebugStringA(message.c_str());
#endif
    }
}