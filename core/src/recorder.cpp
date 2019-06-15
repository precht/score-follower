// Author:  Jakub Precht

#include "recorder.h"
#include "settings.h"

#include <QThread>
#include <QUrl>
#include <qendian.h>
#include <QDateTime>

#include <essentia/algorithmfactory.h>

using namespace essentia;
using namespace standard;

Recorder::Recorder(QObject *parent)
    : QObject(parent)
{
    essentia::init();
}

Recorder::~Recorder() { }

bool Recorder::initialize()
{
    m_analyzer->initializePitchDetector();

    m_recorder = new QAudioRecorder(this);
    m_recorder_settings.setChannelCount(1);
    m_recorder_settings.setSampleRate(m_settings->sampleRate());
    m_recorder->setEncodingSettings(m_recorder_settings);
    m_recorder->setOutputLocation(QString("/dev/null"));

    m_probe = new QAudioProbe(this);
    connect(m_probe, &QAudioProbe::audioBufferProbed, this, &Recorder::processBuffer);
    m_probe->setSource(m_recorder);

    m_recorder->record();
    if (m_recorder->state() != QAudioRecorder::RecordingState) {
        qWarning().nospace() << "Failed to create audio input device. Propably unsupported sample rate "
                             << m_settings->sampleRate() << ".";
        return false;
    } else {
        return true;
    }
}

void Recorder::processBuffer(const QAudioBuffer buffer)
{

    if (m_settings->verbose()) {
        auto time = QDateTime::currentDateTime();
        if (m_current_second != time.toSecsSinceEpoch()) {
            if (m_current_second != 0) {
                qInfo().nospace().noquote() << time.toString("hh:mm:ss") << ": " << m_samples_in_current_second << " samples.";
                m_samples_in_current_second = 0;
            } else {
                qInfo() << "Buffer size: " <<  buffer.sampleCount();
            }
            m_current_second = time.toSecsSinceEpoch();
        }
        m_samples_in_current_second += buffer.sampleCount();
    }

    if (buffer.format() != m_current_format) {
        m_current_format = buffer.format();
        setMaxAmplitude(buffer.format());
    }

    updateLevel(buffer);
    if (!m_is_following)
        return;

    m_analyzer->convertBufferToAudio(buffer.data(), buffer.byteCount(), buffer.sampleCount(), buffer.format(), m_audio_memory);

    const size_t required_size = static_cast<size_t>(m_settings->frameSize());
    while (m_analyzer->audioFrameSize() + m_audio_memory.size() >= required_size) {
        while (m_analyzer->audioFrameSize() < required_size) {
            m_analyzer->audioFramePushBack(m_audio_memory.front());
            m_audio_memory.pop_front();
        }
        m_analyzer->processFrame();
        m_analyzer->audioFrameEraseHopSizeElements();
    }
    while (!m_audio_memory.empty()) {
        m_analyzer->audioFramePushBack(m_audio_memory.front());
        m_audio_memory.pop_front();
    }
}

void Recorder::updateLevel(const QAudioBuffer &buffer)
{
    const unsigned char *ptr = reinterpret_cast<const unsigned char*>(buffer.data());
    const auto format = buffer.format();

    if (format.sampleSize() == 8) {
        m_level += qAbs(*reinterpret_cast<const qint8*>(ptr) / m_max_amplitude);
    } else if (format.sampleSize() == 16 ) {
        if (format.byteOrder() == QAudioFormat::LittleEndian)
            m_level += qAbs(qFromLittleEndian<qint16>(ptr) / m_max_amplitude);
        else
            m_level += qAbs(qFromBigEndian<qint16>(ptr) / m_max_amplitude);
    } else if (format.sampleSize() == 32 && (format.sampleType() == QAudioFormat::UnSignedInt
                                             || format.sampleType() == QAudioFormat::SignedInt)) {
        if (format.byteOrder() == QAudioFormat::LittleEndian)
            m_level += qAbs(qFromLittleEndian<quint32>(ptr) / m_max_amplitude);
        else
            m_level += qAbs(qFromBigEndian<quint32>(ptr) / m_max_amplitude);
    } else if (format.sampleSize() == 32 && format.sampleType() == QAudioFormat::Float) {
        m_level += qAbs(*reinterpret_cast<const float*>(ptr) / m_max_amplitude);
    }

    m_level_count++;
    if (m_level_count == m_max_level_count) {
        emit levelChanged(m_level / m_max_level_count);
        m_level = 0;
        m_level_count = 0;
    }
}

void Recorder::setSettings(const Settings *settings)
{
    m_settings = settings;
}

void Recorder::setAnalyzer(Analyzer *analyzer)
{
    m_analyzer = analyzer;
}

void Recorder::setMaxAmplitude(const QAudioFormat &format)
{
    switch (format.sampleSize()) {
    case 8:
        switch (format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            m_max_amplitude = 255;
            break;
        case QAudioFormat::SignedInt:
            m_max_amplitude = 127;
            break;
        default:
            break;
        }
        break;
    case 16:
        switch (format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            m_max_amplitude = 65535;
            break;
        case QAudioFormat::SignedInt:
            m_max_amplitude = 32767;
            break;
        default:
            break;
        }
        break;

    case 32:
        switch (format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            m_max_amplitude = 0xffffffff;
            break;
        case QAudioFormat::SignedInt:
            m_max_amplitude = 0x7fffffff;
            break;
        case QAudioFormat::Float:
            m_max_amplitude = 1;
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }
    m_max_amplitude /= 4; // because this way level bar changes are more visible
}

void Recorder::startFollowing()
{
    m_analyzer->resetDtw();
    m_is_following = true;
    qInfo() << "Started score following.";
}

void Recorder::stopFollowing()
{
    m_is_following = false;
    qInfo() << "Stopped score following.";
}
