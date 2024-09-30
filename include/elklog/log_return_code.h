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
 * @brief Log initialization return codes
 *
 * @copyright Copyright 2020-2023 Elk Audio AB, Stockholm
 */

#ifndef LOG_RETURN_CODE_H
#define LOG_RETURN_CODE_H

#include <ostream>

#include "elk-warning-suppressor/warning_suppressor.hpp"

#ifndef ELKLOG_DISABLE_LOGGING

ELK_PUSH_WARNING
ELK_DISABLE_SWITCH_ENUM
ELK_DISABLE_FLOAT_EQUAL
#include "spdlog/fmt/bundled/format.h"
ELK_POP_WARNING

#endif
namespace elklog {

enum class Status : int
{
    OK = 0,
    INVALID_LOG_LEVEL = 1,
    FAILED_TO_START_LOGGER = 2,
    INVALID_FLUSH_INTERVAL = 3,
    LOGGER_NOT_INITIALIZED = 4
};

inline std::ostream& operator << (std::ostream& o, const Status& c)
{
    switch (c)
    {
    case Status::OK:
        o << "Ok";
        break;

    case Status::INVALID_LOG_LEVEL:
        o << "Invalid log level";
        break;

    case Status::INVALID_FLUSH_INTERVAL:
        o << "Invalid Flush interval";
        break;

    case Status::LOGGER_NOT_INITIALIZED:
        o << "Logger is not initialized";
        break;

    case Status::FAILED_TO_START_LOGGER:
        o << "Failed to initialize SPD logger instance";
        break;
    }

    return o;
}
} // namespace elklog

#ifndef ELKLOG_DISABLE_LOGGING
// Note this is explicitly defined outside the elklog namespace
template <>
struct fmt::formatter<elklog::Status>: public formatter<string_view>
{
    // parse is inherited from formatter<string_view>.
    auto format(elklog::Status c, format_context& ctx) const;
};

auto inline fmt::formatter<elklog::Status>::format(elklog::Status c, format_context& ctx) const
{
    switch (c)
    {
        case elklog::Status::OK:
            return formatter<string_view>::format("OK", ctx);

        case elklog::Status::INVALID_LOG_LEVEL:
            return formatter<string_view>::format("INVALID_LOG_LEVEL", ctx);

        case elklog::Status::INVALID_FLUSH_INTERVAL:
            return formatter<string_view>::format("INVALID_FLUSH_INTERVAL", ctx);

        case elklog::Status::LOGGER_NOT_INITIALIZED:
            return formatter<string_view>::format("LOGGER_NOT_INITIALIZED", ctx);

        case elklog::Status::FAILED_TO_START_LOGGER:
            return formatter<string_view>::format("FAILED_TO_START_LOGGER", ctx);

        default:
            return formatter<string_view>::format("", ctx);
    }
}

#endif

#endif /* LOG_RETURN_CODE_H */
