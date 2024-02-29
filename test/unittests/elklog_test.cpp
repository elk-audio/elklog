#include <semaphore>

#include "gtest/gtest.h"

#include "elklog/elk_logger.h"

using namespace elklog;

using LoggerCallback = std::function<void(spdlog::level::level_enum level,
                                          const std::string& payload,
                                          const std::string& logger_name)>;
class TestingSink : public spdlog::sinks::base_sink<spdlog::details::null_mutex>
{
public:
    TestingSink(LoggerCallback logging_cb = nullptr):
        _logging_cb(logging_cb)
    {}

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        const std::string payload(msg.payload.begin(), msg.payload.end());
        const std::string logger_name(msg.logger_name.begin(), msg.logger_name.end());
        if (_logging_cb == nullptr)
        {
            return;
        }
        _logging_cb(msg.level, payload, logger_name);
    }

    void flush_() override {};

private:
    LoggerCallback _logging_cb;
};

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

TEST_F(InitLogTest, TestAddingSinkWithoutInit)
{
    auto test_sink = std::make_shared<TestingSink>();
    auto status = _module_under_test->add_log_sink(test_sink);
    ASSERT_EQ(Status::LOGGER_NOT_INITIALIZED, status);
}

TEST_F(InitLogTest, TestAddingSinkWithInit)
{
    auto test_sink = std::make_shared<TestingSink>();
    auto status = _module_under_test->initialize("./log.txt", "log_1");
    status = _module_under_test->add_log_sink(test_sink);
    ASSERT_EQ(Status::OK, status);
}

TEST_F(InitLogTest, TestLoggingToSink)
{
    bool logging_success = false;
    std::binary_semaphore sem(0);
    auto test_sink = std::make_shared<TestingSink>([&logging_success, &sem]
    (spdlog::level::level_enum level, const std::string& payload, const std::string& logger_name)
    {
        EXPECT_EQ(spdlog::level::info, level);
        EXPECT_EQ("Started logger: log_1.", payload);
        EXPECT_EQ("log_1", logger_name);
        logging_success = true;
        sem.release();
    });
    auto status = _module_under_test->initialize("./log.txt", "log_1");
    status = _module_under_test->add_log_sink(test_sink);
    sem.acquire();
    ASSERT_TRUE(logging_success);
}
