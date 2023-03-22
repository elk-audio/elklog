/*
 * Copyright 2020-2023 Modern Ancient Instruments Networked AB, dba Elk
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
 * @brief RT-safe class to pass around Log Messages
 *
 * @copyright Copyright 2020-2023 Elk AB, Stockholm
 */

#ifndef RTLOGMESSAGE_H
#define RTLOGMESSAGE_H

#include <chrono>
#include <ostream>
#include <cstring>
#include <cassert>

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
        _timestamp(std::chrono::nanoseconds(0)),
        _length(0)
    {
        _buffer[0] = '\0';
    }

    // TODO - Keep? Not used in elklog.
    /*RtLogMessage(RtLogLevel level, std::chrono::nanoseconds timestamp, const char* message) :
        _level(level),
        _timestamp(timestamp)
    {
        _set_message_str(message);
    }*/

    virtual ~RtLogMessage() = default;

    RtLogMessage& operator=(const RtLogMessage& rhs)
    {
        if (this == &rhs)
        {
            return *this;
        }
        _level = rhs._level;
        _timestamp = rhs._timestamp;
        _length = rhs._length;
        _set_message_str(rhs._buffer.data());

        //std::copy(rhs._buffer.begin(), rhs._buffer.begin() + rhs._length, _buffer.begin());
        //_set_message_str(rhs._buffer.begin(), rhs._buffer.begin() + rhs._length);
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
        //_set_message_str(temp_buffer.begin(), end + 1);
        _set_message_str(&temp_buffer[0]);
        _level = level;
        _timestamp = timestamp;
    }

private:
    void _set_message_str(const char* message)
    {
        std::strncpy(_buffer.data(), message, buffer_len);
        _length = std::strlen(message);
    }

    RtLogLevel _level;
    std::chrono::nanoseconds _timestamp;
    size_t  _length;
    std::array<char, buffer_len> _buffer;

    void _set_message_str(const char* message_start, const char* message_end)
    {
        std::copy(message_start, message_end, _buffer.data());
        _length = std::distance(message_start, message_end);
    }
};

static_assert(std::is_trivially_copyable<RtLogLevel>::value);
static_assert(std::is_trivially_copyable<std::chrono::nanoseconds>::value);
static_assert(std::is_trivially_copyable<std::array<char,100>>::value);

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
