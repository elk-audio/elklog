############################
#  spdlog compile options  #
############################
# Build spdlog as a static library - improves compile times a lot
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
# Disable unused features
set(SPDLOG_NO_THREAD_ID ON CACHE BOOL "" FORCE)
set(SPDLOG_NO_ATOMIC_LEVELS ON CACHE BOOL "" FORCE)

add_subdirectory(spdlog)
add_subdirectory(fifo)

