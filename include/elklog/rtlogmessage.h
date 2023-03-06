/**
 * @brief RT-safe class to pass around Log Messages
 *
 * @copyright Copyright 2017-2023 Elk AB, Stockholm
 */

#ifndef RTLOGMESSAGE_H
#define RTLOGMESSAGE_H

#include <chrono>
#include <ostream>
#include <cstring>

#include <spdlog/fmt/bundled/format.h>
#include <spdlog/fmt/bundled/chrono.h>

#include "rtloglevel.h"

namespace elklog {

template<size_t buffer_len>
class RtLogMessage
{
public:

    RtLogMessage() :
        _level(RtLogLevel::INFO),
        _timestamp(std::chrono::nanoseconds(0))
    {
        std::fill(_buffer, _buffer + buffer_len, 0);
    }

    RtLogMessage(RtLogLevel level, std::chrono::nanoseconds timestamp, const char* message) :
        _level(level),
        _timestamp(timestamp)
    {
        _set_message_str(message);
    }

    virtual ~RtLogMessage() = default;

    RtLogMessage& operator=(const RtLogMessage& rhs)
    {
        if (this == &rhs)
        {
            return *this;
        }
        _level = rhs._level;
        _timestamp = rhs._timestamp;
        _set_message_str(&rhs._buffer[0]);
        return *this;
    }

    const char* message() const
    {
        return &_buffer[0];
    }

    RtLogLevel level() const
    {
        return _level;
    }

    std::chrono::nanoseconds timestamp() const
    {
        return _timestamp;
    }

    /**
     * @brief Convenience method to reset a message contents,
     *        useful to reuse a message object.
     */
    template<typename... Args>
    void reset(RtLogLevel level, std::chrono::nanoseconds timestamp,
               const char* format_str, Args&&... args)
    {
        fmt::memory_buffer temp_buffer;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        fmt::format_to(temp_buffer, format_str, args...);
#pragma GCC diagnostic pop
        // Add null-termination character
        temp_buffer.push_back(0);
        _set_message_str(temp_buffer.data());
        _level = level;
        _timestamp = timestamp;
    }

private:
    void _set_message_str(const char* message)
    {
        std::strncpy(&_buffer[0], message, buffer_len);
    }

    RtLogLevel _level {0};
    std::chrono::nanoseconds _timestamp {0};
    char _buffer[buffer_len];
};

template<size_t buffer_len>
inline std::ostream& operator << (std::ostream& o, const RtLogMessage<buffer_len>& m)
{
    using namespace std::chrono;
    constexpr auto basetime = time_point<system_clock>{};
    auto timestamp_millis = duration_cast<milliseconds>(m.timestamp())
        - duration_cast<milliseconds>(duration_cast<seconds>(m.timestamp()));
    std::time_t abs_time = system_clock::to_time_t(basetime + m.timestamp());
    fmt::memory_buffer temp_buffer;
    fmt::format_to(temp_buffer, "{:%F %T}.{}", *std::localtime(&abs_time), timestamp_millis.count());
    temp_buffer.push_back(0);

    o << temp_buffer.data() << " " << m.level() << " [RT] " << m.message();
    return o;
}


} // namespace aloha

#endif /* RTLOGMESSAGE_H */
