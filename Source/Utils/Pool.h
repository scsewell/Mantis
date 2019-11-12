#pragma once

#include "Mantis.h"

namespace Mantis
{
    template<class T, uint32_t maxCapacity>
    class FixedSizePool
        : public NonCopyable
    {
    public:
        /// <summary>
        /// Creates a new pool.
        /// </summary>
        /// <param name="name">The name of the pool.</param>
        FixedSizePool(String name) 
            : m_isUsed({})
            , m_name(name)
        {
        }

        /// <summary>
        /// Gets an instance from this pool.
        /// </summary>
        /// <returns>A pointer to the aquired instance or null in pool has no free elements.</returns>
        eastl::unique_ptr<T, eastl::function<void(T*)>> Acquire()
        {
            // look for an unused instance
            uint32_t index = 0;
            while (index < maxCapacity && m_isUsed[index])
            {
                ++index;
            }

            if (index < maxCapacity)
            {
                m_isUsed[index] = true;
                Initialize(&m_instances[index]);
                return eastl::unique_ptr<T, eastl::function<void(T*)>>(&m_instances[index], [this](T* instance) -> void
                    {
                        Free(instance);
                    });
            }
            else
            {
                Logger::ErrorF("All %u instances in fixed size pool \"%s\" are used!", maxCapacity, name);
                return nullptr;
            }
        }

    protected:
        virtual void Initialize(T* instance) = 0;
        virtual void Release(T* instance) = 0;

    private:
        void Free(T* instance)
        {
            Release(element);
            m_isUsed[instance - &m_instances[0]] = false;
        }

        bool m_isUsed[maxCapacity];
        T m_instances[maxCapacity];
        String m_name;
    };

    template<class T, uint32_t maxCapacity>
    class ThreadSafeFixedSizePool
        : public NonCopyable
    {
    public:
        /// <summary>
        /// Creates a new pool.
        /// </summary>
        /// <param name="name">The name of the pool.</param>
        ThreadSafeFixedSizePool(String name)
            : m_isUsed({})
            , m_name(name)
        {
        }

        /// <summary>
        /// Gets an instance from this pool.
        /// </summary>
        /// <returns>A pointer to the aquired instance or null in pool has no free elements.</returns>
        eastl::unique_ptr<T, eastl::function<void(T*)>> Acquire()
        {
            m_lock.lock();

            // look for an unused instance
            uint32_t index = 0;
            while (index < maxCapacity && m_isUsed[index])
            {
                ++index;
            }

            if (index < maxCapacity)
            {
                m_isUsed[index] = true;
                m_lock.unlock();

                Initialize(&m_instances[index]);
                return eastl::unique_ptr<T, eastl::function<void(T*)>>(&m_instances[index], [this](T* instance) -> void
                    {
                        Free(instance);
                    });
            }
            else
            {
                m_lock.unlock();
                Logger::ErrorF("All %u instances in fixed size pool \"%s\" are used!", maxCapacity, name);
                return nullptr;
            }
        }

    protected:
        virtual void Initialize(T* instance) = 0;
        virtual void Release(T* instance) = 0;

    private:
        void Free(T* instance)
        {
            Release(element);

            m_lock.lock();
            m_isUsed[instance - &m_instances[0]] = false;
            m_lock.unlock();
        }

        bool m_isUsed[maxCapacity];
        T m_instances[maxCapacity];
        std::mutex m_lock;
        String m_name;
    };
}
