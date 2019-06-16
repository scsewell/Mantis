#include "Timer.h"

namespace Mantis
{
    const std::chrono::time_point<std::chrono::high_resolution_clock> Timer::Start(std::chrono::high_resolution_clock::now());

    Timer Timer::Now()
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - Start);
    }

    bool Timer::operator == (const Timer& other) const
    {
        return m_microseconds == other.m_microseconds;
    }

    bool Timer::operator != (const Timer& other) const
    {
        return m_microseconds != other.m_microseconds;
    }

    bool Timer::operator < (const Timer& other) const
    {
        return m_microseconds < other.m_microseconds;
    }

    bool Timer::operator <= (const Timer& other) const
    {
        return m_microseconds <= other.m_microseconds;
    }

    bool Timer::operator > (const Timer& other) const
    {
        return m_microseconds > other.m_microseconds;
    }

    bool Timer::operator >= (const Timer& other) const
    {
        return m_microseconds >= other.m_microseconds;
    }

    Timer Timer::operator - () const
    {
        return Timer(-m_microseconds);
    }

    Timer operator + (const Timer& left, const Timer& right)
    {
        return left.m_microseconds + right.m_microseconds;
    }

    Timer operator - (const Timer& left, const Timer& right)
    {
        return left.m_microseconds - right.m_microseconds;
    }

    Timer operator * (const Timer& left, const float& right)
    {
        return left.m_microseconds* right;
    }

    Timer operator * (const Timer& left, const int64_t& right)
    {
        return left.m_microseconds* right;
    }

    Timer operator * (const float& left, const Timer& right)
    {
        return right * left;
    }

    Timer operator * (const int64_t& left, const Timer& right)
    {
        return right * left;
    }

    Timer operator / (const Timer& left, const float& right)
    {
        return left.m_microseconds / right;
    }

    Timer operator / (const Timer& left, const int64_t& right)
    {
        return left.m_microseconds / right;
    }

    double operator / (const Timer& left, const Timer& right)
    {
        return static_cast<double>(left.m_microseconds.count()) / static_cast<double>(right.m_microseconds.count());
    }

    Timer& Timer::operator += (const Timer& other)
    {
        return *this = *this + other;
    }

    Timer& Timer::operator -= (const Timer& other)
    {
        return *this = *this - other;
    }

    Timer& Timer::operator *= (const float& other)
    {
        return *this = *this * other;
    }

    Timer& Timer::operator *= (const int64_t& other)
    {
        return *this = *this * other;
    }

    Timer& Timer::operator /= (const float& other)
    {
        return *this = *this / other;
    }

    Timer& Timer::operator /= (const int64_t& other)
    {
        return *this = *this / other;
    }
}
