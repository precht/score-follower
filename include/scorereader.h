// Author:  Jakub Precht

#ifndef SCOREREADER_H
#define SCOREREADER_H

#include <QString>

class ScoreReader
{
public:
  static QVector<int> readScoreFile(const QString &filename);

private:
  static QVector<int> readMidiFile(const QString &filename);
  static QVector<int> readTextFile(const QString &filename);

  static int _midiNoteOnActionCode;
};

#endif // SCOREREADER_H
