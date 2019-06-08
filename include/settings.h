// Author:  Jakub Precht

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>
#include <QVector>
#include <QJsonObject>

class Settings
{
public:
    bool readSettings();
    int sampleRate() const;
    int frameSize() const;
    int hopSize() const;
    float confidenceCoefficient() const;
    float confidenceShift() const;
    int indicatorWidth() const;
    int indicatorHeight() const;
    int staffIndent() const;
    int notesPerStaff() const;
    int staffsPerPage() const;
    int dpi() const;

    const QVector<float>& minimalConfidence() const;
    const QVector<QPair<float, float>>& notesFrequencyBoundry() const;
    const QVector<int>& indicatorXs() const;
    const QVector<QString>& lilypondNotesNotation() const;

    const QString& lilypondWorkingDirectory() const;
    const QString& lilypondHeader() const;
    const QString& lilypondFooter() const;

    bool verbose() const;
    void setVerbose(bool verbose);

private:
    bool readSettings(const QString &filename);
    double readNumber(const QString &name);
    QString readString(const QString &name);
    bool readNotes();
    bool readIndicatorXPositions();

    // ----------

    bool m_verbose = false;
    QJsonObject m_root;
    bool m_status = false;
    const QString m_default_settings_filename = ":/other/settings.json";

    // essentia

    int m_sample_rate = 0;
    int m_frame_size = 0;
    int m_hop_size = 0;
    float m_confidence_coefficient = 0;
    float m_confidence_shift = 0;
    QVector<float> m_minimal_confidence;
    QVector<QPair<float, float>> m_notes_frequency_boundry;

    // lilypond and gui

    int m_indicator_width = 0;
    int m_indicator_height = 0;
    int m_staff_indent = 0;
    int m_notes_per_staff = 0;
    int m_staffs_per_page = 0;
    int m_dpi = 0;
    QVector<int> m_indicator_xs;
    QVector<QString> m_lilypond_notes_notation;

    QString m_lilypond_working_directory;
    QString m_lilypond_header;
    QString m_lilypond_footer;
};

#endif // SETTINGS_H
