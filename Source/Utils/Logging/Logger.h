#pragma once

#include "Mantis.h"

namespace Mantis
{
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
        template<typename... Args>
        static void Debug(const String& format, Args&& ... args);

        /// <summary>
        /// Logs a debug message. Only has an effect on debug builds.
        /// </summary>
        /// <param name="tag">A short description of the message source.</param>
        /// <param name="message">The message to log.</param>
        static void Debug(const String& tag, const String& message);

        /// <summary>
        /// Logs a debug message. Only has an effect on debug builds.
        /// </summary>
        /// <param name="tag">A short description of the message source.</param>
        /// <param name="format">The formatting of the arguments.</param>
        /// <param name="args">The arguments to format.</param>
        template<typename... Args>
        static void Debug(const String& tag, const String& format, Args&& ... args);

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
        template<typename... Args>
        static void Info(const String& format, Args&& ... args);

        /// <summary>
        /// Logs an info message.
        /// </summary>
        /// <param name="tag">A short description of the message source.</param>
        /// <param name="message">The message to log.</param>
        static void Info(const String& tag, const String& message);

        /// <summary>
        /// Logs an info message.
        /// </summary>
        /// <param name="tag">A short description of the message source.</param>
        /// <param name="format">The formatting of the arguments.</param>
        /// <param name="args">The arguments to format.</param>
        template<typename... Args>
        static void Info(const String& tag, const String& format, Args&& ... args);

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
        template<typename... Args>
        static void Warning(const String& format, Args&& ... args);

        /// <summary>
        /// Logs a warning message.
        /// </summary>
        /// <param name="tag">A short description of the message source.</param>
        /// <param name="message">The message to log.</param>
        static void Warning(const String& tag, const String& message);

        /// <summary>
        /// Logs a warning message.
        /// </summary>
        /// <param name="tag">A short description of the message source.</param>
        /// <param name="format">The formatting of the arguments.</param>
        /// <param name="args">The arguments to format.</param>
        template<typename... Args>
        static void Warning(const String& tag, const String& format, Args&& ... args);

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
        template<typename... Args>
        static void Error(const String& format, Args&& ... args);

        /// <summary>
        /// Logs an error message.
        /// </summary>
        /// <param name="tag">A short description of the message source.</param>
        /// <param name="message">The message to log.</param>
        static void Error(const String& tag, const String& message);

        /// <summary>
        /// Logs an error message.
        /// </summary>
        /// <param name="tag">A short description of the message source.</param>
        /// <param name="format">The formatting of the arguments.</param>
        /// <param name="args">The arguments to format.</param>
        template<typename... Args>
        static void Error(const String& tag, const String& format, Args&& ... args);

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
        template<typename... Args>
        static String Append(const String& level, const String& tag, const String& format, Args&& ... args);

        /// <summary>
        /// Writes out the message.
        /// </summary>
        /// <param name="message">The message to log.</param>
        static void Write(const String& message);
    };
}
