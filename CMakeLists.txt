cmake_minimum_required(VERSION 3.2)
project(project_template LANGUAGES CXX)

###################
#  Build options  #
###################

option(WITH_UNIT_TESTS "Build and run unit tests after compilation" ON)

#########################
#  Include Directories  #
#########################

set(INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/src"
                 "${PROJECT_SOURCE_DIR}/src/libs"
                 "${PROJECT_SOURCE_DIR}/include"
                 "${PROJECT_SOURCE_DIR}/third-party/optionparser/"
                 "${PROJECT_SOURCE_DIR}/third-party")

##################
#  Source Files  #
##################

set(COMPILATION_UNITS src/main.cpp
                      src/logging.cpp)


# Enumerate all the headers separately so that CLion can index them

set(EXTRA_CLION_SOURCES include/public_header.h
                        src/logging.h
                        src/options.h)

set(SOURCE_FILES "${COMPILATION_UNITS}" "${EXTRA_CLION_SOURCES}")


#############################################
#  Subdirectory projects                    #
#############################################

add_subdirectory(libs)
add_subdirectory(third-party)
add_subdirectory(test)

####################
#  Target Objects  #
####################

add_executable(project_template ${SOURCE_FILES})
target_include_directories(project_template PRIVATE ${INCLUDE_DIRS})
target_link_libraries(project_template PRIVATE mylib pthread)
target_compile_features(project_template PUBLIC cxx_std_17)

