// Author:  Jakub Precht

#include "analyzer.h"
#include "settings.h"

#include <QThread>
#include <QUrl>
#include <qendian.h>
#include <QDateTime>

#include <essentia/algorithmfactory.h>

using namespace essentia;
using namespace standard;

Analyzer::Analyzer(QObject *parent)
    : QObject(parent)
{
    essentia::init();
}

Analyzer::~Analyzer() { }

void Analyzer::initializePitchDetector()
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

void Analyzer::processFrame()
{
    detectPitch();

    auto &notes_boundry = m_settings->notesFrequencyBoundry();
    if (m_current_pitch < notes_boundry[m_current_note_number].first
            || m_current_pitch > notes_boundry[m_current_note_number].second)
    {
        int note_number = findNoteFromPitch(m_current_pitch);

        if (m_current_confidence >= m_settings->minimalConfidence()[note_number]) {
            calculatePosition(note_number);

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

float Analyzer::detectPitch()
{
    m_window_calculator->compute();
    m_spectrum_calculator->compute();
    m_pitch_detector->compute();

    return m_current_pitch;
}

size_t Analyzer::audioFrameSize() const
{
    return m_audio_frame.size();
}

void Analyzer::audioFrameEraseHopSizeElements()
{
    m_audio_frame.erase(m_audio_frame.begin(), m_audio_frame.begin() + m_settings->hopSize());
}

void Analyzer::audioFramePushBack(float value)
{
    m_audio_frame.push_back(value);
}

//#define DUMP_AUDIO_BUFFER

void Analyzer::convertBufferToAudio(const void *buffer_data, const int buffer_byte_count, const int buffer_sample_count,
                                    const QAudioFormat &buffer_format, std::deque<float> &audio)
{
#ifdef DUMP_AUDIO_BUFFER
    static int dump_counter = 0;
    static std::vector<uint8_t> dump_buffer;
    static std::vector<float> dump_audio;
    static bool is_recording = false;
    static const int start_iteration = 200;

    dump_counter++;
    if (dump_counter == start_iteration) {
        is_recording = true;
        dump_buffer.clear();
        dump_audio.clear();
    }

    if (is_recording) {
        const uint8_t *dump_ptr = static_cast<const uint8_t*>(buffer_data);
        for (int i = 0; i < buffer_byte_count; i++, dump_ptr++)
            dump_buffer.push_back(*dump_ptr);
    }
#endif

    const unsigned char *ptr = reinterpret_cast<const unsigned char*>(buffer_data);
    const auto &fmt = buffer_format;
    const int channel_bytes = fmt.sampleSize() / 8;

    for (int i = 0; i < buffer_sample_count; i++) {
        float value = 0;

        if (fmt.sampleSize() == 8) {
            value = *reinterpret_cast<const qint8*>(ptr);
        } else if (fmt.sampleSize() == 16 ) {
            if (fmt.byteOrder() == QAudioFormat::LittleEndian)
                value = qFromLittleEndian<qint16>(ptr);
            else
                value = qFromBigEndian<qint16>(ptr);
        } else if (fmt.sampleSize() == 32 && (fmt.sampleType() == QAudioFormat::UnSignedInt
                                                 || fmt.sampleType() == QAudioFormat::SignedInt)) {
            if (fmt.byteOrder() == QAudioFormat::LittleEndian)
                value = qFromLittleEndian<quint32>(ptr);
            else
                value = qFromBigEndian<quint32>(ptr);
        } else if (fmt.sampleSize() == 32 && fmt.sampleType() == QAudioFormat::Float) {
            value = *reinterpret_cast<const float*>(ptr);
        } else {
            qDebug() << "Unsupported audio format.";
        }
        ptr += channel_bytes;
        audio.push_back(value);

#ifdef DUMP_AUDIO_BUFFER
        if (is_recording)
            dump_audio.push_back(value);
#endif
    }

#ifdef DUMP_AUDIO_BUFFER
    if (is_recording && dump_counter == start_iteration + 20) {
        auto dump_out = qInfo().nospace();
        dump_out << "FORMAT " << fmt.sampleRate() << " " << fmt.sampleType() << " " << fmt.sampleSize()
                 << " "  << fmt.byteOrder() << " "  << buffer_byte_count << " "  << buffer_sample_count << "\n"
                 << "BUFFER size " << dump_buffer.size() << "\n{ " << dump_buffer[0];
        for (uint i = 1; i < dump_buffer.size(); i++) {
            if (i % 32 == 0) {
                dump_out << ",\n" << dump_buffer[i];
            } else {
                dump_out << ", " << dump_buffer[i];
            }
        }
        const size_t required_size = static_cast<size_t>(m_settings->frameSize());
        dump_out << "}\n\nAUDIO size " << dump_audio.size() << " required " << required_size << "\n{" << dump_audio[0];
        for (uint i = 1; i < dump_audio.size(); i++) {
            if (i % 22 == 0) {
                dump_out << ",\n" << dump_audio[i];
            } else {
                dump_out << ", " << dump_audio[i];
            }
        }
        dump_out << "}\n";
        dump_buffer.clear();
        dump_audio.clear();
    }
#endif
}

void Analyzer::resetDtw()
{
    m_position = -1;
    m_dtw_row.fill(0);
    emit positionChanged(0);
}

int Analyzer::calculatePosition(int new_note_number)
{
    // dtw algorithm
    m_current_note_number = new_note_number;
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

    return position;
}

void Analyzer::setSettings(const Settings *settings)
{
    m_settings = settings;
}

void Analyzer::setScore(const QVector<int> &scoreNotes)
{
    m_score_notes = scoreNotes;
    m_dtw_row.resize(m_score_notes.size());
    m_next_row.resize(m_score_notes.size());
    resetDtw();
}

int Analyzer::findNoteFromPitch(float pitch)
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
