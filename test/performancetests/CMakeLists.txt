add_executable(rt_log_perf EXCLUDE_FROM_ALL  rt_log_performance.cpp )
target_include_directories(rt_log_perf PRIVATE ${INCLUDE_DIRS})
target_link_libraries(rt_log_perf PRIVATE elklog)
target_compile_features(rt_log_perf PUBLIC cxx_std_17)
