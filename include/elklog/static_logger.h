/*
 * Copyright 2017-2023 Elk AB, Stockholm
 *
 * SUSHI is free software: you can redistribute it and/or modify it under the terms of
 * the GNU Affero General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * SUSHI is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with
 * SUSHI.  If not, see http://www.gnu.org/licenses/
 */

/**
 * @brief Static wrapper around elklog for applications where only a single logger
 * instance. This removes the need to pass around a given logger object.
 *
 * Logging is done via macros that also enable logging to be easily removed at
 * compile time. Useful for for example unit testing code.
 * *
 * If -DELKLOG_DISABLE_LOGGING is passed as a definition, all logging code
 * disappears without a trace. Useful for testing and outside releases.
 *
 * Usage:
 * Call ELKLOG_GET_LOGGER before any code in each file or preferably call
 * ELKLOG_GET_LOGGER_WITH_MODULE_NAME() to set a module name that will be
 * used by all log entries from that file.
 *
 * Write to the logger using the ELKLOG_LOG_XXX macros with cppformat style
 * ie: ELKLOG_LOG_INFO("Setting x to {} and y to {}", x, y);
 *
 * spdlog supports ostream style too, but that doesn't work with
 * ELKLOG_DISABLE_LOGGING unfortunately
 *
 * @copyright Copyright 2017-2023 Elk AB, Stockholm
 */

#ifndef STATIC_LOGGER_H
#define STATIC_LOGGER_H

#include "elk_logger.h"

/** Error codes returned by set_logger_params
 */
enum ELKLOG_LOG_ERROR_CODE
{
    ELKLOG_LOG_ERROR_CODE_OK = 0,
    ELKLOG_LOG_ERROR_CODE_INVALID_LOG_LEVEL = 1,
    ELKLOG_LOG_FAILED_TO_START_LOGGER = 2,
    ELKLOG_LOG_ERROR_CODE_INVALID_FLUSH_INTERVAL = 3
};

#define SPDLOG_DEBUG_ON // TODO - needed?

/* log macros */
#ifndef ELKLOG_DISABLE_LOGGING
#include "spdlog/spdlog.h"

/* Add file and line numbers to debug prints, disabled by default */
//#define ELKLOG_ENABLE_DEBUG_FILE_AND_LINE_NUM

/* Use this macro  at the top of a source file to declare a local logger */
#define ELKLOG_GET_LOGGER_WITH_MODULE_NAME(prefix) constexpr char local_log_prefix[] = "[" prefix "] "

#define ELKLOG_GET_LOGGER constexpr char local_log_prefix[] = ""

/*
 * Use these macros to log messages. Use cppformat style, ie:
 * ELKLOG_LOG_INFO("Setting x to {} and y to {}", x, y);
 *
 * spdlog supports ostream style, but that doesn't work with
 * -DDISABLE_MACROS unfortunately
 */
#ifdef ELKLOG_ENABLE_DEBUG_FILE_AND_LINE_NUM
#define ELKLOG_LOG_DEBUG(msg, ...) spdlog_instance->debug("{}" msg " - [@{} #{}]", ##__VA_ARGS__, __FILE__ , __LINE__)
#else
#define ELKLOG_LOG_DEBUG(msg, ...)         elk::Logger::logger_instance->debug("{}" msg, local_log_prefix, ##__VA_ARGS__)
#endif
#define ELKLOG_LOG_INFO(msg, ...)          elk::Logger::logger_instance->info("{}" msg, local_log_prefix, ##__VA_ARGS__)
#define ELKLOG_LOG_WARNING(msg, ...)       elk::Logger::logger_instance->warn("{}" msg, local_log_prefix, ##__VA_ARGS__)
#define ELKLOG_LOG_ERROR(msg, ...)         elk::Logger::logger_instance->error("{}" msg, local_log_prefix, ##__VA_ARGS__)
#define ELKLOG_LOG_CRITICAL(msg, ...)      elk::Logger::logger_instance->critical("{}" msg, local_log_prefix, ##__VA_ARGS__)

#ifdef ELKLOG_ENABLE_DEBUG_FILE_AND_LINE_NUM
#define ELKLOG_LOG_DEBUG_IF(condition, msg, ...) if (condition) { elk::Logger::logger_instance->debug_if(condition, "{}" msg " - [@{} #{}]", ##__VA_ARGS__, __FILE__ , __LINE__); }
#else
#define ELKLOG_LOG_DEBUG_IF(condition, msg, ...)    if (condition) { elk::Logger::logger_instance->debug(condition, "{}" msg, local_log_prefix, ##__VA_ARGS__); }
#endif
#define ELKLOG_LOG_INFO_IF(condition, msg, ...)     if (condition) { elk::Logger::logger_instance->info("{}" msg, local_log_prefix, ##__VA_ARGS__); }
#define ELKLOG_LOG_WARNING_IF(condition, msg, ...)  if (condition) { elk::Logger::logger_instance->warn("{}" msg, local_log_prefix, ##__VA_ARGS__); }
#define ELKLOG_LOG_ERROR_IF(condition, msg, ...)    if (condition) { elk::Logger::logger_instance->error("{}" msg, local_log_prefix, ##__VA_ARGS__); }
#define ELKLOG_LOG_CRITICAL_IF(condition, msg, ...) if (condition) { elk::Logger::logger_instance->critical("{}" msg, local_log_prefix, ##__VA_ARGS__); }

namespace elklog {

class Logger
{
public:
    /**
     * @brief Configure logger parameters. This must be called before instantiating
     *        any object that use ELKLOG_GET_LOGGER, e.g. at beginning of main.
     *
     * @param file_name Name of file used to append logs (without extension)
     * @param logger_name Internal name of the logger
     * @param min_log_level Minimum logging level, one of ('debug', 'info', 'warning', 'error', 'critical')
     * @param logger_type Choose between TYPE::TEXT (default), and JSON.
     * @param flush_interval Forces a flush every n seconds, no flush if set to 0
     * @param drop_logger_if_duplicate If another logger instance
     *
     * @returns Error code, use ELKLOG_LOG_ERROR_MESSAGES to retrieve the message
     *
     */
    static elklog::LogErrorCode init_logger(const std::string& file_name,
                                            const std::string& logger_name,
                                            const std::string& min_log_level,
                                            ElkLogger::TYPE logger_type = ElkLogger::TYPE::TEXT,
                                            const std::chrono::seconds log_flush_interval = std::chrono::seconds(0),
                                            bool drop_logger_if_duplicate = false,
                                            int max_files = 1)
    {
        logger_instance = std::make_shared<ElkLogger>(min_log_level, logger_type);
        return logger_instance->initialize(file_name, logger_name, log_flush_interval, drop_logger_if_duplicate, max_files);
    }

    static std::shared_ptr<ElkLogger> logger_instance;

private:
    Logger() = default;
};

}

#else
/* Define empty macros */
#define ELKLOG_GET_LOGGER
#define ELKLOG_GET_LOGGER_WITH_MODULE_NAME(...)
#define ELKLOG_LOG_DEBUG(...)
#define ELKLOG_LOG_INFO(...)
#define ELKLOG_LOG_WARNING(...)
#define ELKLOG_LOG_ERROR(...)
#define ELKLOG_LOG_CRITICAL(...)
#define ELKLOG_LOG_DEBUG_IF(...)
#define ELKLOG_LOG_INFO_IF(...)
#define ELKLOG_LOG_WARNING_IF(...)
#define ELKLOG_LOG_ERROR_IF(...)
#define ELKLOG_LOG_CRITICAL_IF(...)

namespace elklog {

class Logger
{
public:
    static elklog::LogErrorCode init_logger([[maybe_unused]] const std::string& file_name,
                                            [[maybe_unused]] const std::string& logger_name,
                                            [[maybe_unused]] const std::string& min_log_level,
                                            [[maybe_unused]] const std::chrono::seconds log_flush_interval,
                                            [[maybe_unused]] bool drop_logger_if_duplicate = false,
                                            [[maybe_unused]] int max_files = 1))
    {
        return LogErrocCode::OK;
    }

private:
    Logger() = default;
};

#endif

#endif //STATIC_LOGGER_H
