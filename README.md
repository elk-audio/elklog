# ElkLog
Logger library with support for logging both non-realtime and realtime threads. ElkLog uses spdlog internally but adds features specifically for logging from Elk Audio OS. 

Copyright 2020-2024 Elk Audio AB, Stockholm, Sweden.

## Usage

### Building and linking
To include ElkLog in a CMake based project the lines below needs to be added to the projects CMake configuration. Further build options can be found in elklog/CMakeLists.txt
```
add_subdirectory(elklog)`
target_link_libraries(project PRIVATE elklog)
```

### Logging
ElkLog can be used in one of 2 ways. Either by directly creating and managing instances of the ElkLogger class. Useful for projects that use multiple loggers and/or libraries that supports multiple instances.
``` 
#include "elklog/elk_logger.h"
elklog::ElkLogger logger("debug");
if (logger.initialize("log.txt", "example_logger") == elklog::Status::OK)
{
   logger.info("log some text");
}  
```
Elklog also has a static log wrapper that can be used for projects where there is only a single logger instance. As the logger is static, logging can be done from any part of the code without passing around an explicit logger instance. 
This can also be used together with ELKLOG_DISABLE_LOGGING to remove logging at compile time, usefor for example in unit testing.

```
#include "elklog/static_logger.h"
ELKLOG_GET_LOGGER;
if (elklog::StaticLogger::init_logger("log.txt", "example_logger", "debug") == elklog::Status::OK)
{
   ELK_LOG_LOG_INFO("Log some text");
}
```
## License

ElkLog is licensed under the MIT License (MIT). See the separate LICENSE file for the details. 
