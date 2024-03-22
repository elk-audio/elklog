#include "gtest/gtest.h"

#include "elklog/rtlogmessage.h"

using namespace elklog;

TEST(RtLogMessageTest, TestCreation)
{
    RtLogMessage<512> module_under_test;

    EXPECT_EQ(std::chrono::nanoseconds(0), module_under_test.timestamp());
    EXPECT_EQ(RtLogLevel::INFO, module_under_test.level());
    EXPECT_EQ(0, module_under_test.length());
    ASSERT_STREQ("", module_under_test.message());
}

TEST(RtLogMessageTest, TestCopyingAndAssignmentFormatted)
{
    RtLogMessage<512> module_under_test;

    module_under_test.set_message(RtLogLevel::ERROR, std::chrono::nanoseconds(123), "Test {}_{}", "message", 1);

    EXPECT_STREQ("Test message_1", module_under_test.message());
    EXPECT_EQ(std::chrono::nanoseconds(123), module_under_test.timestamp());
    EXPECT_EQ(RtLogLevel::ERROR, module_under_test.level());
    EXPECT_EQ(14, module_under_test.length());

    // Exercise assignment operator and verify that copy is identical
    RtLogMessage<512> msg_2 = module_under_test;
    EXPECT_EQ(std::chrono::nanoseconds(123), msg_2.timestamp());
    EXPECT_EQ(RtLogLevel::ERROR, msg_2.level());
    EXPECT_EQ(14, msg_2.length());
    EXPECT_STREQ(module_under_test.message(), msg_2.message());
}

TEST(RtLogMessageTest, TestCopyingAndAssignment)
{
    RtLogMessage<512> module_under_test;

    module_under_test.set_message(RtLogLevel::INFO, std::chrono::nanoseconds(456), "Test single message");

    EXPECT_STREQ("Test single message", module_under_test.message());
    EXPECT_EQ(std::chrono::nanoseconds(456), module_under_test.timestamp());
    EXPECT_EQ(RtLogLevel::INFO, module_under_test.level());
    EXPECT_EQ(20, module_under_test.length());
}

TEST(RtLogMessageTest, TestMaxSize)
{
    RtLogMessage<24> module_under_test;

    module_under_test.set_message(RtLogLevel::WARNING, std::chrono::nanoseconds(123), "Test message is too {}, {}", "long and will be clipped", 1);

    EXPECT_STREQ("Test message is too lon", module_under_test.message());
    EXPECT_EQ(std::chrono::nanoseconds(123), module_under_test.timestamp());
    EXPECT_EQ(RtLogLevel::WARNING, module_under_test.level());
    EXPECT_EQ(23, module_under_test.length());

    // Exercise assignment operator and verify that copy is identical
    RtLogMessage<24> msg_2 = module_under_test;
    EXPECT_EQ(std::chrono::nanoseconds(123), module_under_test.timestamp());
    EXPECT_EQ(RtLogLevel::WARNING, module_under_test.level());
    EXPECT_EQ(23, module_under_test.length());
    EXPECT_STREQ(module_under_test.message(), msg_2.message());
}