/**
 * @brief RT-logger levels
 *
 * @copyright Copyright 2017-2023 Elk AB, Stockholm
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
    DEBUG
};

inline std::ostream& operator << (std::ostream& o, const RtLogLevel& l)
{
    switch (l)
    {
    case RtLogLevel::DEBUG:
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


} // namespace aloha


#endif /* RTLOGLEVEL_H */
