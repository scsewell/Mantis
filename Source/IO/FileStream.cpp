#include "stdafx.h"
#include "FileStream.h"

namespace Mantis
{
    FileStream::FileStream(FILE* fileHandle, const String path)
        : m_file(fileHandle)
        , m_path(path)
    {
    }

    FileStream::~FileStream()
    {
        Close();
    }

    bool FileStream::Close()
    {
        bool result = true;

        if (m_file != nullptr)
        {
            fclose(m_file);
            result = !CheckError();
            m_file = nullptr;
        }

        return result;
    }

    bool FileStream::Read(uint8_t* buffer, int bufferSize, int length)
    {
        return Read(buffer, bufferSize, length, 0);
    }

    bool FileStream::Read(uint8_t* buffer, int bufferSize, int length, int offset)
    {
        if (CheckValid())
        {
            // check for buffer underrun
            if (offset < 0)
            {
                Logger::ErrorTF(MANTIS_TEXT("Filesystem"), "Aborted reading from file \"%s\" using a negative offset %i!", m_path.c_str(), offset);
                return false;
            }

            // check for buffer overrun
            int remaining = bufferSize - offset;
            if (length > remaining)
            {
                Logger::ErrorTF(MANTIS_TEXT("Filesystem"), "Aborted reading %i bytes from file \"%s\", destination buffer only has %i bytes remaining!", length, m_path.c_str(), remaining);
                return false;
            }

            // read the data
            fread(buffer + offset, length, 1, m_file);
            return !CheckError();
        }
        return false;
    }

    bool FileStream::Write(const uint8_t* data, int bufferSize, int length)
    {
        return Write(data, bufferSize, length, 0);
    }

    bool FileStream::Write(const uint8_t* data, int bufferSize, int length, int offset)
    {
        if (CheckValid())
        {
            // check for buffer underrun
            if (offset < 0)
            {
                Logger::ErrorTF(MANTIS_TEXT("Filesystem"), "Aborted writing to file \"%s\" using a negative offset %i!", m_path.c_str(), offset);
                return false;
            }

            // check for buffer overrun
            int remaining = bufferSize - offset;
            if (length > remaining)
            {
                Logger::ErrorTF(MANTIS_TEXT("Filesystem"), "Aborted writing to %i bytes to file \"%s\", source buffer only has %i bytes remaining!", length, m_path.c_str(), remaining);
                return false;
            }

            // write the data
            fwrite(data + offset, length, 1, m_file);
            return !CheckError();
        }
        return false;
    }

    long FileStream::GetPosition()
    {
        if (CheckValid())
        {
            return ftell(m_file);
        }
        return -1;
    }

    bool FileStream::Seek(SeekMode mode, long offset)
    {
        if (CheckValid())
        {
            fseek(m_file, offset, static_cast<int>(mode));
            return !CheckError();
        }
        return false;
    }

    void FileStream::Rewind()
    {
        if (CheckValid())
        {
            rewind(m_file);
        }
    }

    bool FileStream::Flush()
    {
        if (CheckValid())
        {
            fflush(m_file);
            return !CheckError();
        }
        return false;
    }

    bool FileStream::CheckValid()
    {
        if (m_file == nullptr)
        {
            Logger::ErrorTF(MANTIS_TEXT("Filesystem"), "Cannot operate on file \"%s\", the stream is closed!", m_path.c_str());
            return false;
        }
        return true;
    }

    bool FileStream::CheckError()
    {
        if (ferror(m_file))
        {
            Logger::ErrorTF(MANTIS_TEXT("Filesystem"), "File operation on \"%s\" failed: %s", m_path.c_str(), strerror(errno));
            clearerr(m_file);
            return true;
        }
        return false;
    }
}