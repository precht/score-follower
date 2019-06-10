#ifndef TST_CASENAME1_H
#define TST_CASENAME1_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "controller.h"

using namespace testing;

TEST(UnitTests, Controller_level)
{
    Controller c;
    float level = 0.123f;
    c.setLevel(level);
    ASSERT_EQ(level, c.level());
}

TEST(UnitTests, Controller_playedNotes)
{
    Controller c;
    int played_notes = 123;
    c.setPlayedNotes(played_notes);
    ASSERT_EQ(played_notes, c.playedNotes());
}

TEST(UnitTests, Controller_indicatorScale)
{
    Controller c;
    double indicator_scale = 0.123;
    c.setIndicatorScale(indicator_scale);
    ASSERT_EQ(indicator_scale, c.indicatorScale());
}

TEST(UnitTests, Controller_pagesNumber)
{
    Controller c;
    int pages_number = 123;
    c.setPagesNumber(pages_number);
    ASSERT_EQ(pages_number, c.pagesNumber());
}

TEST(UnitTests, Controller_scoreLength)
{
    Controller c;
    int score_length = 123;
    c.setScoreLength(score_length);
    ASSERT_EQ(score_length, c.scoreLength());
}

TEST(UnitTests, Controller_follow)
{
    Controller c;
    c.setFollow(true);
    ASSERT_TRUE(c.follow());
    c.setFollow(false);
    ASSERT_FALSE(c.follow());
}

#endif // TST_CASENAME1_H
