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

option(ELKLOG_WITH_UNIT_TESTS "Build and run unit tests after compilation" ON)
option(ELKLOG_WITH_EXAMPLES "Build included examples"  ON)

set(TWINE_WITH_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory(twine)
add_subdirectory(third-party)

#############################
#  Create interface target  #
#############################

add_library(elklog STATIC src/elklog.cpp)
target_include_directories(elklog PUBLIC include)
target_compile_features(elklog PUBLIC cxx_std_17)
target_link_libraries(elklog fifo spdlog twine)


###########
#  Tests  #
###########

add_subdirectory(test)

##############
#  Examples  #
##############

add_executable(elklog_example examples/direct_logging.cpp)
target_include_directories(elklog_example PRIVATE ${INCLUDE_DIRS})
target_link_libraries(elklog_example PRIVATE elklog)
target_compile_features(elklog_example PUBLIC cxx_std_17)

add_executable(static_example examples/static_logging.cpp)
target_include_directories(static_example PRIVATE ${INCLUDE_DIRS})
target_link_libraries(static_example PRIVATE elklog)
target_compile_features(static_example PUBLIC cxx_std_17)

