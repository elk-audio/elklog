#include <cstdio> 
#include "optionparser.h"

////////////////////////////////////////////////////////////////////////////////
// Options Defaults
////////////////////////////////////////////////////////////////////////////////

#define PROJECT_TEMPLATE_LOG_LEVEL_DEFAULT "info"
#define PROJECT_TEMPLATE_LOG_FILENAME_DEFAULT "log"

#define PROJECT_TEMPLATE_NUMERIC_ARG_DEFAULT_STR "47"
#define PROJECT_TEMPLATE_NUMERIC_ARG_DEFAULT 47
#define PROJECT_TEMPLATE_STRING_ARG_DEFAULT "foo"

////////////////////////////////////////////////////////////////////////////////
// Helpers for optionparse
////////////////////////////////////////////////////////////////////////////////

struct ProjectTemplateArg : public option::Arg
{

    static void print_error(const char* msg1, const option::Option& opt, const char* msg2)
    {
        fprintf(stderr, "%s", msg1);
        fwrite(opt.name, opt.namelen, 1, stderr);
        fprintf(stderr, "%s", msg2);
    }

    static option::ArgStatus Unknown(const option::Option& option, bool msg)
    {
        if (msg)
        {
            print_error("Unknown option '", option, "'\n");
        }
        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus NonEmpty(const option::Option& option, bool msg)
    {
        if (option.arg != 0 && option.arg[0] != 0)
        {
            return option::ARG_OK;
        }

        if (msg)
        {
            print_error("Option '", option, "' requires a non-empty argument\n");
        }
        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus Numeric(const option::Option& option, bool msg)
    {
        char* endptr = 0;
        if (option.arg != 0 && strtol(option.arg, &endptr, 10))
        {};

        if (endptr != option.arg && *endptr == 0)
        {
          return option::ARG_OK;
        }
        if (msg)
        {
            print_error("Option '", option, "' requires a numeric argument\n");
        }
        return option::ARG_ILLEGAL;
    }

};

////////////////////////////////////////////////////////////////////////////////
// Command Line options descriptors
////////////////////////////////////////////////////////////////////////////////

// List here the different command line options
enum OptionIndex
{ 
    OPT_IDX_UNKNOWN,
    OPT_IDX_HELP,
    OPT_IDX_LOG_LEVEL,
    OPT_IDX_LOG_FILE,
    OPT_IDX_PROJECT_TEMPLATE_NUMERIC_ARG,
    OPT_IDX_PROJECT_TEMPLATE_STRING_ARG
};

// Option types (UNUSED is generally used for options that take a value as argument)
enum OptionType
{
    OPT_TYPE_UNUSED = 0,
    OPT_TYPE_DISABLED = 1,
    OPT_TYPE_ENABLED = 2
};

// Option descriptors, one for each entry of the OptionIndex enum
const option::Descriptor usage[] =
{
    {
        OPT_IDX_UNKNOWN,    // index
        OPT_TYPE_UNUSED,    // type
        "",         // shortopt
        "",         // longopt
        ProjectTemplateArg::Unknown, // check_arg
        "ProjectTemplate for C++. Copyright 2016 MIND Music Labs\n\nUSAGE: project_template [options]\n\nOptions:" // help
    },
    {
        OPT_IDX_HELP,
        OPT_TYPE_UNUSED,
        "h?",
        "help",
        ProjectTemplateArg::None,
        "\t\t-h --help \tPrint usage and exit."
    },
    {
        OPT_IDX_LOG_LEVEL,
        OPT_TYPE_UNUSED,
        "l",
        "log-level",
        ProjectTemplateArg::NonEmpty,
        "\t\t-l <level>, --log-level=<level> \tSpecify minimum logging level, from ('debug', 'info', 'warning', 'error') [default=" PROJECT_TEMPLATE_LOG_LEVEL_DEFAULT "]."
    },
    {
        OPT_IDX_LOG_FILE,
        OPT_TYPE_UNUSED,
        "L",
        "log-file",
        ProjectTemplateArg::NonEmpty,
        "\t\t-L <filename>, --log-file=<filename> \tSpecify logging file [default=" PROJECT_TEMPLATE_LOG_FILENAME_DEFAULT "]."
    },
    {
        OPT_IDX_PROJECT_TEMPLATE_NUMERIC_ARG,
        OPT_TYPE_UNUSED,
        "n",
        "number",
        ProjectTemplateArg::NonEmpty,
        "\t\t-n <number>, --number=<number> \tSpecify numeric argument [default=" PROJECT_TEMPLATE_NUMERIC_ARG_DEFAULT_STR "]."
    },
    {
        OPT_IDX_PROJECT_TEMPLATE_STRING_ARG,
        OPT_TYPE_UNUSED,
        "s",
        "string",
        ProjectTemplateArg::NonEmpty,
        "\t\t-s <string>, --string=<string> \tSpecify string argument [default=" PROJECT_TEMPLATE_STRING_ARG_DEFAULT "]."
    },
    // Don't touch this one (set default values for optionparse library)
    { 0, 0, 0, 0, 0, 0}
};
