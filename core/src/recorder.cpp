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

bool Recorder::initialize()
{
    initializePitchDetector();

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

void Recorder::initializePitchDetector()
{
    AlgorithmFactory& factory = AlgorithmFactory::instance();

    m_window_calculator = factory.create("Windowing", "type", "hann", "zeroPadding", 0);
    m_spectrum_calculator = factory.create("Spectrum", "size", m_settings->frameSize());
    m_pitch_detector = factory.create("PitchYinFFT",
                                      "frameSize", m_settings->frameSize(),
                                      "sampleRate", m_settings->sampleRate());

    m_window_calculator->input("frame").set(m_audio_frame);
    m_window_calculator->output("frame").set(m_windowed_frame);

    m_spectrum_calculator->input("frame").set(m_windowed_frame);
    m_spectrum_calculator->output("spectrum").set(m_spectrum);

    m_pitch_detector->input("spectrum").set(m_spectrum);
    m_pitch_detector->output("pitch").set(m_current_pitch);
    m_pitch_detector->output("pitchConfidence").set(m_current_confidence);
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

    convertBufferToAudio(buffer);

    const size_t required_size = static_cast<size_t>(m_settings->frameSize());
    while (m_audio_frame.size() + m_memory.size() >= required_size) {
        while (m_audio_frame.size() < required_size) {
            m_audio_frame.push_back(m_memory.front());
            m_memory.pop_front();
        }
        processFrame();
        m_audio_frame.erase(m_audio_frame.begin(), m_audio_frame.begin() + m_settings->hopSize());
    }
    while (!m_memory.empty()) {
        m_audio_frame.push_back(m_memory.front());
        m_memory.pop_front();
    }
}

void Recorder::processFrame()
{
    m_window_calculator->compute();
    m_spectrum_calculator->compute();
    m_pitch_detector->compute();

    auto &notes_boundry = m_settings->notesFrequencyBoundry();
    if (m_current_pitch < notes_boundry[m_current_note_number].first || m_current_pitch > notes_boundry[m_current_note_number].second) {
        int note_number = findNoteFromPitch(m_current_pitch);

        if (m_current_confidence >= m_settings->minimalConfidence()[note_number]) {
            m_current_note_number = note_number;
            calculatePosition();

            if (m_settings->verbose() && m_last_was_skipped) {
                qInfo().nospace() << "Note " << m_last_skipped_note << " was skipped " << m_skipped_count  << " times (<"
                                  << m_settings->minimalConfidence()[note_number] << ").";
                m_last_was_skipped = false;
            }

            qInfo().nospace() << "Detected note " << note_number << ".";
        }
        else if (m_settings->verbose()){
            if (m_last_was_skipped && m_last_skipped_note != note_number) {
                qInfo().nospace() << "Note " << m_last_skipped_note << " was skipped " << m_skipped_count  << " times (<"
                                  << m_settings->minimalConfidence()[note_number] << ").";
                m_skipped_count = 0;
            }
            m_last_was_skipped = true;
            m_last_skipped_note = note_number;
            m_skipped_count++;
        }
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

void Recorder::convertBufferToAudio(const QAudioBuffer &buffer)
{
    const unsigned char *ptr = reinterpret_cast<const unsigned char*>(buffer.data());
    const auto format = buffer.format();
    const int channel_bytes = format.sampleSize() / 8;

    const size_t required_size = static_cast<size_t>(m_settings->frameSize());
    for (int i = 0; i < buffer.sampleCount(); i++) {
        float value = 0;

        if (format.sampleSize() == 8) {
            value = *reinterpret_cast<const qint8*>(ptr);
        } else if (format.sampleSize() == 16 ) {
            if (format.byteOrder() == QAudioFormat::LittleEndian)
                value = qFromLittleEndian<qint16>(ptr);
            else
                value = qFromBigEndian<qint16>(ptr);
        } else if (format.sampleSize() == 32 && (format.sampleType() == QAudioFormat::UnSignedInt
                                                 || format.sampleType() == QAudioFormat::SignedInt)) {
            if (format.byteOrder() == QAudioFormat::LittleEndian)
                value = qFromLittleEndian<quint32>(ptr);
            else
                value = qFromBigEndian<quint32>(ptr);
        } else if (format.sampleSize() == 32 && format.sampleType() == QAudioFormat::Float) {
            value = *reinterpret_cast<const float*>(ptr);
        } else {
            qDebug() << "Unsupported audio format.";
        }
        ptr += channel_bytes;

        if (m_audio_frame.size() < required_size)
            m_audio_frame.push_back(value);
        else
            m_memory.push_back(value);
    }
}

void Recorder::resetDtw()
{
    m_position = -1;
    m_dtw_row.fill(0);
}

void Recorder::calculatePosition()
{
    // dtw algorithm
    m_next_row[0] = qAbs(m_current_note_number - m_score_notes[0]) + m_dtw_row[0];

    int position = 0;
    int64_t min_value = m_next_row[0];
    for (int i = 1; i < m_dtw_row.size(); i++) {
        m_next_row[i] = qAbs(m_current_note_number - m_score_notes[i]) + qMin(m_next_row[i - 1], qMin(m_dtw_row[i], m_dtw_row[i-1]));
        if (m_next_row[i] < min_value) {
            position = i;
            min_value = m_next_row[i];
        }
    }

    m_dtw_row.swap(m_next_row); // fast swap
    if (position != m_position)
        emit positionChanged(position + 1);
    m_position = position;
}

void Recorder::setSettings(const Settings *settings)
{
    m_settings = settings;
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

void Recorder::setScore(const QVector<int> &scoreNotes)
{
    m_score_notes = scoreNotes;
    m_dtw_row.resize(m_score_notes.size());
    m_next_row.resize(m_score_notes.size());
    resetDtw();
}

int Recorder::findNoteFromPitch(float pitch)
{
    auto &notes_boundry = m_settings->notesFrequencyBoundry();
    int beg = 0, end = notes_boundry.size() - 1;
    while (beg < end) {
        int mid = (beg + end) / 2;
        if (notes_boundry[mid].second < pitch)
            beg = mid + 1;
        else if (notes_boundry[mid].first > pitch)
            end = mid - 1;
        else
            beg = end = mid;
    }
    return beg;
}

void Recorder::startFollowing()
{
    resetDtw();
    m_is_following = true;
    emit positionChanged(0);
    qInfo() << "Started score following.";
}

void Recorder::stopFollowing()
{
    m_is_following = false;
    qInfo() << "Stopped score following.";
}
