#include "logging.h"
#ifndef DISABLE_LOGGING

namespace mind {

// Static variable need to be initialized here
// so that the linker can find them.
//
// See:
// http://stackoverflow.com/questions/14808864/can-i-initialize-static-float-variable-during-runtime
// answer from Edward A.

std::string Logger::_logger_file_name = "log";
std::string Logger::_logger_name = "Logger";
spdlog::level::level_enum Logger::_min_log_level = spdlog::level::warn;


void Logger::set_logger_params(const std::string file_name,
                               const std::string logger_name,
                               const spdlog::level::level_enum min_log_level)
{
    Logger::_logger_file_name.assign(file_name);
    Logger::_logger_name.assign(logger_name);
    Logger::_min_log_level = min_log_level;
}

std::shared_ptr<spdlog::logger> Logger::get_logger()
{
    /*
     * Note: A static function variable avoids all initialization
     * order issues associated with a static member variable
     */
    static auto spdlog_instance = setup_logging();
    if (!spdlog_instance)
    {
        std::cerr << "Error, logger is not initialized properly!!! " << std::endl;
    }
    return spdlog_instance;
}

std::shared_ptr<spdlog::logger> setup_logging()
{
    /*
     * Note, configuration parameters are defined here to guarantee
     * that they are defined before calling get_logger()
     */
    const size_t LOGGER_QUEUE_SIZE  = 4096;                  // Should be power of 2
    const int  MAX_LOG_FILE_SIZE    = 10000000;              // In bytes
    const bool LOGGER_FORCE_FLUSH   = true;

    spdlog::set_level(Logger::min_log_level());
    spdlog::set_async_mode(LOGGER_QUEUE_SIZE);
    auto async_file_logger = spdlog::rotating_logger_mt(Logger::logger_name(),
                                                        Logger::logger_file_name(),
                                                        MAX_LOG_FILE_SIZE,
                                                        1,
                                                        LOGGER_FORCE_FLUSH);

    async_file_logger->warn("#############################");
    async_file_logger->warn("   Started Mind Logger!");
    async_file_logger->warn("#############################");
    return async_file_logger;
}

} // end namespace mind

#endif // DISABLE_LOGGING
