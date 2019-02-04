// Author:  Jakub Precht

#include "lilypond.h"
#include "settings.h"

#include <QProcess>
#include <QDebug>
#include <QDir>
#include <QImage>

Lilypond::Lilypond(QObject *parent)
  : QObject(parent)
{ }

void Lilypond::setScore(const QVector<int> &scoreNotes)
{
  _scoreNotes = scoreNotes;
}

void Lilypond::setSettings(const Settings *settings)
{
  _settings = settings;
}

void Lilypond::generateScore()
{
  // create directory
  const QString directoryPath = _settings->lilypondWorkingDirectory();
  if (!QDir(directoryPath).exists())
    QDir().mkdir(directoryPath);

  // delete old files
  QDir directory(directoryPath);
  directory.setNameFilters(QStringList() << "*.*");
  directory.setFilter(QDir::Files);
  for (auto &dirFile : directory.entryList())
    directory.remove(dirFile);

  QFile lilypondFile(directoryPath + "score.ly");
  if (!lilypondFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
    qDebug() << "Failed to open: " << directoryPath + "score.ly";
    return;
  }

  QTextStream stream(&lilypondFile);
  stream << _settings->lilypondHeader();
  int index = 0;
  const int notesPerStaff = _settings->notesPerStaff();
  //bool isBassClef = false;
  auto &notation = _settings->lilypondNotesNotation();
  for (; index < _scoreNotes.size(); index++) {
    if (index % notesPerStaff == 0 && index > 0) {
      stream << "\\break\n";
    }
    // switch bass and tremble clef
    //if (index % _settings->notesPerStaff() == 0) {
    //  bool isLowNote = false;
    //  for (int j = 0; j < notesPerStaff && index + j < _scoreNotes.size(); j++)
    //    if (_scoreNotes[index + j] < 48)
    //      isLowNote = true;
    //  if (isLowNote && !isBassClef) {
    //    stream << "\\clef bass ";
    //    isBassClef = true;
    //  } else if (!isLowNote && isBassClef) {
    //    stream << "\\clef treble ";
    //    isBassClef = false;
    //  }
    //}
    stream << notation[_scoreNotes[index]];
    if (index == 0) // set length of first note (rest will follow)
      stream << 1;
    stream << ' ';
  }
  // add invisible rests at the end of last line
  if (index % notesPerStaff != 0) {
    do {
      stream << "s ";
    } while (++index % notesPerStaff != 0);
  }
  stream << _settings->lilypondFooter();
  stream.flush();
  lilypondFile.close();

  _process = new QProcess();
  QStringList config;
  config << "--png";
  config << "-dresolution=" + QString::number(_settings->dpi());
  config << "-o" << directoryPath + "score";
  config << directoryPath + "score.ly";

  connect(_process, qOverload<int>(&QProcess::finished), _process, &QProcess::deleteLater);
  connect(_process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &Lilypond::finish);

  QFileInfo checkLilypond("/usr/bin/lilypond");
  if (!checkLilypond.exists()) {
    qCritical() << "No file \"/usr/bin/lilypond\". Lilypond may be not installed.";
    return;
  }

  _process->start("/usr/bin/lilypond", config);
}

void Lilypond::finish(int exitCode, QProcess::ExitStatus exitStatus)
{
  if (exitCode != 0 || exitStatus != QProcess::NormalExit) {
    qWarning() << "lilypond error:";
    qWarning().nospace() << QString::fromStdString(_process->readAllStandardError().toStdString());
  }
  auto pages = countPages();
  auto ys = calculateIndicatorYs(pages);
  emit finishedGeneratingScore(pages, ys);
}

int Lilypond::countPages() const
{
  QDir dir(_settings->lilypondWorkingDirectory());
  dir.setNameFilters(QStringList() << "*.png");
  dir.setFilter(QDir::Files);
  return (dir.entryList().size() - 1);
}

QVector<QVector<int>> Lilypond::calculateIndicatorYs(int pagesNumber) const
{
  QVector<QVector<int>> indicatorYs;
  for (int i = 1; i <= pagesNumber; i++) {
    QString pageFileName = _settings->lilypondWorkingDirectory() + "score-page" + QString::number(i) + ".png";
    QFileInfo checkFile(pageFileName);
    if (!checkFile.exists() || !checkFile.isFile()) {
      qWarning() << "Score file does not exists: " << pageFileName;
      break;
    }

    indicatorYs.push_back({}); // new page

    QImage image(pageFileName);
    bool lastWasWhite = true;
    int counter = 0;
    for (int y = image.height(); y >= 0; y--) {
      QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
      QColor color(line[_settings->staffIndent()]);

      if(color == Qt::white) {
        lastWasWhite = true;
      } else {
        if (!lastWasWhite)
          continue;
        lastWasWhite = false;
        if (++counter == 5)
          indicatorYs.back().push_back(y);
        counter %= 5;
      }
    }
    std::sort(indicatorYs.back().begin(), indicatorYs.back().end());
  }

  return indicatorYs;
}
