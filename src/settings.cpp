// Author:  Jakub Precht

#include "include/settings.h"
#include <cmath>

#include <QDebug>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <algorithm>
#include <QApplication>

bool Settings::readSettings()
{
    QString m_settings_filename = QApplication::applicationDirPath() + "/settings.json";
    QFileInfo check_file(m_settings_filename);
    if (check_file.exists() && check_file.isFile()) {
        m_status = true;
        readSettings(m_settings_filename);
        if (m_status)
            return true;
    }
    qInfo().nospace() << "\nFailed to read " << m_settings_filename << ". Using default settings.\n";

    m_status = true;
    readSettings(m_default_settings_filename);
    if (!m_status)
        qCritical() << "\nFailed to read default settings!\n";
    return m_status;
}

bool Settings::readSettings(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QString content = file.readAll();
    file.close();
    QJsonDocument document = QJsonDocument::fromJson(content.toUtf8());
    m_root = document.object();

    m_status = true;
    m_sample_rate = static_cast<int>(readNumber("sampleRate"));
    m_frame_size = static_cast<int>(readNumber("frameSize"));
    m_hop_size = static_cast<int>(readNumber("hopSize"));
    m_indicator_width = static_cast<int>(readNumber("indicatorWidth"));
    m_indicator_height = static_cast<int>(readNumber("indicatorHeight"));
    m_staff_indent = static_cast<int>(readNumber("staffIndent"));
    m_notes_per_staff = static_cast<int>(readNumber("notesPerStaff"));
    m_staffs_per_page = static_cast<int>(readNumber("staffsPerPage"));
    m_dpi = static_cast<int>(readNumber("dpi"));
    m_confidence_coefficient = static_cast<float>(readNumber("confidenceCoefficient"));
    m_confidence_shift = static_cast<float>(readNumber("confidenceShift"));
    m_lilypond_working_directory = readString("lilypondWorkingDirectory");
    m_lilypond_header = readString("lilypondHeader");
    m_lilypond_footer = readString("lilypondFooter");

    if (readNotes() == false) {
        qWarning() << "Failed to read \"notes\".";
        m_status = false;
    }
    if (readIndicatorXPositions() == false) {
        qWarning() << "Failed to read \"indicatorXPositions\".";
        m_status = false;
    }

    if (m_frame_size % 2 == 1) {
        qWarning().nospace() << "Frame size cannot be odd. Read value: " << m_frame_size << ".";
        m_status = false;
    }

    m_root = { };
    return m_status;
}

double Settings::readNumber(const QString &name)
{
    QJsonValue value = m_root.value(name);
    double result = 0;
    bool tmp_status = false;

    if (value.isDouble()) {
        result = value.toDouble();
        tmp_status = true;
    }
    else if (value.isString()) {
        QStringList pieces = value.toString().split(" ");
        pieces.removeAll("");

        if (pieces.size() == 1) {
            bool ok = true;
            result = pieces[0].toDouble(&ok);
            tmp_status = ok;
        }
        else if (pieces.size() == 3 && pieces[1] == "*") {
            bool ok1 = true, ok2 = true;
            result = pieces[0].toInt(&ok1) * pieces[2].toInt(&ok2);
            tmp_status = ok1 && ok2;
        }
    }

    if (tmp_status == false)
        qWarning().nospace() << "Failed to read number " << name << '.';
    m_status &= tmp_status;
    return result;
}

QString Settings::readString(const QString &name)
{
    QJsonValue value = m_root.value(name);
    if (!value.isString()) {
        qWarning().nospace() << "Failed to read string " << name << '.';
        m_status = false;
        return "";
    }
    return value.toString();
}

bool Settings::readNotes()
{
    if (m_root.value("notes").isArray() == false)
        return false;

    QJsonArray notes = m_root.value("notes").toArray();
    QVector<float> frequency(notes.size());
    m_minimal_confidence.fill(0, notes.size());
    m_lilypond_notes_notation.fill("", notes.size());

    for (auto it : notes) {
        if (it.isArray() == false)
            return false;
        QJsonArray row = it.toArray();
        if (!row[0].isDouble() || !row[1].isDouble() || !row[2].isDouble() || !row[3].isString()) // check if row is correct
            return false;
        int number = static_cast<int>(row[0].toDouble());
        frequency[number] = static_cast<float>(row[1].toDouble());
        m_minimal_confidence[number] = static_cast<float>(row[2].toDouble());
        if (m_minimal_confidence[number] == 0.f) {
            m_minimal_confidence[number] = 1;
        } else {
            m_minimal_confidence[number] =
                    qMax(0.f, m_minimal_confidence[number] * m_confidence_coefficient + m_confidence_shift);
        }
        m_lilypond_notes_notation[number] = row[3].toString();
    }

    // calc notes frequency boundries
    float last_boundry = 0;
    m_notes_frequency_boundry.clear();
    for (int i = 1; i < frequency.size(); i++) {
        float boundry = std::sqrt(frequency[i - 1] * frequency[i]);
        m_notes_frequency_boundry.push_back({ last_boundry, boundry });
        last_boundry = boundry;
    }
    m_notes_frequency_boundry.push_back({ last_boundry, std::numeric_limits<float>::max() });

    return true;
}

bool Settings::readIndicatorXPositions()
{
    if (m_root.value("indicatorXPositions").isArray() == false)
        return false;

    QJsonArray positions = m_root.value("indicatorXPositions").toArray();
    m_indicator_xs.clear();
    for (auto it : positions) {
        if (it.isDouble() == false)
            return false;
        m_indicator_xs.push_back(it.toInt());
    }
    std::sort(m_indicator_xs.begin(), m_indicator_xs.end());
    if (m_indicator_xs.size() > 1) {
        int shift = (m_indicator_xs[1] - m_indicator_xs[0]) / 2;
        for (auto &x : m_indicator_xs)
            x += shift;
    }

    return true;
}

bool Settings::verbose() const
{
    return m_verbose;
}

void Settings::setVerbose(bool verbose)
{
    m_verbose = verbose;
}

int Settings::sampleRate() const
{
    return m_sample_rate;
}

int Settings::frameSize() const
{
    return m_frame_size;
}

int Settings::hopSize() const
{
    return m_hop_size;
}

float Settings::confidenceCoefficient() const
{
    return m_confidence_coefficient;
}

float Settings::confidenceShift() const
{
    return m_confidence_shift;
}

int Settings::indicatorWidth() const
{
    return m_indicator_width;
}

int Settings::indicatorHeight() const
{
    return m_indicator_height;
}

int Settings::staffIndent() const
{
    return m_staff_indent;
}

int Settings::notesPerStaff() const
{
    return m_notes_per_staff;
}

int Settings::staffsPerPage() const
{
    return m_staffs_per_page;
}

int Settings::dpi() const
{
    return m_dpi;
}

const QVector<float>& Settings::minimalConfidence() const
{
    return m_minimal_confidence;
}

const QVector<QPair<float, float>>& Settings::notesFrequencyBoundry() const
{
    return m_notes_frequency_boundry;
}

const QString& Settings::lilypondWorkingDirectory() const
{
    return m_lilypond_working_directory;
}

const QString &Settings::lilypondHeader() const
{
    return m_lilypond_header;
}

const QString &Settings::lilypondFooter() const
{
    return m_lilypond_footer;
}

const QVector<int>& Settings::indicatorXs() const
{
    return m_indicator_xs;
}

const QVector<QString>& Settings::lilypondNotesNotation() const
{
    return m_lilypond_notes_notation;
}
