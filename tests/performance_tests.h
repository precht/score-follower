#ifndef PERFORMANCE_TESTS_H
#define PERFORMANCE_TESTS_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "controller.h"
#include "sample_data.h"

#include <chrono>

using namespace testing;

using Time = std::chrono::system_clock::time_point;
using Duration = std::chrono::duration<double>;
const auto& getTime = std::chrono::system_clock::now;

TEST(PerformanceTests, detectPitch)
{
    Settings settings;
    settings.readSettings();
    Analyzer analyzer;
    analyzer.setSettings(&settings);
    analyzer.initializePitchDetector();

    ASSERT_EQ(analyzer.audioFrameSize(), 0);
    for (auto x : SAMPLE_AUDIO_FRAME)
        analyzer.audioFramePushBack(x);

    ASSERT_EQ(analyzer.audioFrameSize(), SAMPLE_AUDIO_FRAME.size());

    const int called_times_per_second = 25;

    Time beg = getTime();
    for (uint i = 0; i < called_times_per_second; i++) {
        analyzer.detectPitch();
    }
    Time end = getTime();
    Duration d = end - beg;

    EXPECT_LT(d.count(), 0.1);
    std::cout << "Time: " << d.count() << std::endl;
}

// converting buffer should take less then 10% of total processing time
TEST(PerformanceTests, convertBufferToAudio)
{
    auto b = SAMPLE_RAW_BUFFER;
    std::deque<float> out;

    Settings s;
    s.readSettings();
    Analyzer a;
    a.setSettings(&s);

    QAudioFormat f;
    f.setSampleRate(48000);
    f.setSampleType(QAudioFormat::SignedInt);
    f.setSampleSize(16);
    f.setByteOrder(QAudioFormat::LittleEndian);

    const int called_times_per_second = 25;

    Time beg = getTime();
    for (uint i = 0; i < called_times_per_second; i++) {
        a.convertBufferToAudio(b.data(), 960 * 21, 480 * 21, f, out);
        out.clear();
    }
    Time end = getTime();
    Duration d = end - beg;

    EXPECT_LT(d.count(), 0.1);
    std::cout << "Time: " << d.count() << std::endl;
}

TEST(PerformanceTests, calculatePosition)
{
    QVector<int> score;
    for (int i = 0; i < 1e4; i++)
        score.push_back(i);

    Settings settings;
    settings.readSettings();
    Analyzer analyzer;
    analyzer.setSettings(&settings);
    analyzer.setScore(score);

    const int called_times_per_second = 25;

    Time beg = getTime();
    for (int i = 0; i < called_times_per_second; i++)
        analyzer.calculatePosition(i);

    Time end = getTime();
    Duration d = end - beg;

    EXPECT_LT(d.count(), 0.1);
    std::cout << "Time: " << d.count() << std::endl;
}


#endif // PERFORMANCE_TESTS_H
