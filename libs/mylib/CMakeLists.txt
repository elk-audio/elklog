cmake_minimum_required(VERSION 3.8)
project(mylibrary LANGUAGES CXX)

#########################
#  Include Directories  #
#########################

set(INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include"
                 "${PROJECT_SOURCE_DIR}/src"
)

##################
#  Source Files  #
##################

set(COMPILATION_UNITS src/mylib.cpp)

# Enumerate all the headers separately so that CLion can index them

set(EXTRA_CLION_SOURCES include/mylib.h)

set(SOURCE_FILES "${COMPILATION_UNITS}" "${EXTRA_CLION_SOURCES}")

####################
#  Target Objects  #
####################

add_library(mylib ${SOURCE_FILES})
target_include_directories(mylib PUBLIC ${INCLUDE_DIRS})
target_compile_features(mylib PUBLIC cxx_std_17)

