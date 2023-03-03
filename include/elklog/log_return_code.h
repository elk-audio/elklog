/**
 * @brief Log initialization return codes
 *
 * @copyright Modern Ancient Instruments Networked AB, dba Elk
 */

#ifndef LOG_RETURN_CODE_H
#define LOG_RETURN_CODE_H

#include <ostream>

namespace aloha {

enum class LogErrorCode : int
{
    OK = 0,
    INVALID_LOG_LEVEL = 1,
    FAILED_TO_START_LOGGER = 2,
    INVALID_FLUSH_INTERVAL = 3
};

inline std::ostream& operator << (std::ostream& o, const LogErrorCode& c)
{
    switch (c)
    {
    case LogErrorCode::OK:
        o << "Ok";
        break;

    case LogErrorCode::INVALID_LOG_LEVEL:
        o << "Invalid log level";
        break;

    case LogErrorCode::INVALID_FLUSH_INTERVAL:
        o << "Invalid Flush interval";
        break;

    case LogErrorCode::FAILED_TO_START_LOGGER:
        o << "Failed to initialize SPD logger instance";
        break;
    }

    return o;
}

} // namespace aloha


#endif /* LOG_RETURN_CODE_H */
