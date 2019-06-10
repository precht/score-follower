#ifndef TST_CASENAME1_H
#define TST_CASENAME1_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

using namespace testing;

TEST(CaseName1, SetName1)
{
    EXPECT_EQ(1, 1);
    ASSERT_THAT(0, Eq(0));
}

#endif // TST_CASENAME1_H
