#pragma once

#include "Mantis.h"
#include <stdio.h>

namespace Mantis
{
    /// <summary>
    /// How to seek in file operations.
    /// </summary>
    enum struct SeekMode
    {
        /// <summary>
        /// Seek relative to the start of the file.
        /// </summary>
        Start = SEEK_SET,
        /// <summary>
        /// Seek relative to the current position in the file.
        /// </summary>
        Current = SEEK_CUR,
        /// <summary>
        /// Seek relative to the end of the file.
        /// </summary>
        End = SEEK_END,
    };

    /// <summary>
    /// Represents an file on the filesystem.
    /// </summary>
    class FileStream
    {
    public:
        FileStream(FILE* fileHandle, const String path);

        ~FileStream();

        /// <summary>
        /// Closes the stream.
        /// </summary>
        /// <returns>True if the stream closed sucessfully.</returns>
        bool Close();

        /// <summary>
        /// Reads bytes from a file.
        /// </summary>
        /// <param name="buffer">The destination buffer.</param>
        /// <param name="bufferSize">The size of the destination buffer.</param>
        /// <param name="length">The number of bytes to read.</param>
        /// <returns>True if the read was sucessful.</returns>
        bool Read(uint8_t* buffer, int bufferSize, int length);

        /// <summary>
        /// Reads bytes from a file.
        /// </summary>
        /// <param name="buffer">The destination buffer.</param>
        /// <param name="bufferSize">The size of the destination buffer.</param>
        /// <param name="length">The number of bytes to read.</param>
        /// <param name="offset">The byte offset in the buffer to read to.</param>
        /// <returns>True if the read was sucessful.</returns>
        bool Read(uint8_t* buffer, int bufferSize, int length, int offset);

        /// <summary>
        /// Writes to the file.
        /// </summary>
        /// <param name="data">The buffer to write.</param>
        /// <param name="bufferSize">The size of the data buffer.</param>
        /// <param name="length">The number of bytes to write.</param>
        /// <returns>True if the write was sucessful.</returns>
        bool Write(const uint8_t* data, int bufferSize, int length);

        /// <summary>
        /// Writes to the file.
        /// </summary>
        /// <param name="data">The buffer to write from.</param>
        /// <param name="bufferSize">The size of the data buffer.</param>
        /// <param name="length">The number of bytes to write.</param>
        /// <param name="offset">The byte offset in the buffer to write from.</param>
        /// <returns>True if the write was sucessful.</returns>
        bool Write(const uint8_t* data, int bufferSize, int length, int offset);

        /// <summary>
        /// Gets the current position in the file.
        /// </summary>
        /// <returns>The current position as a byte offset from the start of the file.</returns>
        long GetPosition();

        /// <summary>
        /// Sets the current position in the file to a given byte.
        /// </summary>
        /// <param name="mode">The base offset to seek from.</param>
        /// <param name="offset">The seek offset in bytes.</param>
        bool Seek(SeekMode mode, long offset);

        /// <summary>
        /// Resets the current position to the start of the file.
        /// </summary>
        void Rewind();

        /// <summary>
        /// Flushes the data buffers.
        /// </summary>
        /// <returns>True if the stream flushed sucessfully.</returns>
        bool Flush();

    private:
        /// <summary>
        /// Checks if the stream is currently valid.
        /// </summary>
        /// <returns>True if the stream is valid.</returns>
        bool CheckValid();

        /// <summary>
        /// Checks if the last file operation failed, printing a message if so.
        /// </summary>
        /// <returns>True if the last operation failed.</returns>
        bool CheckError();

        FILE* m_file;
        const String m_path;
    };
}