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

#include "logging.h"
#include "options.h"

#include "public_header.h"

using namespace mind;


int main(int argc, char* argv[])
{
    ////////////////////////////////////////////////////////////////////////////////
    // Command Line arguments parsing
    ////////////////////////////////////////////////////////////////////////////////

    // option_parser accepts arguments excluding program name,
    // so skip it if it is present
    if (argc > 0)
    {
        argc--;
        argv++;
    }

    option::Stats cl_stats(usage, argc, argv);
    std::vector<option::Option> cl_options(cl_stats.options_max);
    std::vector<option::Option> cl_buffer(cl_stats.buffer_max);
    option::Parser cl_parser(usage, argc, argv, &cl_options[0], &cl_buffer[0]);

    if (cl_parser.error())
    {
        return 1;
    }
    if (cl_parser.nonOptionsCount() > 0)
    {
        fprintf(stderr, "Unexpected non-optional argument %s\n", cl_parser.nonOption(0));
        return 1;
    }
    if (cl_options[OPT_IDX_HELP])
    {
        option::printUsage(fwrite, stdout, usage);
        return 0;
    }

    int project_template_numeric_arg = PROJECT_TEMPLATE_NUMERIC_ARG_DEFAULT;
    std::string project_template_string_arg = std::string(PROJECT_TEMPLATE_STRING_ARG_DEFAULT);
    for (int i=0; i<cl_parser.optionsCount(); i++)
    {
        option::Option& opt = cl_buffer[i];
        switch(opt.index())
        {
        case OPT_IDX_HELP:
        case OPT_IDX_UNKNOWN:
            // should be handled before arriving here
            assert(false);
            break;

        case OPT_IDX_PROJECT_TEMPLATE_NUMERIC_ARG:
            {
                int parsed_int = atoi(opt.arg);
                // horrible, but that's how atoi works and std::stoi needs exceptions
                if (parsed_int == 0)
                {
                    ProjectTemplateArg::print_error("Option '", opt, "' invalid number\n");
                    return 1;
                }
                project_template_numeric_arg = parsed_int;
            }
            break;

        case OPT_IDX_PROJECT_TEMPLATE_STRING_ARG:
            project_template_string_arg.assign(opt.arg);
            break;

        default:
            ProjectTemplateArg::print_error("Unhandled option '", opt, "' \n");
            break;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Logger configuration
    ////////////////////////////////////////////////////////////////////////////////
    MIND_LOG_SET_PARAMS("template_log", "Logger", MIND_LOG_LEVEL_DEBUG);
    MIND_GET_LOGGER;

    ////////////////////////////////////////////////////////////////////////////////
    // Main body
    ////////////////////////////////////////////////////////////////////////////////

    std::cout << "Starting with arguments:" << std::endl;
    std::cout << "Numeric: " << project_template_numeric_arg << std::endl;
    std::cout << "String: " << project_template_string_arg << std::endl;

    // Log something
    MIND_LOG_INFO("I'm logging at INFO level");
    MIND_LOG_DEBUG("I'm logging at DEBUG level");

    return 0;
}

