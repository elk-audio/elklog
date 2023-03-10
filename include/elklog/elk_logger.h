/**
 * @brief Wrapper around Logging library to log multiple plugin instances to a single
 *        file and a marker for the instance in the log-line.
 *
 *        Provides a unified logger abstraction that is safe to call in any context
 *        (either RT or non-RT).
 *
 * @copyright Modern Ancient Instruments Networked AB, dba Elk
 */

#ifndef ALOHA_LOGGER_H
#define ALOHA_LOGGER_H

#include <memory>
#include <string>
#include <map>
#include <algorithm>
#include <variant>
#include <iomanip>

#include <future>

#include "log_return_code.h"

#ifndef ALOHA_DISABLE_LOGGING
#include "spdlog/spdlog.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"
#include "spdlog/sinks/rotating_file_sink.h"
#pragma GCC diagnostic pop

#include "spdlog/async.h"
#include "spdlog/spdlog.h"

#include "rtlogger.h"

namespace elklog {

constexpr int RTLOG_MESSAGE_SIZE = 2048;
constexpr int RTLOG_QUEUE_SIZE = 1024;
constexpr int MAX_LOG_FILE_SIZE = 10'000'000;   // In bytes

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
    ElkLogger(const std::string& min_log_level,
              Type logger_type = Type::TEXT) :
             _min_log_level(min_log_level),
             _type(logger_type)
    {
        _rt_logger = std::make_unique<RtLogger<RTLOG_MESSAGE_SIZE, RTLOG_QUEUE_SIZE>>(50,
                std::bind(&ElkLogger::_rt_logger_callback, this, std::placeholders::_1),
                min_log_level);
    }

    virtual ~ElkLogger()
    {
        close_log();

        spdlog::drop(_logger_instance->name());

        _closed_promise.set_value(_closed);
    }

    /**
     * @brief Initialize logger
     *
     * @param log_file_path Log file (should be unique for each instance)
     *
     * @return Error code. LogErrorCode::OK if all good.
     *         Can be passed straight to << stream operators
     *         for a human-readable output error string.
     */
    LogErrorCode initialize(const std::string& log_file_path,
                            const std::string& logger_name = "\"elk_logger",
                            std::chrono::seconds flush_interval = std::chrono::seconds(0),
                            bool drop_logger_if_duplicate = false,
                            int max_files = 1)
    {
        std::map<std::string, spdlog::level::level_enum> level_map;
        level_map["debug"] = spdlog::level::debug;
        level_map["info"] = spdlog::level::info;
        level_map["warning"] = spdlog::level::warn;
        level_map["error"] = spdlog::level::err;
        level_map["critical"] = spdlog::level::critical;

        _log_file_path = log_file_path;

        std::string log_level_lowercase = _min_log_level;
        std::transform(_min_log_level.begin(), _min_log_level.end(), log_level_lowercase.begin(), ::tolower);

        if (flush_interval.count() > 0)
        {
            spdlog::flush_every(std::chrono::seconds(flush_interval));
        }

        if (level_map.count(log_level_lowercase) <= 0)
        {
            return LogErrorCode::INVALID_LOG_LEVEL;
        }
        auto log_level = level_map[log_level_lowercase];
        spdlog::set_level(log_level);
        spdlog::flush_on(log_level);

        if (drop_logger_if_duplicate)
        {
            auto possible_logger = spdlog::get(logger_name);

            if (possible_logger.get() != nullptr)
            {
                spdlog::drop(logger_name);
            }
        }

        _logger_instance = spdlog::rotating_logger_mt<spdlog::async_factory>(logger_name,
                                                                             log_file_path,
                                                                             MAX_LOG_FILE_SIZE,
                                                                             max_files,
                                                                             false // Rotate on open: false
                                                                             );

        if (_logger_instance == nullptr)
        {
            return LogErrorCode::FAILED_TO_START_LOGGER;
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

        return LogErrorCode::OK;
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
        case RtLogLevel::DEBUG:
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
    friend class Logger;
};

} // namespace aloha

#else // ALOHA_DISABLE_LOGGING

namespace aloha {

class AlohaLogger
{
public:
    AlohaLogger(int /*instance_counter*/,
                const std::string& /*min_log_level*/)
    {}

    virtual ~AlohaLogger() = default;

    LogErrorCode initialize(const std::string& /*log_file_path*/,
                            const std::string& /*logger_name*/ = "\"aloha_logger_{0}\"",
                            bool /*drop_if_duplicate*/ = false,
                            int /*max_files*/ = 1)
    {
        return LogErrorCode::OK;
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
};

} // namespace aloha

#endif // ALOHA_DISABLE_LOGGING

#endif /* ALOHA_LOGGER_H */
