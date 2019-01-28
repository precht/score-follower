#include "controller.h"
#include "scorereader.h"
#include <QDebug>
#include <QVector>
#include <QTimer>
#include <QFile>
#include <QFileDialog>
#include <QStandardPaths>

Controller::Controller(QObject *parent)
  : QObject(parent), _lilypond(new Lilypond()), _recorder(new Recorder())
{
  initializeIndicatorSettings();

  _lilypond->moveToThread(&_lilypondThread);
  _recorder->moveToThread(&_recorderThread);

  connect(this, &Controller::startRecording, _recorder, &Recorder::startRecording);
  connect(this, &Controller::stopRecording, _recorder, &Recorder::stopRecording);
  connect(this, &Controller::generateScore, _lilypond, &Lilypond::generateScore);
  connect(_recorder, &Recorder::positionChanged, [=](int position){ setPlayedNotes(position); });
  connect(_recorder, &Recorder::levelChanged, [=](float level){ setLevel(level); });
  connect(_lilypond, &Lilypond::finishedGeneratingScore,
          [=](int pagesCount){
    setPagesNumber(pagesCount);
    calculateIndicatorYs();
    emit updateScore();
  });

  connect(&_timer, &QTimer::timeout, [=](){ emit generateScore(); });

  _recorderThread.start();
  _lilypondThread.start();
}

Controller::~Controller()
{
  _lilypondThread.quit();
  _recorderThread.quit();
  QThread::msleep(100);
}

void Controller::openScore()
{
  _scoreFileName = QFileDialog::getOpenFileName(nullptr, "Open Image",
                                                QStandardPaths::writableLocation(QStandardPaths::MusicLocation));
  if (_scoreFileName == "")
    return ;

  QVector<int> scoreNotes = ScoreReader::readScoreFile(_scoreFileName);
  _lilypond->setScore(scoreNotes);
  _recorder->setScore(scoreNotes);
  emit generateScore();
}

int Controller::playedNotes() const
{
  return _playedNotes;
}

void Controller::setPlayedNotes(int playedNotes)
{
  if (playedNotes == _playedNotes)
    return;

  _playedNotes = playedNotes;
  emit playedNotesChanged(playedNotes);
}

int Controller::indicatorHeight() const
{
  return _indicatorHeight;
}

void Controller::setIndicatorHeight(int indicatorHeight)
{
  _indicatorHeight = indicatorHeight;
  emit indicatorHeightChanged();
}

int Controller::indicatorWidth() const
{
  return _indicatorWidth;
}

void Controller::setIndicatorWidth(int indicatorWidth)
{
  _indicatorWidth = indicatorWidth;
  emit indicatorWidthChanged();
}

double Controller::indicatorScale() const
{
  return _indicatorScale;
}

void Controller::setIndicatorScale(double indicatorScale)
{
  _indicatorScale = indicatorScale;
  emit indicatorScaleChanged();
}

void Controller::initializeIndicatorSettings()
{
  QFile settingsFile(_indicatorSettingsFileName);
  if (settingsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream in(&settingsFile);
    int x;
    if (!in.atEnd()) {
      in >> x;
      setIndicatorWidth(x);
    }
    if (!in.atEnd()) {
      in >> x;
      setIndicatorHeight(x);
    }
    if (!in.atEnd()) {
      in >> x;
      _staffIndent = x;
    }
    if (!in.atEnd()) {
      in >> x;
      _indicatorXs.fill(0, x);
    }
    if (!in.atEnd()) {
      in >> x;
      //      _indicatorYs.fill(0, x);
      // TODO for each page?
    }
    int index = 0;
    for (; index < _indicatorXs.size() && !in.atEnd(); index++)
      in >> _indicatorXs[index];

    if (index != _indicatorXs.size()) {
      qWarning() << "Wrong number of indicator positions in file:" << _indicatorSettingsFileName;
    }
    settingsFile.close();
  } else {
    qDebug() << "Failed to open: " << _indicatorSettingsFileName;
  }

  if (_indicatorXs.size() > 1) {
    int shift = (_indicatorXs[1] - _indicatorXs[0]) / 2;
    for (auto &x : _indicatorXs)
      x += shift;
  }
}

bool Controller::follow() const
{
  return _follow;
}

void Controller::setFollow(bool follow)
{
  _follow = follow;

  if (follow == true) {
    emit startRecording();
    //    _timer.start(scoreGenerateInterval);
  } else {
    emit stopRecording();
    //    _timer.stop();
  }

  emit followChanged();
}

float Controller::level() const
{
  return _level;
}

void Controller::setLevel(float level)
{
  _level = level;
  emit levelChanged();
}

void Controller::calculateIndicatorYs()
{
  _indicatorYs.clear();
  for (int i = 1;; i++) {
    QString pageFileName = _scoreImagePath + "-page" + QString::number(i) + ".png";
    QFileInfo checkFile(pageFileName);
    if (!checkFile.exists() || !checkFile.isFile())
      break;

    _indicatorYs.push_back({ });

    QImage image(pageFileName);
    bool lastWasWhite = true;
    int counter = 0;

    for (int y = 0; y < image.height(); y++) {
      QRgb *line = (QRgb *) image.scanLine(y);
      QColor col(line[_staffIndent]);

      if(col == Qt::white) {
        lastWasWhite = true;
      } else {
        if (!lastWasWhite)
          continue;
        lastWasWhite = false;
        if (counter == 0)
          _indicatorYs.back().push_back(y);
        counter = (counter + 1) % 5;
      }
    }
  }
}

int Controller::indicatorX(int index)
{
  if (index < 0 || _indicatorXs.size() == 0) {
    qWarning() << "wrong indicator x index or empty x indicator vector";
    return 0;
  }
  return _indicatorXs[index % _indicatorXs.size()];
}

int Controller::indicatorY(int index)
{
  // TODO support for multiple pages
  if (_indicatorXs.size() == 0 || _indicatorYs.size() == 0) {
    qWarning() << "Empty indicator vectors";
    return 0;
  }
  int y = index / _indicatorXs.size();
  if (y < 0 || y >= _indicatorYs.front().size()) {
//    qWarning() << "wrong indicator y index:" << index;
    return 0;
  }
  return _indicatorYs.front()[y];
}

int Controller::notesPerPage() const
{
  return _notesPerPage;
}

int Controller::pagesNumber() const
{
  return _pagesNumber;
}

void Controller::setPagesNumber(int pagesNumber)
{
  if (_pagesNumber == pagesNumber)
    return;

  _pagesNumber = pagesNumber;
  emit pagesNumberChanged(_pagesNumber);
}
