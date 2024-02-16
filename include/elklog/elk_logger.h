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
 * @brief Wrapper around Logging library to log multiple plugin instances to a single
 *        file and a marker for the instance in the log-line.
 *
 *        Provides a unified logger abstraction that is safe to call in any context
 *        (either RT or non-RT).
 *
 * @copyright Copyright 2020-2023 Elk Audio AB, Stockholm
 */

#ifndef ELK_LOGGER_H
#define ELK_LOGGER_H

#include <memory>
#include <string>
#include <map>
#include <algorithm>
#include <variant>
#include <iomanip>

#include <future>

#include "log_return_code.h"

#ifndef ELKLOG_DISABLE_LOGGING
#include "spdlog/spdlog.h"

#include "elk-warning-suppressor/warning_suppressor.hpp"

ELK_PUSH_WARNING
ELK_DISABLE_REORDER
#include "spdlog/sinks/rotating_file_sink.h"
ELK_POP_WARNING

#include "spdlog/async.h"
#include "spdlog/spdlog.h"

#include "rtlogger.h"

namespace elklog {

constexpr int RTLOG_MESSAGE_SIZE = ELKLOG_RT_MESSAGE_SIZE;
constexpr int RTLOG_QUEUE_SIZE = ELKLOG_RT_QUEUE_SIZE;
constexpr int MAX_LOG_FILE_SIZE = ELKLOG_FILE_SIZE;   // In bytes
constexpr auto RT_CONSUMER_POLL_PERIOD = std::chrono::milliseconds(50);

class ElkLogger
{
public:
    enum class Type
    {
        TEXT,
        JSON
    };

    /**
     * @brief Create a logger instance
     *        Most of the relevant initialization is actually done in
     *        initialize() which should be called after and check for
     *        error codes.
     *
     * @param min_log_level Minimum logging level (debug, info, warning, error)
     * @param logger_type Choose between TYPE::TEXT (default), and JSON.
     */
    explicit ElkLogger(const std::string& min_log_level,
                       Type logger_type = Type::TEXT) :
                       _min_log_level(min_log_level),
                       _type(logger_type)
    {
        _rt_logger = std::make_unique<RtLogger<RTLOG_MESSAGE_SIZE, RTLOG_QUEUE_SIZE>>(RT_CONSUMER_POLL_PERIOD,
                std::bind(&ElkLogger::_rt_logger_callback, this, std::placeholders::_1),
                min_log_level);
    }

    virtual ~ElkLogger()
    {
        if (_logger_instance)
        {
            close_log();
            spdlog::drop(_logger_instance->name());
        }

        _closed_promise.set_value(_closed);
    }

    /**
     * @brief Initialize logger
     *
     * @param log_file_path Log file (should be unique for each instance)
     *
     * @return Error code. elklog::Status::OK if all good.
     *         Can be passed straight to << stream operators
     *         for a human-readable output error string.
     */
    Status initialize(const std::string& log_file_path,
                      const std::string& logger_name,
                      std::chrono::seconds flush_interval = std::chrono::seconds(0),
                      bool drop_logger_if_duplicate = false,
                      int max_files = 1)
    {
        _log_file_path = log_file_path;
        
        Status status = set_log_level(_min_log_level);
        
        if (status != Status::OK)
        {
            return status;
        }
        
        if (flush_interval.count() > 0)
        {
            spdlog::flush_every(std::chrono::seconds(flush_interval));
        }
        
        spdlog::flush_on(spdlog::level::err);

        // Check for already registered logger
        auto possible_logger = spdlog::get(logger_name);

        if (drop_logger_if_duplicate)
        {
            if (spdlog::get(logger_name))
            {
                spdlog::drop(logger_name);
            }
        }

        try
        {
            _logger_instance = spdlog::rotating_logger_mt<spdlog::async_factory>(logger_name,
                                                                                 log_file_path,
                                                                                 MAX_LOG_FILE_SIZE,
                                                                                 max_files,
                                                                                 false);
        }
        catch (const std::exception &ex)
        {
            return Status::FAILED_TO_START_LOGGER;
        }

        if (_logger_instance == nullptr)
        {
            return Status::FAILED_TO_START_LOGGER;
        }

        if (_type == Type::JSON)
        {
            // We have some extra formatting on the log level %l below,
            // to keep color coding when dumping json to the console,
            // and we use a full ISO 8601 time/date format.

            // "time" here is human-readable - there's another raw timestamp in the payload from wrappers.
            _logger_instance->set_pattern(
                "{\"time\": \"%Y-%m-%dT%H:%M:%S.%e%z\", "
                "\"name\": \"%n\", "
                "\"level\": \"%^%l%$\", "
                "\"process\": %P, "
                "\"thread\": %t, "
                "\"data\": %v}");

            _logger_instance->info("{}", R"({ "status": "Started" })");
        }
        else
        {
            _logger_instance->set_pattern("[%Y-%m-%d %T.%e] [%l] %v");
            _logger_instance->info("Started logger: {}.", logger_name);
        }

        return Status::OK;
    }
    
    Status set_log_level(const std::string& min_log_level)
    {
        _rt_logger->set_log_level(min_log_level);
        
        std::map<std::string, spdlog::level::level_enum> level_map;
        level_map["debug"] = spdlog::level::debug;
        level_map["info"] = spdlog::level::info;
        level_map["warning"] = spdlog::level::warn;
        level_map["error"] = spdlog::level::err;
        level_map["critical"] = spdlog::level::critical;
        
        _min_log_level = min_log_level;
        
        std::string log_level_lowercase = _min_log_level;
        std::transform(_min_log_level.begin(), _min_log_level.end(), log_level_lowercase.begin(), ::tolower);

        if (level_map.count(log_level_lowercase) <= 0)
        {
            return Status::INVALID_LOG_LEVEL;
        }
        
        auto log_level = level_map[log_level_lowercase];
        spdlog::set_level(log_level);
        
        return Status::OK;
    }

    template<typename... Args>
    void debug(const char* format_str, Args&&... args)
    {
        if (_closed == true) return;

        if (twine::is_current_thread_realtime())
        {
            _rt_logger->log_debug(format_str, args...);
        }
        else
        {
            _logger_instance->debug(format_str, args...);
        }
    }

    template<typename... Args>
    void info(const char* format_str, Args&&... args)
    {
        if (_closed == true) return;

        if (twine::is_current_thread_realtime())
        {
            _rt_logger->log_info(format_str, args...);
        }
        else
        {
            _logger_instance->info(format_str, args...);
        }
    }

    template<typename... Args>
    void warning(const char* format_str, Args&&... args)
    {
        if (_closed == true) return;

        if (twine::is_current_thread_realtime())
        {
            _rt_logger->log_warning(format_str, args...);
        }
        else
        {
            _logger_instance->warn(format_str, args...);
        }
    }

    template<typename... Args>
    void error(const char* format_str, Args&&... args)
    {
        if (_closed == true) return;

        if (twine::is_current_thread_realtime())
        {
            _rt_logger->log_error(format_str, args...);
        }
        else
        {
            _logger_instance->error(format_str, args...);
        }
    }

    template<typename... Args>
    void critical(const char* format_str, Args&&... args)
    {
        if (_closed == true) return;

        if (twine::is_current_thread_realtime())
        {
            _rt_logger->log_error(format_str, args...);
        }
        else
        {
            _logger_instance->critical(format_str, args...);
        }
    }

    void close_log()
    {
        if (_type != Type::JSON)
        {
            _logger_instance->flush();
            _closed = true;
            return;
        }

        if (_closed == true) return;

        _logger_instance->flush();

        // We have some extra formatting on the log level %l below,
        // to keep color coding when dumping json to the console,
        // and we use a full ISO 8601 time/date format.
        _logger_instance->set_pattern(
            "{\"time\": \"%Y-%m-%dT%H:%M:%S.%e%z\", "
            "\"name\": \"%n\", "
            "\"level\": \"%^%l%$\", "
            "\"process\": %P, "
            "\"thread\": %t, "
            "\"data\": %v}");

        // Below is our last log entry.
        _logger_instance->info(R"({ "status": "Finished" })");
        _logger_instance->flush();

        _closed = true;
    }

    const std::string& min_log_level()
    {
        return _min_log_level;
    }

    const std::string& log_file_path()
    {
        return _log_file_path;
    }

    std::promise<bool>& closed_promise()
    {
        return _closed_promise;
    }

private:
    void _rt_logger_callback(const RtLogMessage<RTLOG_MESSAGE_SIZE>& msg)
    {
        if (_closed) return;

        switch (msg.level())
        {
        case RtLogLevel::DBG:
            _logger_instance->debug("{}", msg.message());
            break;

        case RtLogLevel::INFO:
            _logger_instance->info("{}", msg.message());
            break;

        case RtLogLevel::WARNING:
            _logger_instance->warn("{}", msg.message());
            break;

        case RtLogLevel::ERROR:
            _logger_instance->error("{}", msg.message());
            break;
        }
    }

    std::string _min_log_level;
    std::string _log_file_path;
    std::shared_ptr<spdlog::logger> _logger_instance;
    std::unique_ptr<RtLogger<RTLOG_MESSAGE_SIZE, RTLOG_QUEUE_SIZE>> _rt_logger {nullptr};

    Type _type {Type::TEXT};
    bool _closed {false};

    std::promise<bool> _closed_promise;
};

} // namespace elklog

#else // ELKLOG_DISABLE_LOGGING

namespace elklog {

class ElkLogger
{
public:
    enum class Type
    {
        TEXT,
        JSON
    };

    explicit ElkLogger([[maybe_unused]] const std::string& min_log_level,
                       [[maybe_unused]] Type logger_type = Type::TEXT)
    {}

    virtual ~ElkLogger()
    {
        _closed_promise.set_value(_closed);
    }

    Status initialize([[maybe_unused]] const std::string& log_file_path,
                      [[maybe_unused]] const std::string& logger_name = "\"elk_logger",
                      [[maybe_unused]] std::chrono::seconds flush_interval = std::chrono::seconds(0),
                      [[maybe_unused]] bool drop_logger_if_duplicate = false,
                      [[maybe_unused]] int max_files = 1)
    {
        return Status::OK;
    }
    
    Status set_log_level([[maybe_unused]] const std::string& min_log_level)
    {
        return Status::OK;
    }

    template<typename... Args>
    void debug(const char* /*format_str*/, Args&&... /*args*/)
    {}

    template<typename... Args>
    void info(const char* /*format_str*/, Args&&... /*args*/)
    {}

    template<typename... Args>
    void warning(const char* /*format_str*/, Args&&... /*args*/)
    {}

    template<typename... Args>
    void error(const char* /*format_str*/, Args&&... /*args*/)
    {}

    void close_log()
    {
        _closed = true;
    }

    const std::string& min_log_level()
    {
        return _dummy;
    }

    const std::string& log_file_path()
    {
        return _dummy;
    }

    std::promise<bool>& closed_promise()
    {
        return _closed_promise;
    }

    std::string _dummy;
    bool _closed{false};
    std::promise<bool> _closed_promise;
};

} // namespace elklog

#endif // ELKLOG_DISABLE_LOGGING

#endif /* ELK_LOGGER_H */
