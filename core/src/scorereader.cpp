// Author:  Jakub Precht

#include "scorereader.h"

#include "MidiFile.h"

#include <QVector>
#include <QFileInfo>
#include <QFile>
#include <QDebug>

int ScoreReader::m_midi_note_on_action_code = 144;

QVector<int> ScoreReader::readScoreFile(const QString &filename)
{
    QFileInfo info(filename);
    auto file_extension = info.suffix();
    if (!info.exists()) {
        qDebug() << "File don't exists: " << filename;
        return QVector<int>();
    }

    if (file_extension == "mid")
        return readMidiFile(filename);
    else
        return readTextFile(filename);
}

QVector<int> ScoreReader::readTextFile(const QString &filename)
{
    QVector<int> score_notes;
    QFile score_file(filename);
    if (score_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&score_file);
        QString note_number;
        while (!in.atEnd()) {
            in >> note_number;
            if (!note_number.isEmpty())
                score_notes.push_back(note_number.toInt());
        }
        score_file.close();
    } else {
        qDebug() << "Failed to open: " << filename;
    }
    return score_notes;
}

QVector<int> ScoreReader::readMidiFile(const QString &filename)
{
    smf::MidiFile midifile;
    midifile.read(filename.toStdString());
    if (midifile.getTrackCount() > 1) {
        qDebug() << "Failed reading midi score - it contains multiple tracks";
        return QVector<int>();
    }

    QVector<int> score_notes;
    const int track = 0;
    for (int event = 0; event < midifile[track].size(); event++) {
        if (midifile[track][event].size() < 3)
            continue;
        int action = midifile[track][event][0];
        int note = midifile[track][event][1];
        int volume = midifile[track][event][2];
        if (action == m_midi_note_on_action_code && volume > 0)
            score_notes.push_back(note);
    }

    return score_notes;
}
