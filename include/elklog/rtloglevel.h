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
 * @brief RT-logger levels
 *
 * @copyright Copyright 2020-2023 Elk Audio AB, Stockholm
 */

#ifndef RTLOGLEVEL_H
#define RTLOGLEVEL_H

#include <ostream>

namespace elklog {

enum class RtLogLevel : int
{
    ERROR,
    WARNING,
    INFO,
    DBG
};

inline std::ostream& operator << (std::ostream& o, const RtLogLevel& l)
{
    switch (l)
    {
    case RtLogLevel::DBG:
        o << "[debug]";
        break;

    case RtLogLevel::INFO:
        o << "[info]";
        break;

    case RtLogLevel::WARNING:
        o << "[warning]";
        break;

    case RtLogLevel::ERROR:
        o << "[error]";
        break;
    }
    return o;
}


} // namespace elklog


#endif /* RTLOGLEVEL_H */
