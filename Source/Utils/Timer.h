#pragma once

#include "Mantis.h"

#include <chrono>

namespace Mantis
{
    /// <summary>
    /// A high resolution timing utility.
    /// </summary>
    class Timer
    {
    public:
        Timer() = default;

        template<typename Rep, typename Period>
        constexpr Timer(const std::chrono::duration<Rep, Period>& duration) :
            m_microseconds(std::chrono::duration_cast<std::chrono::microseconds>(duration).count())
        {}

        /// <summary>
        /// Creates a time value from a number of seconds.
        /// </summary>
        /// <typeparam name="Rep">The type of value to be casted to.</typeparam>
        /// <param name="seconds">Number of seconds.</param>
        template<typename Rep = float>
        static constexpr Timer Seconds(const Rep& seconds)
        {
            return Timer(std::chrono::duration<Rep>(seconds));
        }

        /// <summary>
        /// Creates a time value from a number of milliseconds
        /// </summary>
        /// <typeparam name="Rep">The type of value to be casted to.</typeparam>
        /// <param name="seconds">Number of milliseconds.</param>
        template<typename Rep = int32_t>
        static constexpr Timer Milliseconds(const Rep& milliseconds)
        {
            return Timer(std::chrono::duration<Rep, std::micro>(milliseconds));
        }

        /// <summary>
        /// Creates a time value from a number of microseconds
        /// </summary>
        /// <typeparam name="Rep">The type of value to be casted to.</typeparam>
        /// <param name="seconds">Number of microseconds.</param>
        template<typename Rep = int64_t>
        static constexpr Timer Microseconds(const Rep& microseconds)
        {
            return Timer(std::chrono::duration<Rep, std::micro>(microseconds));
        }

        template<typename Rep, typename Period>
        operator std::chrono::duration<Rep, Period> () const { return std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(m_microseconds); }

        /// <summary>
        /// Gets the time value as a number of seconds.
        /// </summary>
        /// <typeparam name="T">The type of value to be casted to.</typeparam>
        template<typename T = float>
        auto AsSeconds() const
        {
            return static_cast<T>(m_microseconds.count()) / static_cast<T>(1000000);
        }

        /// <summary>
        /// Gets the time value as a number of milliseconds.
        /// </summary>
        /// <typeparam name="T">The type of value to be casted to.</typeparam>
        template<typename T = int32_t>
        auto AsMilliseconds() const
        {
            return static_cast<T>(m_microseconds.count()) / static_cast<T>(1000);
        }

        /// <summary>
        /// Gets the time value as a number of microseconds.
        /// </summary>
        /// <typeparam name="T">The type of value to be casted to.</typeparam>
        template<typename T = int64_t>
        auto AsMicroseconds() const
        {
            return static_cast<T>(m_microseconds.count());
        }

        /// <summary>
        /// Gets the current time of this application.
        /// </summary>
        static Timer Now();

        bool operator == (const Timer& other) const;

        bool operator != (const Timer& other) const;

        bool operator < (const Timer& other) const;

        bool operator <= (const Timer& other) const;

        bool operator > (const Timer& other) const;

        bool operator >= (const Timer& other) const;

        Timer operator - () const;

        friend Timer operator + (const Timer& left, const Timer& right);

        friend Timer operator - (const Timer& left, const Timer& right);

        friend Timer operator * (const Timer& left, const float& right);

        friend Timer operator * (const Timer& left, const int64_t& right);

        friend Timer operator * (const float& left, const Timer& right);

        friend Timer operator * (const int64_t& left, const Timer& right);

        friend Timer operator / (const Timer& left, const float& right);

        friend Timer operator / (const Timer& left, const int64_t& right);

        friend double operator / (const Timer& left, const Timer& right);

        Timer& operator += (const Timer& other);

        Timer& operator -= (const Timer& other);

        Timer& operator *= (const float& other);

        Timer& operator *= (const int64_t& other);

        Timer& operator /= (const float& other);

        Timer& operator /= (const int64_t& other);

    private:
        static const std::chrono::time_point<std::chrono::high_resolution_clock> Start;

        std::chrono::microseconds m_microseconds;
    };
}