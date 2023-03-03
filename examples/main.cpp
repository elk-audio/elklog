/**
 * @brief Template for C++ program with command-line arguments and
 *        spd logger.
 *        Grep for
 *              projecttemplate
 *        and
 *              project_template
 *         (case insensitive)
 *        and rename the occurences.
 * @version 0.0
 */

#include <vector>
#include <fstream>
#include <iostream>

#include "elklog/elk_logger.h"


int main()
{

    std::string log_level = "info";
    std::string log_filename = "log.txt";

    ////////////////////////////////////////////////////////////////////////////////
    // Logger configuration
    ////////////////////////////////////////////////////////////////////////////////
    auto logger = elklog::ElkLogger(1, "info");

    auto res = logger.initialize("./log.txt", "example_logger");
    if (res != elklog::LogErrorCode::OK)
    {
        std::cout << "Failed to initialize logger" << std::endl;
        return -1;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Main body
    ////////////////////////////////////////////////////////////////////////////////

    logger.info("Logging something: ");
    for (int i = 1; i < 4 ; ++i)
    {
        logger.warning("{}...", i);
    }

    return 0;
}

