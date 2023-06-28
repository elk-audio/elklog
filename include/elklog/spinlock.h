/*
 * Copyright 2020-2023 Elk Audio AB
 * ElkLog is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * Twine is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with Twine.
 * If not, see http://www.gnu.org/licenses/ .
 */

/**
 * @brief Basic spinlock implementation safe for xenomai use
 * @copyright 2017-2023 Elk Audio AB, Stockholm
 */

#ifndef ELKLOG_SPINLOCK_H
#define ELKLOG_SPINLOCK_H

#ifdef ELKLOG_MULTI_THREADED_RT_LOGGING

#include <atomic>

namespace elklog {

// since std::hardware_destructive_interference_size is not yet supported in GCC 11
constexpr int ASSUMED_CACHE_LINE_SIZE = 64;

/**
 * @brief Simple rt-safe test-and-set spinlock
 */
class SpinLock
{
public:
    SpinLock() = default;

    void lock()
    {
        while (flag.load(std::memory_order_relaxed) == true)
        {
            /* Spin until flag is cleared. According to
             * https://geidav.wordpress.com/2016/03/23/test-and-set-spinlocks/
             * this is better as it causes fewer cache invalidations */
        }
        while (flag.exchange(true, std::memory_order_acquire))
        {}
    }

    void unlock()
    {
        flag.store(false, std::memory_order_release);
    }

private:
    SpinLock(const SpinLock& other) = delete;
    SpinLock& operator=(const SpinLock&) = delete;

    alignas(ASSUMED_CACHE_LINE_SIZE) std::atomic_bool flag{false};
};

} // end namespace elklog

#endif

#ifndef ELKLOG_MULTI_THREADED_RT_LOGGING
namespace elklog {
/**
 * @brief No-op spinlock
 */
class SpinLock
{
public:
    void lock() {}
    void unlock() {}
};

} // end namespace elklog

#endif //ELKLOG_MULTI_THREADED_RT_LOGGING

#endif //ELKLOG_SPINLOCK_H
