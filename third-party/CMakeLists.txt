############################
#  spdlog compile options  #
############################
# Build spdlog as a static library - improves compile times a lot
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
# Disable unused features
set(SPDLOG_NO_THREAD_ID ON CACHE BOOL "" FORCE)
set(SPDLOG_NO_ATOMIC_LEVELS ON CACHE BOOL "" FORCE)

# The two below are mutually exclusive.
set(SPDLOG_USE_STD_FORMAT OFF CACHE BOOL "" FORCE) # this also forces C++ 20, in spdlog's CMakeLists.txt
set(SPDLOG_FMT_EXTERNAL OFF CACHE BOOL "" FORCE)

add_subdirectory(spdlog)

if (MSVC)
    target_compile_options(spdlog PRIVATE "/wd4996") #C4996: Deprecated warning.
endif()

add_subdirectory(fifo)

