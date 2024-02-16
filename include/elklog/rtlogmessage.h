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
 * @brief RT-safe class to pass around Log Messages
 *
 * @copyright Copyright 2020-2023 Elk Audio AB, Stockholm
 */

#ifndef RTLOGMESSAGE_H
#define RTLOGMESSAGE_H

#include <chrono>
#include <ostream>
#include <array>
#include <cstring>
#include <cassert>

#include "elk-warning-suppressor/warning_suppressor.hpp"

ELK_PUSH_WARNING
ELK_DISABLE_DEPRECATED_DECLARATIONS
ELK_DISABLE_UNKNOWN_PRAGMAS
ELK_DISABLE_DEPRECATED
#include <spdlog/fmt/bundled/format.h>
#include <spdlog/fmt/bundled/chrono.h>
ELK_POP_WARNING

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

    ~RtLogMessage() = default;

    RtLogMessage& operator=(const RtLogMessage& rhs)
    {
        if (this == &rhs)
        {
            return *this;
        }
        _level = rhs._level;
        _timestamp = rhs._timestamp;
        _length = rhs._length;

        std::copy(rhs._buffer.begin(), rhs._buffer.begin() + rhs._length + 1, _buffer.begin());
        return *this;
    }

    /*
    * @brief Returns a null-terminated string with a formatted message.
    */
    const char* message() const
    {
        return _buffer.data();
    }

    RtLogLevel level() const
    {
        return _level;
    }

    std::chrono::nanoseconds timestamp() const
    {
        return _timestamp;
    }

    /*
     * @brief Returns the length of the formatted message excluding null termination
     */
    size_t length() const
    {
        return _length;
    }

    /**
     * @brief Set the log message string with formatting
     */
    template<typename... Args>
    void set_message(RtLogLevel level, std::chrono::nanoseconds timestamp,
                     const char* format_str, Args&&... args)
    {
        _level = level;
        _timestamp = timestamp;
        auto end = fmt::format_to_n(_buffer.data(), buffer_len - 1, format_str, args...);

        // Add null-termination character
        *end.out = '\0';
        _length = static_cast<int>(std::distance(_buffer.data(), end.out));
    }

private:
    RtLogLevel _level;
    std::chrono::nanoseconds _timestamp;
    int _length;
    std::array<char, buffer_len> _buffer;
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


} // namespace elklog

#endif /* RTLOGMESSAGE_H */
