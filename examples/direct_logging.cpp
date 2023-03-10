/**
 * @brief Example of how to use the elklogger directly
 */

#include <string>
#include <iostream>

#include "elklog/elk_logger.h"

constexpr std::chrono::milliseconds SLEEP_TIME(100);

int main()
{
    std::string log_level = "info";
    std::string log_filename = "log.txt";
    std::string log_name = "Direct logger example";
    std::chrono::seconds log_flush_interval(1);

    // Logger configuration
    auto logger = elklog::ElkLogger("info");

    auto res = logger.initialize(log_filename, log_name, log_flush_interval);
    if (res != elklog::LogErrorCode::OK)
    {
        std::cout << "Failed to initialize logger: " << res << std::endl;
        return -1;
    }

    // Log some data
    logger.info("Logging something: ");
    for (int i = 1; i < 4 ; ++i)
    {
        std::this_thread::sleep_for(SLEEP_TIME);
        logger.warning("{}...", i);
    }

    return 0;
}

