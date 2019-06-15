// Author:  Jakub Precht

#ifndef SCOREREADER_H
#define SCOREREADER_H

#include <QString>

class ScoreReader
{
public:
    virtual QVector<int> readScoreFile(const QString &filename);
    virtual ~ScoreReader() = default;

private:
    static QVector<int> readMidiFile(const QString &filename);
    static QVector<int> readTextFile(const QString &filename);

    static int m_midi_note_on_action_code;
};

#endif // SCOREREADER_H
