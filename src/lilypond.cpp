#include "include/lilypond.h"
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QUrl>
#include <QDir>
#include <QDirIterator>
#include <QImage>

Lilypond::Lilypond(QObject *parent)
  : QObject(parent)
{
  loadFiles();
  if (!QDir(_scoreFolderName).exists())
    QDir().mkdir(_scoreFolderName);
}

void Lilypond::setScore(const QVector<int> &scoreNotes)
{
  _scoreNotes = scoreNotes;
}

void Lilypond::generateScore()
{
  // delete old files
  QDir dir("/tmp/score-follower/");
  dir.setNameFilters(QStringList() << "*.*");
  dir.setFilter(QDir::Files);
  for (auto &dirFile : dir.entryList())
      dir.remove(dirFile);

  QFile lilypondFile(_lilypondFileName);
  if (!lilypondFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
    qDebug() << "Failed to open: " << _lilypondFileName;
    emit finishedGeneratingScore(0);
  }

  QTextStream out(&lilypondFile);
  out << _header;
  int index = 0;
//  bool isBassClef = false;
  for (; index < _scoreNotes.size(); index++) {
    if (index % _notesPerLine == 0 && index > 0) {
      out << "\\break\n";
    }
    // switch bass and tremble clef
//    if (index % _notesPerLine == 0) {
//      bool isLowNote = false;
//      for (int j = 0; j < _notesPerLine && index + j < _scoreNotes.size(); j++)
//        if (_scoreNotes[index + j] < 48)
//          isLowNote = true;
//      if (isLowNote && !isBassClef) {
//        out << "\\clef bass ";
//        isBassClef = true;
//      } else if (!isLowNote && isBassClef) {
//        out << "\\clef treble ";
//        isBassClef = false;
//      }
//    }
    out << _notes[_scoreNotes[index]];
    if (index == 0) // set length of first note (rest will follow)
      out << 1;
    out << ' ';
  }
  // add invisible rests at the end of last line
  if (index % _notesPerLine != 0) {
    do {
      out << "s ";
    } while (++index % _notesPerLine != 0);
  }
  out << _footer;
  out.flush();
  lilypondFile.close();

  QProcess *process = new QProcess();
  QStringList config;
  config << "--png";
  config << "-dresolution=" + QString::number(_dpi);
  config << "-o" << _scoreFolderName + _scoreFileName;
  config << _lilypondFileName;

  connect(process, qOverload<int>(&QProcess::finished), process, &QProcess::deleteLater);
  connect(process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
          [=](int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitCode != 0 || exitStatus != QProcess::NormalExit)
      qInfo() << "lilypond error:" << process->readAllStandardError();
    emit finishedGeneratingScore(countPages());
  });

  process->start("/usr/bin/lilypond", config);
}

void Lilypond::loadFiles()
{
  // header
  QFile headerFile(_headerFileName);
  if (headerFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    _header = headerFile.readAll();
    headerFile.close();
  } else {
    qDebug() << "Failed to open: " << _headerFileName;
  }

  // footer
  QFile footerFile(_footerFileName);
  if (footerFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    _footer = footerFile.readAll();
    footerFile.close();
  } else {
    qDebug() << "Failed to open: " << _footerFileName;
  }

  // color changer
  QFile colorChangerFile(_colorChangerFileName);
  if (colorChangerFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    _colorChanger = colorChangerFile.readAll();
    colorChangerFile.close();
  } else {
    qDebug() << "Failed to open: " << _colorChangerFileName;
  }

  // notes lilypond
  QFile notesFile(_notesFileName);
  if (notesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream notesInput(&notesFile);
    int noteNumber;
    QString noteString;
    _notes.clear();
    while (!notesInput.atEnd()) {
      notesInput >> noteNumber >> noteString;
      if (!noteString.isEmpty()) {
        _notes[noteNumber] = noteString;
        // qInfo() << noteNumber << noteString;
      }
    }
    notesFile.close();
  } else {
    qDebug() << "Failed to open: " << _notesFileName;
  }
}

int Lilypond::countPages()
{
  QDir dir("/tmp/score-follower/");
  dir.setNameFilters(QStringList() << "*.png");
  dir.setFilter(QDir::Files);
  return (dir.entryList().size() - 1);
}
