#include "include/lilypond.h"
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QUrl>
#include <QDir>
#include <QDirIterator>

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

void Lilypond::setPosition(int position)
{
  _numberOfPlayedNotes = position + 1;
  qInfo() << "position" << position + 1;
}

void Lilypond::generateScore()
{
  if (_numberOfPlayedNotes == _previousNumberOfPlayedNotes)
    return;
  _previousNumberOfPlayedNotes = _numberOfPlayedNotes;

  QFile lilypondFile(_lilypondFileName);
  if (!lilypondFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
    qDebug() << "Failed to open: " << _lilypondFileName;
    emit finishedGeneratingScore();
  }

  QTextStream out(&lilypondFile);
  out << _header;
  for (int index = 0; index < _scoreNotes.size(); index++) {
    if (index % _notesPerLine == 0 && index > 0)
      out << "\\break\n";
    if (index == _numberOfPlayedNotes)
      out << _colorChanger;
    out << _notes[_scoreNotes[index]];
    if (index == 0)
      out << 1;
    out << ' ';
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
          [=](int exitCode, QProcess::ExitStatus) {
    if (exitCode != 0)
      qInfo() << "lilypond error:" << process->readAllStandardError();
    emit finishedGeneratingScore();
  });

  process->start("/usr/bin/lilypond", config);
}

void Lilypond::setPositionGenerateScore(int position)
{
  setPosition(position);
  generateScore();
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
