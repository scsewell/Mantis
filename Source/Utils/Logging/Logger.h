#pragma once

#include "Mantis.h"

namespace Mantis
{
    class FileStream;

    /// <summary>
    /// Formats and outputs useful debugging information.
    /// </summary>
    class Logger
    {
    public:
        /// <summary>
        /// Logs a debug message. Only has an effect on debug builds.
        /// </summary>
        /// <param name="message">The message to log.</param>
        static void Debug(const String& message);

        /// <summary>
        /// Logs a debug message. Only has an effect on debug builds.
        /// </summary>
        /// <param name="format">The formatting of the arguments.</param>
        /// <param name="args">The arguments to format.</param>
        static void DebugF(const String format, ...);

        /// <summary>
        /// Logs a debug message. Only has an effect on debug builds.
        /// </summary>
        /// <param name="tag">A short description of the message source.</param>
        /// <param name="message">The message to log.</param>
        static void DebugT(const String& tag, const String& message);

        /// <summary>
        /// Logs a debug message. Only has an effect on debug builds.
        /// </summary>
        /// <param name="tag">A short description of the message source.</param>
        /// <param name="format">The formatting of the arguments.</param>
        /// <param name="args">The arguments to format.</param>
        static void DebugTF(const String& tag, const String format, ...);

        /// <summary>
        /// Logs an info message.
        /// </summary>
        /// <param name="message">The message to log.</param>
        static void Info(const String& message);

        /// <summary>
        /// Logs an info message.
        /// </summary>
        /// <param name="format">The formatting of the arguments.</param>
        /// <param name="args">The arguments to format.</param>
        static void InfoF(const String format, ...);

        /// <summary>
        /// Logs an info message.
        /// </summary>
        /// <param name="tag">A short description of the message source.</param>
        /// <param name="message">The message to log.</param>
        static void InfoT(const String& tag, const String& message);

        /// <summary>
        /// Logs an info message.
        /// </summary>
        /// <param name="tag">A short description of the message source.</param>
        /// <param name="format">The formatting of the arguments.</param>
        /// <param name="args">The arguments to format.</param>
        static void InfoTF(const String& tag, const String format, ...);

        /// <summary>
        /// Logs a warning message.
        /// </summary>
        /// <param name="message">The message to log.</param>
        static void Warning(const String& message);

        /// <summary>
        /// Logs a warning message.
        /// </summary>
        /// <param name="format">The formatting of the arguments.</param>
        /// <param name="args">The arguments to format.</param>
        static void WarningF(const String format, ...);

        /// <summary>
        /// Logs a warning message.
        /// </summary>
        /// <param name="tag">A short description of the message source.</param>
        /// <param name="message">The message to log.</param>
        static void WarningT(const String& tag, const String& message);

        /// <summary>
        /// Logs a warning message.
        /// </summary>
        /// <param name="tag">A short description of the message source.</param>
        /// <param name="format">The formatting of the arguments.</param>
        /// <param name="args">The arguments to format.</param>
        static void WarningTF(const String& tag, const String format, ...);

        /// <summary>
        /// Logs an error message.
        /// </summary>
        /// <param name="message">The message to log.</param>
        static void Error(const String& message);

        /// <summary>
        /// Logs an error message.
        /// </summary>
        /// <param name="format">The formatting of the arguments.</param>
        /// <param name="args">The arguments to format.</param>
        static void ErrorF(const String format, ...);

        /// <summary>
        /// Logs an error message.
        /// </summary>
        /// <param name="tag">A short description of the message source.</param>
        /// <param name="message">The message to log.</param>
        static void ErrorT(const String& tag, const String& message);

        /// <summary>
        /// Logs an error message.
        /// </summary>
        /// <param name="tag">A short description of the message source.</param>
        /// <param name="format">The formatting of the arguments.</param>
        /// <param name="args">The arguments to format.</param>
        static void ErrorTF(const String& tag, const String format, ...);

    private:
        /// <summary>
        /// Gets a new string containing a log message.
        /// </summary>
        /// <param name="level">The log message type.</param>
        /// <param name="tag">A short description of the message source.</param>
        /// <param name="message">The message to log.</param>
        static String Append(const String& level, const String& tag, const String& message);

        /// <summary>
        /// Gets a new string containing a log message.
        /// </summary>
        /// <param name="level">The log message type.</param>
        /// <param name="tag">A short description of the message source.</param>
        /// <param name="format">The formatting of the arguments.</param>
        /// <param name="args">The arguments to format.</param>
        static String Append(const String& level, const String& tag, const String& format, va_list args);

        /// <summary>
        /// Writes out the message.
        /// </summary>
        /// <param name="message">The message to log.</param>
        static void Write(const String& message);

        static eastl::shared_ptr<FileStream> s_logStream;
        static std::mutex s_writeLock;
    };
}
