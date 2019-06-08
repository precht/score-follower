// Author:  Jakub Precht

#ifndef RECORDER_H
#define RECORDER_H

#include <essentia/algorithmfactory.h>

#include <QTimer>
#include <QAudioRecorder>
#include <QAudioProbe>
#include <QAudioInput>

class Settings;

class Recorder : public QObject
{
    Q_OBJECT

public:
    Recorder(QObject *parent = nullptr);
    bool initialize();
    void setScore(const QVector<int> &score_notes);
    void resetDtw();
    void setSettings(const Settings *settings);
    void setAudioInput(QString audio_input);

public slots:
    void startFollowing();
    void stopFollowing();
    void processBuffer(const QAudioBuffer buffer);

signals:
    void positionChanged(int position);
    void levelChanged(float level);

private:
    void initializePitchDetector();
    int findNoteFromPitch(float pitch);
    void calculatePosition();
    void calculateMaxAmplitude();
    void updateLevel(const QAudioBuffer &buffer);
    void convertBufferToAudio(const QAudioBuffer &buffer);
    void processFrame();
    void setMaxAmplitude(const QAudioFormat &format);

    // ----------

    // recording

    const Settings *m_settings;
    bool m_is_following = false;
    QTimer *m_timer = nullptr;
    QAudioProbe *m_probe = nullptr;
    QAudioRecorder *m_recorder = nullptr;
    QAudioInput *m_audio_input = nullptr;
    QAudioFormat m_current_format;
    QAudioEncoderSettings m_recorder_settings;

    float m_max_amplitude = 1;
    float m_level = 0;
    int m_level_count = 0;
    const int m_max_level_count = 16;

    // position

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

    std::deque<float> m_memory;
    std::vector<float> m_audio_frame;
    std::vector<float> m_spectrum;
    std::vector<float> m_windowed_frame;

    essentia::standard::Algorithm* m_window_calculator;
    essentia::standard::Algorithm* m_spectrum_calculator;
    essentia::standard::Algorithm* m_pitch_detector;
};

#endif // RECORDER_H
