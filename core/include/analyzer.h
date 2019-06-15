// Author:  Jakub Precht

#ifndef ANALYZER_H
#define ANALYZER_H

#include <essentia/algorithmfactory.h>

#include <QTimer>
#include <QAudioRecorder>
#include <QAudioProbe>
#include <QAudioInput>

#include <queue>

class Settings;

class Analyzer : public QObject
{
    Q_OBJECT

public:
    Analyzer(QObject *parent = nullptr);
    virtual ~Analyzer();
    bool initialize();
    virtual void setScore(const QVector<int> &score_notes);
    void resetDtw();
    void setSettings(const Settings *settings);
    void initializePitchDetector();
    int findNoteFromPitch(float pitch);
    int calculatePosition(int new_note_number);
    virtual void convertBufferToAudio(const void *buffer_data, const int buffer_byte_count, const int buffer_sample_count,
                                      const QAudioFormat &buffer_format, std::deque<float> &audio);
    void processFrame();
    float detectPitch();
    size_t audioFrameSize() const;
    void audioFrameEraseHopSizeElements();
    void audioFramePushBack(float value);

signals:
    void positionChanged(int position);

private:
    // ----------

    const Settings *m_settings;
    int m_position = 0;
    qint64 m_current_second = 0;
    int m_samples_in_current_second = 0;
    QVector<int> m_score_notes;
    QVector<int64_t> m_dtw_row;
    QVector<int64_t> m_next_row;

    // essentia

    const int64_t m_infinity = std::numeric_limits<int64_t>::max();
    int m_buffer_size = 0;
    int m_current_note_number = 0;
    float m_current_pitch = 0;
    float m_current_confidence = 0;
    bool m_last_was_skipped = false;
    int m_last_skipped_note = 0;
    int m_skipped_count = 0;

    std::vector<float> m_audio_frame;
    std::vector<float> m_spectrum;
    std::vector<float> m_windowed_frame;

    essentia::standard::Algorithm* m_window_calculator;
    essentia::standard::Algorithm* m_spectrum_calculator;
    essentia::standard::Algorithm* m_pitch_detector;
};

#endif // ANALYZER_H
