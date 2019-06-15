// Author:  Jakub Precht

#ifndef RECORDER_H
#define RECORDER_H

#include <essentia/algorithmfactory.h>

#include <QTimer>
#include <QAudioRecorder>
#include <QAudioProbe>
#include <QAudioInput>

#include "analyzer.h"

class Settings;

class Recorder : public QObject
{
    Q_OBJECT

public:
    Recorder(QObject *parent = nullptr);
    virtual ~Recorder();
    bool initialize();
    void setSettings(const Settings *settings);
    void setAnalyzer(Analyzer *analyzer);
    void setAudioInput(QString audio_input);

public slots:
    virtual void startFollowing();
    virtual void stopFollowing();
    void processBuffer(const QAudioBuffer buffer);

signals:
    void levelChanged(float level);

private:
    void calculateMaxAmplitude();
    void updateLevel(const QAudioBuffer &buffer);
    void convertBufferToAudio(const QAudioBuffer &buffer);
    void setMaxAmplitude(const QAudioFormat &format);

    // ----------

    // recording

    Analyzer *m_analyzer;
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

    qint64 m_current_second = 0;
    int m_samples_in_current_second = 0;
    std::deque<float> m_audio_memory;
};

#endif // RECORDER_H
