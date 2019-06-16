#pragma once

namespace Mantis
{
    /// <summary>
    /// A class that removes the copy constructor and operator from derived classes while leaving move.
    /// </summary>
    class NonCopyable
    {
    protected:
        NonCopyable() = default;
        virtual ~NonCopyable() = default;

    public:
        NonCopyable(const NonCopyable&) = delete;
        NonCopyable(NonCopyable&&) = default;
        NonCopyable& operator=(const NonCopyable&) = delete;
        NonCopyable& operator=(NonCopyable&&) = default;
    };
}