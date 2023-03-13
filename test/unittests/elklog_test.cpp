#include "gtest/gtest.h"

#include "elklog/elk_logger.h"

using namespace elklog;

// A more complex test case where tests can be grouped
// And setup and teardown functions added.
class InitLogTest : public ::testing::Test
{
    protected:
    InitLogTest()
    {
    }
    void SetUp()
    {
        _module_under_test = std::make_unique<ElkLogger>("info");
    }

    void TearDown()
    {
    }

    std::unique_ptr<ElkLogger> _module_under_test;
};

TEST_F(InitLogTest, TestCreation)
{
    auto status = _module_under_test->initialize("./log.txt", "log_1");
    ASSERT_EQ(Status::OK, status);

    // Create another logger with the same name
    ElkLogger logger_2("info");
    status = logger_2.initialize("log_2.txt", "log_1", std::chrono::seconds(0), false);
    ASSERT_EQ(Status::FAILED_TO_START_LOGGER, status);

    // Create a logger with invalid level
    ElkLogger logger_3("debbbug");
    status = logger_3.initialize("log_3.txt", "log_1");
    ASSERT_EQ(Status::INVALID_LOG_LEVEL, status);
}
