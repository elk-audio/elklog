/**
 * @brief Example of how to use elklog through a static instance using logging macros
 */

#include <string>
#include <iostream>
#include <thread>

#include "elklog/static_logger.h"

constexpr std::chrono::milliseconds SLEEP_TIME(100);

ELKLOG_GET_LOGGER_WITH_MODULE_NAME("mod_1");

int log_thread()
{
    std::this_thread::sleep_for(SLEEP_TIME);

    ELKLOG_LOG_INFO("Logging from another thread ");
    for (int i = 1; i < 4 ; ++i)
    {
        ELKLOG_LOG_WARNING("{}...", i);
        ELKLOG_LOG_WARNING_IF(i == 2, "Counter = 2");
        std::this_thread::sleep_for(SLEEP_TIME);
    }
    return 0;
}

int main()
{
    std::string log_level = "info";
    std::string log_filename = "log.txt";
    std::string log_name = "Direct logger example";
    std::chrono::seconds log_flush_interval(1);

    // Logger configuration
    auto res = elklog::StaticLogger::init_logger(log_filename, log_name, log_level, log_flush_interval);

    if (res != elklog::Status::OK)
    {
        std::cout << "Failed to initialize logger: " << res << std::endl;
        return -1;
    }

    // Start a new thread to log from
    std::thread thread(log_thread);

    // Log some data in this thread too
    ELKLOG_LOG_INFO("Logging something from the main thread:");
    for (int i = 1; i < 4 ; ++i)
    {
        ELKLOG_LOG_INFO("{}...", i);
        std::this_thread::sleep_for(SLEEP_TIME);
    }

    thread.join();

    return 0;
}

