#ifndef UNIT_TESTS_H
#define UNIT_TESTS_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "scorereader.h"
#include "controller.h"

using namespace testing;

class UnitTests : public ::testing::Test
{
protected:
    ScoreReader score_reader;
    Lilypond lilypond;
    Recorder recorder;
    Analyzer analyzer;
    Controller *controller;

    void SetUp()
    {
        controller = new Controller(false, &lilypond, &recorder, &score_reader, &analyzer);
    }

    void TearDown()
    {
        delete controller;
    }
};

TEST_F(UnitTests, Controller_level)
{
    float level = 0.123f;
    controller->setLevel(level);
    ASSERT_EQ(level, controller->level());
}

TEST_F(UnitTests, Controller_playedNotes)
{
    int played_notes = 123;
    controller->setPlayedNotes(played_notes);
    ASSERT_EQ(played_notes, controller->playedNotes());
}

TEST_F(UnitTests, Controller_indicatorScale)
{
    double indicator_scale = 0.123;
    controller->setIndicatorScale(indicator_scale);
    ASSERT_EQ(indicator_scale, controller->indicatorScale());
}

TEST_F(UnitTests, Controller_pagesNumber)
{
    int pages_number = 123;
    controller->setPagesNumber(pages_number);
    ASSERT_EQ(pages_number, controller->pagesNumber());
}

TEST_F(UnitTests, Controller_scoreLength)
{
    int score_length = 123;
    controller->setScoreLength(score_length);
    ASSERT_EQ(score_length, controller->scoreLength());
}

TEST_F(UnitTests, Controller_follow)
{
    controller->setFollow(true);
    ASSERT_TRUE(controller->follow());
    controller->setFollow(false);
    ASSERT_FALSE(controller->follow());
}

#endif // UNIT_TESTS_H
