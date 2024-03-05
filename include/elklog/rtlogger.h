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
 * @brief Real-Time logger that can be used to pass messages
 *        from RT to non-RT and merged into the main log system used.
 *
 *        Not safe to log messages from multiple RT threads at the same time.
 *
 * @copyright Copyright 2020-2023 Elk Audio AB, Stockholm
 */

#ifndef RT_LOGGER_H
#define RT_LOGGER_H

#include <string>
#include <functional>

#include "rtlogmessage.h"

#ifndef ELKLOG_DISABLE_LOGGING

#include <map>
#include <algorithm>
#include <chrono>
#include <thread>
#include <atomic>

#include "fifo/circularfifo_memory_relaxed_aquire_release.h"
#include "twine/twine.h"

#include "spinlock.h"


namespace elklog {

template<size_t message_len, size_t fifo_size>
class RtLogger
{
public:
    RtLogger(std::chrono::milliseconds consumer_poll_period,
             std::function<void(const RtLogMessage<message_len>& msg)> consumer_callback,
             const std::string& min_log_level) :
        _consumer_callback(consumer_callback)
    {
        set_log_level(min_log_level);

        _sleep_period = std::chrono::milliseconds(consumer_poll_period);
        _consumer_running.store(true);
        _consumer_thread = std::thread(&RtLogger::_consumer_worker, this);
    }

    virtual ~RtLogger()
    {
        _consumer_running.store(false);
        if (_consumer_thread.joinable())
        {
            _consumer_thread.join();
        }
    }

    void set_log_level(const std::string& min_log_level)
    {
        std::map<std::string, RtLogLevel> level_map;
        level_map["debug"] = RtLogLevel::DBG;
        level_map["info"] = RtLogLevel::INFO;
        level_map["warning"] = RtLogLevel::WARNING;
        level_map["error"] = RtLogLevel::ERROR;

        std::string log_level_lowercase = min_log_level;
        std::transform(min_log_level.begin(), min_log_level.end(), log_level_lowercase.begin(), ::tolower);

        if (level_map.count(log_level_lowercase) > 0)
        {
            _min_log_level = level_map[log_level_lowercase];
        }
        else
        {
            _min_log_level = RtLogLevel::INFO;
        }
    }

    template<RtLogLevel level, typename... Args>
    void log(const char* format_str, Args&&... args)
    {
        if (_min_log_level < level)
        {
            return;
        }
        RtLogMessage<message_len> message;
        message.set_message(level, twine::current_rt_time(), format_str, args...);

        _lock.lock();
        _queue.push(message);
        _lock.unlock();
    }

    template<typename... Args>
    void log_debug(const char* format_str, Args&&... args)
    {
        log<RtLogLevel::DBG>(format_str, args...);
    }

    template<typename... Args>
    void log_info(const char* format_str, Args&&... args)
    {
        log<RtLogLevel::INFO>(format_str, args...);
    }

    template<typename... Args>
    void log_warning(const char* format_str, Args&&... args)
    {
        log<RtLogLevel::WARNING>(format_str, args...);
    }

    template<typename... Args>
    void log_error(const char* format_str, Args&&... args)
    {
        log<RtLogLevel::ERROR>(format_str, args...);
    }


private:
    void _consumer_worker()
    {
        RtLogMessage<message_len> message;
        while (_consumer_running)
        {
            while (_queue.pop(message))
            {
                _consumer_callback(message);
            }
            std::this_thread::sleep_for(_sleep_period);
        }
    }

    std::thread _consumer_thread;
    std::atomic<bool> _consumer_running {false};
    std::chrono::milliseconds _sleep_period;

    SpinLock _lock;

    memory_relaxed_aquire_release::CircularFifo<RtLogMessage<message_len>, fifo_size> _queue;

    std::function<void(const RtLogMessage<message_len>& msg)> _consumer_callback;

    RtLogLevel _min_log_level;
};

} // namespace elklog

#else // ELKLOG_DISABLE_LOGGING

namespace elklog {

template<size_t message_len, size_t fifo_size>
class RtLogger
{
public:
    RtLogger(int /*consumer_poll_period_ms*/,
             std::function<void(const RtLogMessage<message_len>& msg)> /*consumer_callback*/,
             const std::string& /*min_log_level*/)
    {}

    virtual ~RtLogger() = default;

    template<RtLogLevel level, typename... Args>
    void log(const char* /*format_str*/, Args&&... /*args*/)
    {}

    template<typename... Args>
    void log_debug(const char* /*format_str*/, Args&&... /*args*/)
    {}

    template<typename... Args>
    void log_info(const char* /*format_str*/, Args&&... /*args*/)
    {}

    template<typename... Args>
    void log_warning(const char* /*format_str*/, Args&&... /*args*/)
    {}

    template<typename... Args>
    void log_error(const char* /*format_str*/, Args&&... /*args*/)
    {}

};

} // namespace elklog

#endif // ELKLOG_DISABLE_LOGGING

#endif /* RT_LOGGER_H */

