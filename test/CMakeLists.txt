# build gtest framework
add_subdirectory(googletest)
enable_testing()

################
#  Test Files  #
################

SET(TEST_FILES unittests/elklog_test.cpp)

#################################
#  Statically linked libraries  #
#################################

set(LINK_LIBRARIES gtest gtest_main elklog)

add_executable(unit_tests ${TEST_FILES})

target_link_libraries(unit_tests "${LINK_LIBRARIES}")
add_test(unit_tests unit_tests)

# custom target for running the tests
add_custom_target(run_tests ALL COMMAND "./unit_tests")
add_dependencies(run_tests unit_tests)

add_subdirectory(performancetests)