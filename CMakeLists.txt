cmake_minimum_required(VERSION 3.2)
project(elk-log LANGUAGES CXX)
#####################
#  Project Version  #
#####################

# Don't change version anywhere else. Everything is generated from this.
set(ELKLOG_VERSION_MAJOR 1)
set(ELKLOG_VERSION_MINOR 0)
set(ELKLOG_VERSION_REVISION 0)

project(elk-log 
        DESCRIPTION "Logging library for Elk Audio OS"
        HOMEPAGE_URL ""
        LANGUAGES CXX
        VERSION ${ELKLOG_VERSION_MAJOR}.${ELKLOG_VERSION_MINOR}.${ELKLOG_VERSION_REVISION}
        )

###################
#  Build options  #
###################

option(ELKLOG_MULTI_THREADED_RT_LOGGING "Allow realtime logging from multiple threads simultaneously"  ON)
option(ELKLOG_USE_INCLUDED_TWINE "If Elklog is included in a project that already include Twine, set this to OFF" ON)
option(ELKLOG_WITH_UNIT_TESTS "Build and run unit tests after compilation" ON)
option(ELKLOG_WITH_EXAMPLES "Build included examples"  ON)

set(ELKLOG_FILE_SIZE 10000000 CACHE STRING "Maximum log file size in bytes")
set(ELKLOG_RT_MESSAGE_SIZE 2048 CACHE STRING "Maximum length of log messages from realtime threads")
set(ELKLOG_RT_QUEUE_SIZE 1024 CACHE STRING "Size of realtime log queue")

######################
#  Add dependencies  #
######################

set(TWINE_WITH_TESTS OFF CACHE BOOL "" FORCE)

if (${ELKLOG_USE_INCLUDED_TWINE})
    set(TWINE_WITH_TESTS OFF CACHE BOOL "")
    add_subdirectory(twine)
    set(TWINE_LIB twine)
else()
    find_path(TWINE_PATH NAMES "twine/twine.h")
    find_library(TWINE_LIB
                 NAMES twine
                 PATHS /usr/lib /usr/local/lib
                 )
endif()

add_subdirectory(third-party)

###################
#  Create target  #
###################

add_library(elklog STATIC src/elklog.cpp)
target_include_directories(elklog PUBLIC include ${TWINE_PATH})
set_target_properties(elklog spdlog PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_features(elklog PUBLIC cxx_std_17)
target_compile_definitions(elklog PUBLIC -DELKLOG_FILE_SIZE=${ELKLOG_FILE_SIZE}
                                         -DELKLOG_RT_MESSAGE_SIZE=${ELKLOG_RT_MESSAGE_SIZE}
                                         -DELKLOG_RT_QUEUE_SIZE=${ELKLOG_RT_QUEUE_SIZE})

if(ELKLOG_MULTI_THREADED_RT_LOGGING)
    target_compile_definitions(elklog PUBLIC -DELKLOG_MULTI_THREADED_RT_LOGGING=1)
endif()

target_link_libraries(elklog fifo spdlog ${TWINE_LIB})

###########
#  Tests  #
###########

if (ELKLOG_WITH_UNIT_TESTS)
    add_subdirectory(test)
endif()

##############
#  Examples  #
##############

if (ELKLOG_WITH_EXAMPLES)
    add_subdirectory(examples)
endif()



