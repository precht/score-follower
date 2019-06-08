// Author:  Jakub Precht

#include "lilypond.h"
#include "settings.h"

#include <QProcess>
#include <QDebug>
#include <QDir>
#include <QImage>

Lilypond::Lilypond(QObject *parent) : QObject(parent) { }

void Lilypond::setScore(const QVector<int> &score_notes)
{
    m_score_notes = score_notes;
}

void Lilypond::setSettings(const Settings *settings)
{
    m_settings = settings;
}

void Lilypond::generateScore()
{
    // create directory
    const QString directory_path = m_settings->lilypondWorkingDirectory();
    if (!QDir(directory_path).exists())
        QDir().mkdir(directory_path);

    // delete old files
    QDir directory(directory_path);
    directory.setNameFilters(QStringList() << "*.*");
    directory.setFilter(QDir::Files);
    for (auto &dir_file : directory.entryList())
        directory.remove(dir_file);

    QFile lilypond_file(directory_path + "score.ly");
    if (!lilypond_file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qDebug() << "Failed to open: " << directory_path + "score.ly";
        return;
    }

    QTextStream stream(&lilypond_file);
    stream << m_settings->lilypondHeader();
    int index = 0;
    const int notes_per_staff = m_settings->notesPerStaff();
    //  bool is_bass_clef = false;
    auto &notation = m_settings->lilypondNotesNotation();
    for (; index < m_score_notes.size(); index++) {
        if (index % notes_per_staff == 0 && index > 0) {
            stream << "\\break\n";
        }
        //    // switch bass and tremble clef
        //    if (index % m_settings->notesPerStaff() == 0) {
        //      bool is_low_note = false;
        //      for (int j = 0; j < notes_per_staff && index + j < m_score_notes.size(); j++)
        //        if (m_score_notes[index + j] < 48)
        //          is_low_note = true;
        //      if (is_low_note && !is_bass_clef) {
        //        stream << "\\clef bass ";
        //        is_bass_clef = true;
        //      } else if (!is_low_note && is_bass_clef) {
        //        stream << "\\clef treble ";
        //        is_bass_clef = false;
        //      }
        //    }
        stream << notation[m_score_notes[index]];
        if (index == 0) // set length of first note (rest will follow)
            stream << 1;
        stream << ' ';
    }
    // add invisible rests at the end of last line
    if (index % notes_per_staff != 0) {
        do {
            stream << "s ";
        } while (++index % notes_per_staff != 0);
    }
    stream << m_settings->lilypondFooter();
    stream.flush();
    lilypond_file.close();

    m_process = new QProcess();
    QStringList config;
    config << "--png";
    config << "-dresolution=" + QString::number(m_settings->dpi());
    config << "-o" << directory_path + "score";
    config << directory_path + "score.ly";

    connect(m_process, qOverload<int>(&QProcess::finished), m_process, &QProcess::deleteLater);
    connect(m_process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &Lilypond::finish);

    QFileInfo check_lilypond("/usr/bin/lilypond");
    if (!check_lilypond.exists()) {
        qCritical() << "No file \"/usr/bin/lilypond\". Lilypond may be not installed.";
        return;
    }

    m_process->start("/usr/bin/lilypond", config);
}

void Lilypond::finish(int exit_code, QProcess::ExitStatus exit_status)
{
    if (exit_code != 0 || exit_status != QProcess::NormalExit) {
        qWarning() << "lilypond error:";
        qWarning().nospace() << QString::fromStdString(m_process->readAllStandardError().toStdString());
    }
    auto pages = countPages();
    auto ys = calculateIndicatorYs(pages);
    emit finishedGeneratingScore(pages, ys);
}

int Lilypond::countPages() const
{
    QDir dir(m_settings->lilypondWorkingDirectory());
    dir.setNameFilters(QStringList() << "*.png");
    dir.setFilter(QDir::Files);
    return (dir.entryList().size() - 1);
}

QVector<QVector<int>> Lilypond::calculateIndicatorYs(int pages_number) const
{
    QVector<QVector<int>> indicator_ys;
    for (int i = 1; i <= pages_number; i++) {
        QString page_file_name = m_settings->lilypondWorkingDirectory() + "score-page" + QString::number(i) + ".png";
        QFileInfo check_file(page_file_name);
        if (!check_file.exists() || !check_file.isFile()) {
            qWarning() << "Score file does not exists: " << page_file_name;
            break;
        }

        indicator_ys.push_back({}); // new page

        QImage image(page_file_name);
        bool last_was_white = true;
        int counter = 0;
        for (int y = image.height(); y >= 0; y--) {
            QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
            QColor color(line[m_settings->staffIndent()]);

            if(color == Qt::white) {
                last_was_white = true;
            } else {
                if (!last_was_white)
                    continue;
                last_was_white = false;
                if (++counter == 5)
                    indicator_ys.back().push_back(y);
                counter %= 5;
            }
        }
        std::sort(indicator_ys.back().begin(), indicator_ys.back().end());
    }

    return indicator_ys;
}
