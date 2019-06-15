#ifndef ACCEPTANCE_TESTS_H
#define ACCEPTANCE_TESTS_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "controller.h"
#include "sample_data.h"
#include <QImage>

#include <QDir>
#include <QFile>

using namespace testing;

TEST(AcceptanceTests, detectPitch)
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
    float pitch = analyzer.detectPitch();
    float lower_boundary = 427.4742f;
    float upper_boundary = 452.8931f;
    ASSERT_GT(pitch, lower_boundary);
    ASSERT_LT(pitch, upper_boundary);
}

// test correctness of audio conversion method
TEST(AcceptanceTests, convertBufferToAudio)
{
    auto buffer = SAMPLE_RAW_BUFFER;
    std::deque<float> audio;

    Settings settings;
    settings.readSettings();
    Analyzer analyzer;
    analyzer.setSettings(&settings);

    QAudioFormat format;
    format.setSampleRate(48000);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setSampleSize(16);
    format.setByteOrder(QAudioFormat::LittleEndian);

    analyzer.convertBufferToAudio(buffer.data(), 960 * 21, 480 * 21, format, audio);
    ASSERT_EQ(audio.size(), SAMPLE_AUDIO_FRAME.size());
    ASSERT_EQ(audio, SAMPLE_AUDIO_FRAME);
}

TEST(AcceptanceTests, calculatePosition)
{
    QVector<int> score;
    for (int i = 0; i < 100; i++)
        score.push_back(i);

    Settings settings;
    settings.readSettings();
    Analyzer analyzer;
    analyzer.setSettings(&settings);
    analyzer.setScore(score);

    for (int i = 0; i < 100; i++) {
        int position = analyzer.calculatePosition(i);
        EXPECT_EQ(position, i);
    }
}

TEST(AcceptanceTests, generateMusicSheets)
{
    Settings settings;
    settings.readSettings();

    // create directory
    const QString directory_path = settings.lilypondWorkingDirectory();
    if (!QDir(directory_path).exists())
        QDir().mkdir(directory_path);

    // delete old files
    QDir directory1(directory_path);
    directory1.setNameFilters(QStringList() << "*.*");
    directory1.setFilter(QDir::Files);
    for (auto &dir_file : directory1.entryList())
        directory1.remove(dir_file);

    Lilypond lilypond;
    lilypond.setSettings(&settings);
    lilypond.setScore({60, 61, 62, 63 });
    lilypond.generateScore();

    bool is_not_empty = false;
    QDir directory2(directory_path);
    directory2.setNameFilters(QStringList() << "*.*");
    directory2.setFilter(QDir::Files);
    for (auto &dir_file : directory2.entryList())
        is_not_empty = true;

    ASSERT_TRUE(is_not_empty);
}

TEST(AcceptanceTests, detectStaffPositions)
{
    Settings settings;
    settings.readSettings();
    Lilypond lilypond;
    lilypond.setSettings(&settings);

    ASSERT_LT(settings.staffIndent(), SAMPLE_IMAGE_WIDTH);

    QImage image(SAMPLE_IMAGE_WIDTH, SAMPLE_IMAGE_HEIGHT, QImage::Format_RGB32);
    for (int y = 0; y < image.height(); y++) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        int r = SAMPLE_IMAGE_COLUMN_VALUES[y * 3];
        int g = SAMPLE_IMAGE_COLUMN_VALUES[y * 3 + 1];
        int b = SAMPLE_IMAGE_COLUMN_VALUES[y * 3 + 2];
        line[settings.staffIndent()] = QColor(r, g, b).rgb();
    }
    QVector<int> positions;
    lilypond.detectStaffHorizontalPositionsInImage(image, positions);
    ASSERT_EQ(positions, SAMPLE_IMAGE_CORRECT_STAFF_POSITIONS);
}

TEST(AcceptanceTests, convertPitchIntoMidiNoteNumber)
{
    Settings settings;
    settings.readSettings();
    Analyzer analyzer;
    analyzer.setSettings(&settings);

    for (int i = 0; i < ORDERED_PITCHES.size(); i++) {
        auto &v = ORDERED_PITCHES[i];
        ASSERT_EQ(analyzer.findNoteFromPitch(v), i);
    }

}

#endif // ACCEPTANCE_TESTS_H
