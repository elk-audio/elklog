add_executable(elklog_example direct_logging.cpp)
target_include_directories(elklog_example PRIVATE ${INCLUDE_DIRS})
target_link_libraries(elklog_example PRIVATE elklog)
target_compile_features(elklog_example PUBLIC cxx_std_17)

add_executable(static_example static_logging.cpp)
target_include_directories(static_example PRIVATE ${INCLUDE_DIRS})
target_link_libraries(static_example PRIVATE elklog)
target_compile_features(static_example PUBLIC cxx_std_17)