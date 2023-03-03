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

add_library(elklog INTERFACE)
target_include_directories(elklog INTERFACE include)
target_compile_features(elklog INTERFACE cxx_std_17)
target_link_libraries(elklog INTERFACE fifo spdlog twine)

#set(INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/examples"
#                 "${PROJECT_SOURCE_DIR}/examples/libs"
#                 "${PROJECT_SOURCE_DIR}/include"
#                 "${PROJECT_SOURCE_DIR}/third-party/optionparser/"
#                 "${PROJECT_SOURCE_DIR}/third-party")


###########
#  Tests  #
###########

add_subdirectory(test)

##############
#  Examples  #
##############

add_executable(elklog_example examples/main.cpp)
target_include_directories(elklog_example PRIVATE ${INCLUDE_DIRS})
target_link_libraries(elklog_example PRIVATE elklog)
target_compile_features(elklog_example PUBLIC cxx_std_17)

