#include "include/lilypond.h"
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QUrl>
#include <QDirIterator>

Lilypond::Lilypond(QObject *parent)
  : QObject(parent)
{
  loadFiles();
}

void Lilypond::generateScore(const QVector<int> &scoreNotes, int numberOfPlayedNotes)
{
  QFile lilypondFile(m_lilypondFileName);
  if (!lilypondFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
    qDebug() << "Failed to open: " << m_lilypondFileName;
    emit finishedGeneratingScore();
  }

  QTextStream out(&lilypondFile);
  out << m_header;
  for (int index = 0; index < scoreNotes.size(); index++) {
    if (index % m_notesPerLine == 0 && index > 0)
      out << "\\break\n";
    if (index == numberOfPlayedNotes)
      out << m_colorChanger;
    out << m_notes[scoreNotes[index]];
    if (index == 0)
      out << 1;
    out << ' ';
  }
  out << m_footer;
  out.flush();
  lilypondFile.close();

  // remove existing files
//  QDir dir(m_scoreFolderName);
//  dir.setNameFilters(QStringList() << "*.*");
//  dir.setFilter(QDir::Files);
//  foreach(QString dirFile, dir.entryList())
//    dir.remove(dirFile);

  QProcess *process = new QProcess();
  QStringList config;
  config << "--png";
  config << "-dresolution=" + QString::number(m_dpi);
  config << "-o" << m_scoreFolderName + m_scoreFileName;
  config << m_lilypondFileName;

  connect(process, qOverload<int>(&QProcess::finished), process, &QProcess::deleteLater);
  connect(process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
          [=](int, QProcess::ExitStatus){ emit finishedGeneratingScore(); });

  process->start("lilypond", config);
}

//void Lilypond::finishGeneratingScore(int exitCode, QProcess::ExitStatus exitStatus)
//{
//  if (exitStatus == QProcess::CrashExit || exitCode != 0)
//    qDebug() << "Failed to generate score with lilypond. Exit status: " << exitStatus << ", exit code: " << exitCode;
//  emit finishedGeneratingScore();
//}

void Lilypond::loadFiles()
{
  // header
  QFile headerFile(m_headerFileName);
  if (headerFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    m_header = headerFile.readAll();
    headerFile.close();
  } else {
    qDebug() << "Failed to open: " << m_headerFileName;
  }

  // footer
  QFile footerFile(m_footerFileName);
  if (footerFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    m_footer = footerFile.readAll();
    footerFile.close();
  } else {
    qDebug() << "Failed to open: " << m_footerFileName;
  }

  // color changer
  QFile colorChangerFile(m_colorChangerFileName);
  if (colorChangerFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    m_colorChanger = colorChangerFile.readAll();
    colorChangerFile.close();
  } else {
    qDebug() << "Failed to open: " << m_colorChangerFileName;
  }

  // notes lilypond
  QFile notesFile(m_notesFileName);
  if (notesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream notesInput(&notesFile);
    int noteNumber;
    QString noteString;
    m_notes.clear();
    while (!notesInput.atEnd()) {
      notesInput >> noteNumber;
      notesInput >> noteString;
      if (!noteString.isEmpty()) {
        m_notes[noteNumber] = noteString;
        // qInfo() << noteNumber << noteString;
      }
    }
    notesFile.close();
  } else {
    qDebug() << "Failed to open: " << m_notesFileName;
  }
}

