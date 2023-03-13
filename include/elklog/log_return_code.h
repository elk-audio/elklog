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
 * @brief Log initialization return codes
 *
 * @copyright Copyright 2020-2023 Elk AB, Stockholm
 */

#ifndef LOG_RETURN_CODE_H
#define LOG_RETURN_CODE_H

#include <ostream>

namespace elklog {

enum class Status : int
{
    OK = 0,
    INVALID_LOG_LEVEL = 1,
    FAILED_TO_START_LOGGER = 2,
    INVALID_FLUSH_INTERVAL = 3
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

    case Status::FAILED_TO_START_LOGGER:
        o << "Failed to initialize SPD logger instance";
        break;
    }

    return o;
}

} // namespace aloha


#endif /* LOG_RETURN_CODE_H */
