#ifndef INTEGRATION_TESTS_H
#define INTEGRATION_TESTS_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <gmock/gmock.h>

#include "controller.h"
#include "scorereader.h"
#include "lilypond.h"
#include "analyzer.h"
#include "recorder.h"

#include "sample_data.h"

using namespace testing;

class MockScoreReader : public ScoreReader
{
public:
    MOCK_METHOD1(readScoreFile, QVector<int>(const QString &filename));
};

class MockLilypond : public Lilypond
{
public:
    MOCK_METHOD1(setScore, void(const QVector<int> &score_notes));
    MOCK_METHOD0(countPages, int());
};

class MockAnalyzer : public Analyzer
{
public:
    MOCK_METHOD1(setScore, void(const QVector<int> &score_notes));
    MOCK_METHOD5(convertBufferToAudio, void(const void *buffer_data, const int buffer_byte_count, const int buffer_sample_count,
                                const QAudioFormat &buffer_format, std::deque<float> &audio));
};

class MockRecorder : public Recorder
{
public:
    MOCK_METHOD0(startFollowing, void());
    MOCK_METHOD0(stopFollowing, void());
};

TEST(IntegrationTests, ControllerOpenScore_ScoreReaderReadScoreFile)
{
    Lilypond lilypond;
    Recorder recorder;
    Analyzer analyzer;
    MockScoreReader mock_score_reader;
    Controller controller(false, &lilypond, &recorder, &mock_score_reader, &analyzer);
    EXPECT_CALL(mock_score_reader, readScoreFile(_)).Times(1);
    controller.openScore("xxx");
}

TEST(IntegrationTests, ControllerOpenScore_LilypondSetScore)
{
    ScoreReader score_reader;
    MockLilypond mock_lilypond;
    Recorder recorder;
    Analyzer analyzer;
    Controller controller(false, &mock_lilypond, &recorder, &score_reader, &analyzer);
    EXPECT_CALL(mock_lilypond, setScore(_)).Times(1);
    controller.openScore("xxx");
}

TEST(IntegrationTests, ControllerOpenScore_AnalyzerSetScore)
{
    ScoreReader score_reader;
    Lilypond lilypond;
    Recorder recorder;
    MockAnalyzer mock_analyzer;
    Controller controller(false, &lilypond, &recorder, &score_reader, &mock_analyzer);
    EXPECT_CALL(mock_analyzer, setScore(_)).Times(1);
    controller.openScore("xxx");
}

TEST(IntegrationTests, ControllerSetFollow_RecorderStartStopFollowing)
{
    ScoreReader score_reader;
    Lilypond lilypond;
    MockRecorder mock_recorder;
    Analyzer analyzer;
    Controller controller(false, &lilypond, &mock_recorder, &score_reader, &analyzer);
    EXPECT_CALL(mock_recorder, startFollowing()).Times(3);
    EXPECT_CALL(mock_recorder, stopFollowing()).Times(3);

    controller.setFollow(true);
    controller.setFollow(false);
    controller.setFollow(true);
    controller.setFollow(false);
    controller.setFollow(true);
    controller.setFollow(false);
}

TEST(IntegrationTests, RecorderProcessBuffer_AnalyzerConvertBufferToAudio)
{
    ScoreReader score_reader;
    Lilypond lilypond;
    Recorder recorder;
    MockAnalyzer mock_analyzer;
    Controller controller(false, &lilypond, &recorder, &score_reader, &mock_analyzer);
    EXPECT_CALL(mock_analyzer, convertBufferToAudio(_, _, _, _, _)).Times(1);

    QAudioBuffer buffer;
    recorder.startFollowing();
    recorder.processBuffer(buffer);
}

TEST(IntegrationTests, LilypondFinishGenerationScore_PageCount)
{
    Settings settings;
    settings.readSettings();
    MockLilypond mock_lilypond;
    mock_lilypond.setSettings(&settings);
    EXPECT_CALL(mock_lilypond, countPages()).Times(AnyNumber()).WillOnce(Return(0));
    mock_lilypond.finish(0, QProcess::ExitStatus::NormalExit);
}

#endif // INTEGRATION_TESTS_H
