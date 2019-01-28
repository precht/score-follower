#include "scorereader.h"
#include <QVector>
#include <QFileInfo>
#include <QFile>
#include <QDebug>

#include "MidiFile.h"
#include <iostream>
#include <iomanip>

int ScoreReader::_midiNoteOnActionCode = 144;

QVector<int> ScoreReader::readScoreFile(const QString &filename)
{
  QFileInfo info(filename);
  auto fileExtension = info.suffix();
  if (!info.exists()) {
    qDebug() << "File don't exists: " << filename;
    return QVector<int>();
  }

  if (fileExtension == "mid")
    return readMidiFile(filename);
  else
    return readTextFile(filename);
}

QVector<int> ScoreReader::readTextFile(const QString &filename)
{
  QVector<int> scoreNotes;
  QFile scoreFile(filename);
  if (scoreFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream in(&scoreFile);
    QString noteNumber;
    while (!in.atEnd()) {
      in >> noteNumber;
      if (!noteNumber.isEmpty())
        scoreNotes.push_back(noteNumber.toInt());
    }
    scoreFile.close();
  } else {
    qDebug() << "Failed to open: " << filename;
  }
  return scoreNotes;
}

QVector<int> ScoreReader::readMidiFile(const QString &filename)
{
  smf::MidiFile midifile;
  midifile.read(filename.toStdString());
  if (midifile.getTrackCount() > 1) {
    qDebug() << "Failed reading midi score - it contains multiple tracks";
    return QVector<int>();
  }

  QVector<int> scoreNotes;
  const int track = 0;
  for (int event = 0; event < midifile[track].size(); event++) {
    if (midifile[track][event].size() < 3)
      continue;
    int action = midifile[track][event][0];
    int note = midifile[track][event][1];
    int volume = midifile[track][event][2];
    if (action == _midiNoteOnActionCode && volume > 0)
      scoreNotes.append(note);
  }

  return scoreNotes;
}
