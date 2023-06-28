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
 * @brief Static wrapper around elklog for applications with only a single logger
 * instance. This removes the need to pass around a given logger object.
 *
 * Logging is done via macros that also enable logging to be easily removed at
 * compile time. Useful for for example unit testing code.
 *
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
 * @copyright Copyright 2020-2023 Elk Audio AB, Stockholm
 */

#ifndef STATIC_LOGGER_H
#define STATIC_LOGGER_H

#include "elk_logger.h"

/* log macros */
#ifndef ELKLOG_DISABLE_LOGGING
//#include "spdlog/spdlog.h"

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
#define ELKLOG_LOG_DEBUG(msg, ...) elklog::Logger::public_instance->debug("{}" msg " - [@{} #{}]", ##__VA_ARGS__, __FILE__ , __LINE__)
#else
#define ELKLOG_LOG_DEBUG(msg, ...)         elklog::StaticLogger::public_instance->debug("{}" msg, local_log_prefix, ##__VA_ARGS__)
#endif
#define ELKLOG_LOG_INFO(msg, ...)          elklog::StaticLogger::public_instance->info("{}" msg, local_log_prefix, ##__VA_ARGS__)
#define ELKLOG_LOG_WARNING(msg, ...)       elklog::StaticLogger::public_instance->warning("{}" msg, local_log_prefix, ##__VA_ARGS__)
#define ELKLOG_LOG_ERROR(msg, ...)         elklog::StaticLogger::public_instance->error("{}" msg, local_log_prefix, ##__VA_ARGS__)
#define ELKLOG_LOG_CRITICAL(msg, ...)      elklog::StaticLogger::public_instance->critical("{}" msg, local_log_prefix, ##__VA_ARGS__)

#ifdef ELKLOG_ENABLE_DEBUG_FILE_AND_LINE_NUM
#define ELKLOG_LOG_DEBUG_IF(condition, msg, ...) if (condition) { elklog::StaticLogger::public_instance->debug_if(condition, "{}" msg " - [@{} #{}]", ##__VA_ARGS__, __FILE__ , __LINE__); }
#else
#define ELKLOG_LOG_DEBUG_IF(condition, msg, ...)    if (condition) { elklog::StaticLogger::public_instance->debug(condition, "{}" msg, local_log_prefix, ##__VA_ARGS__); }
#endif
#define ELKLOG_LOG_INFO_IF(condition, msg, ...)     if (condition) { elklog::StaticLogger::public_instance->info("{}" msg, local_log_prefix, ##__VA_ARGS__); }
#define ELKLOG_LOG_WARNING_IF(condition, msg, ...)  if (condition) { elklog::StaticLogger::public_instance->warning("{}" msg, local_log_prefix, ##__VA_ARGS__); }
#define ELKLOG_LOG_ERROR_IF(condition, msg, ...)    if (condition) { elklog::StaticLogger::public_instance->error("{}" msg, local_log_prefix, ##__VA_ARGS__); }
#define ELKLOG_LOG_CRITICAL_IF(condition, msg, ...) if (condition) { elklog::StaticLogger::public_instance->critical("{}" msg, local_log_prefix, ##__VA_ARGS__); }

namespace elklog {

class StaticLogger
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
     * @param flush_interval Forces a flush every n seconds, no periodic flush if set to 0
     * @param drop_logger_if_duplicate If another logger instance is active with the same name, drop
     *                                 that before creating a new one
     *
     * @returns Error code, use ELKLOG_LOG_ERROR_MESSAGES to retrieve the message
     *
     */
    static elklog::Status init_logger(const std::string& file_name,
                                      const std::string& logger_name,
                                      const std::string& min_log_level,
                                      std::chrono::seconds log_flush_interval = std::chrono::seconds(0),
                                      ElkLogger::Type logger_type = ElkLogger::Type::TEXT,
                                      bool drop_logger_if_duplicate = false,
                                      int max_files = 1)
    {
        /* Make one call to spdlog to make avoid the static initialization
         * order problem and ensure sure that static spdlog internals are
         * initialized before the logger instance is created, and destroyed
         * after the logger is destroyed.
         */
        [[maybe_unused]] auto level = spdlog::get_level();
        static std::shared_ptr<ElkLogger> internal_instance;
        internal_instance = std::make_shared<ElkLogger>(min_log_level, logger_type);
        auto res = internal_instance->initialize(file_name, logger_name, log_flush_interval, drop_logger_if_duplicate, max_files);

        if (res == Status::OK)
        {
            public_instance = internal_instance.get();
        }
        return res;
    }

    static ElkLogger* public_instance;

private:
    StaticLogger() = default;
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

class StaticLogger
{
public:
    static elklog::Status init_logger([[maybe_unused]] const std::string& file_name,
                                      [[maybe_unused]] const std::string& logger_name,
                                      [[maybe_unused]] const std::string& min_log_level,
                                      [[maybe_unused]] const std::chrono::seconds log_flush_interval,
                                      [[maybe_unused]] bool drop_logger_if_duplicate = false,
                                      [[maybe_unused]] int max_files = 1)
    {
        return Status::OK;
    }
};
}
#endif

#endif //STATIC_LOGGER_H
