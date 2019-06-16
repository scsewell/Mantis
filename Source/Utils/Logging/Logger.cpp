#include "stdafx.h"
#include "Logger.h"

#include "IO/Filesystem.h"
#include "IO/FileStream.h"

namespace Mantis
{
    eastl::shared_ptr<FileStream> Logger::s_logStream = nullptr;
    std::mutex Logger::s_writeLock;

    void Logger::Debug(const String& message)
    {
#ifdef MANTIS_DEBUG
        Write(Append("Debug", "", message));
#endif
    }

    void Logger::DebugF(const String format, ...)
    {
#ifdef MANTIS_DEBUG
        va_list args;
        va_start(args, format);
        Write(Append("Debug", "", format, args));
        va_end(args);
#endif
    }

    void Logger::DebugT(const String& tag, const String& message)
    {
#ifdef MANTIS_DEBUG
        Write(Append("Debug", tag, message));
#endif
    }

    void Logger::DebugTF(const String& tag, const String format, ...)
    {
#ifdef MANTIS_DEBUG
        va_list args;
        va_start(args, format);
        Write(Append("Debug", tag, format, args));
        va_end(args);
#endif
    }

    void Logger::Info(const String& message)
    {
        Write(Append("Info", "", message));
    }

    void Logger::InfoF(const String format, ...)
    {
        va_list args;
        va_start(args, format);
        Write(Append("Info", "", format, args));
        va_end(args);
    }

    void Logger::InfoT(const String& tag, const String& message)
    {
        Write(Append("Info", tag, message));
    }

    void Logger::InfoTF(const String& tag, const String format, ...)
    {
        va_list args;
        va_start(args, format);
        Write(Append("Info", tag, format, args));
        va_end(args);
    }

    void Logger::Warning(const String& message)
    {
        Write(Append("Warning", "", message));
    }

    void Logger::WarningF(const String format, ...)
    {
        va_list args;
        va_start(args, format);
        Write(Append("Warning", "", format, args));
        va_end(args);
    }

    void Logger::WarningT(const String& tag, const String& message)
    {
        Write(Append("Warning", tag, message));
    }

    void Logger::WarningTF(const String& tag, const String format, ...)
    {
        va_list args;
        va_start(args, format);
        Write(Append("Warning", tag, format, args));
        va_end(args);
    }

    void Logger::Error(const String& message)
    {
        Write(Append("Error", "", message));
    }

    void Logger::ErrorF(const String format, ...)
    {
        va_list args;
        va_start(args, format);
        Write(Append("Error", "", format, args));
        va_end(args);
    }

    void Logger::ErrorT(const String& tag, const String& message)
    {
        Write(Append("Error", tag, message));
    }

    void Logger::ErrorTF(const String& tag, const String format, ...)
    {
        va_list args;
        va_start(args, format);
        Write(Append("Error", tag, format, args));
        va_end(args);
    }

    String Logger::Append(const String& level, const String& tag, const String& message)
    {
        String str = String();
        str.append_sprintf("%-8s|%-16s| ", level, tag);
        str.append(message);
        str.append("\r\n");
        return str;
    }

    String Logger::Append(const String& level, const String& tag, const String& format, va_list args)
    {
        String str = String();
        str.append_sprintf("%-8s|%-16s| ", level, tag);
        str.append_sprintf_va_list(format.c_str(), args);
        str.append("\r\n");
        return str;
    }

    void Logger::Write(const String& message)
    {
        // ensure only one thread is writing to the file at a time
        std::lock_guard<std::mutex> guard(s_writeLock);

#if defined(MANTIS_WINDOWS) && defined(MANTIS_DEBUG)
        OutputDebugStringA(message.c_str());
#endif

        if (!s_logStream)
        {
            s_logStream = Filesystem::Open(PathRoot::OutputDir, "Log.txt", FileMode::Overwrite);
        }

        if (s_logStream)
        {
            s_logStream->Write((uint8_t*)message.data(), static_cast<int>(message.capacity()), static_cast<int>(message.length()));
            s_logStream->Flush();
        }
    }
}
