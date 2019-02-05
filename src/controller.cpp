// Author:  Jakub Precht

#include "controller.h"
#include "scorereader.h"
#include "settings.h"

#include <QDebug>
#include <QVector>
#include <QTimer>
#include <QFile>
#include <QFileDialog>
#include <QStandardPaths>

Controller::Controller(bool verbose, QObject *parent)
  : QObject(parent), _lilypond(new Lilypond()), _recorder(new Recorder())
{
  Settings *settings = new Settings();
  settings->setVerbose(verbose);
  _status = settings->readSettings();
  if (!_status)
    return;
  _settings = settings;
  _lilypond->setSettings(settings);
  _recorder->setSettings(settings);
  _status &= _recorder->initialize();

  _lilypond->moveToThread(&_lilypondThread);
  _recorder->moveToThread(&_recorderThread);

  connect(this, &Controller::startRecording, _recorder, &Recorder::startFollowing);
  connect(this, &Controller::stopRecording, _recorder, &Recorder::stopFollowing);
  connect(this, &Controller::generateScore, _lilypond, &Lilypond::generateScore);
  connect(_recorder, &Recorder::positionChanged, [=](int position){ setPlayedNotes(position); });
  connect(_recorder, &Recorder::levelChanged, [=](float level){ setLevel(level); });

  connect(_lilypond, &Lilypond::finishedGeneratingScore, [=](int pagesCount, QVector<QVector<int>> indicatorYs){
    _currentPage = 1;
    _indicatorYs = indicatorYs;
    setPagesNumber(pagesCount);
    emit currentPageChanged();
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
  delete _settings;
}

bool Controller::createdSuccessfully() const
{
  return _status;
}

bool Controller::openScore()
{
  _toOpenFilename = QFileDialog::getOpenFileName(nullptr, "Open Score",
                                                 QStandardPaths::writableLocation(QStandardPaths::MusicLocation),
                                                 "Score Files (*.txt *.mid);; All Files (*.*)");
  if (_toOpenFilename == "")
    return false;

  QVector<int> scoreNotes = ScoreReader::readScoreFile(_toOpenFilename);
  _lilypond->setScore(scoreNotes);
  _recorder->setScore(scoreNotes);
  setScoreLength(scoreNotes.size());

  emit generateScore();
  return true;
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
  updateCurrentPage();
  emit playedNotesChanged();
}

int Controller::indicatorHeight() const
{
  return _settings->indicatorHeight();
}

int Controller::indicatorWidth() const
{
  return _settings->indicatorWidth();
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

bool Controller::follow() const
{
  return _follow;
}

void Controller::setFollow(bool follow)
{
  if (_follow == follow)
    return;

  _follow = follow;
  if (follow == true) {
    emit startRecording();
  } else {
    emit stopRecording();
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

int Controller::indicatorX(int index)
{
  if (index < 0 || _settings->indicatorXs().size() == 0) {
    qWarning() << "wrong indicator x index or empty x indicator vector";
    return 0;
  }
  return _settings->indicatorXs()[index % _settings->indicatorXs().size()];
}

int Controller::indicatorY(int index)
{
  if (_settings->indicatorXs().size() == 0 || _indicatorYs.size() == 0) {
    qWarning() << "Empty indicator vectors";
    return 0;
  }

  int page = 0;
  while (page < _pagesNumber && index >= _indicatorYs[page].size() * _settings->indicatorXs().size()) {
    index -= _indicatorYs[page].size() * _settings->indicatorXs().size();
    page++;
  }

  int y = index / _settings->indicatorXs().size();
  if (y < 0 || y >= _indicatorYs[page].size()) {
    qWarning() << "wrong indicator y index:" << index;
    return 0;
  }
  return _indicatorYs[page][y];
}

int Controller::pagesNumber() const
{
  return _pagesNumber;
}

int Controller::scoreLength() const
{
  return _scoreLength;
}

int Controller::currentPage() const
{
  return _currentPage;
}

void Controller::setPagesNumber(int pagesNumber)
{
  if (_pagesNumber == pagesNumber)
    return;

  _pagesNumber = pagesNumber;
  emit pagesNumberChanged();
}

void Controller::setScoreLength(int scoreLength)
{
  if (_scoreLength == scoreLength)
    return;

  _scoreLength = scoreLength;
  emit scoreLengthChanged();
}

void Controller::updateCurrentPage()
{
  int index = _playedNotes;
  int page = 0;
  while (page < _pagesNumber && index > _indicatorYs[page].size() * _settings->indicatorXs().size()) {
    index -= _indicatorYs[page].size() * _settings->indicatorXs().size();
    page++;
  }
  if (page + 1!= _currentPage) {
    _currentPage = page + 1;
    emit currentPageChanged();
  }
}

void Controller::resetPageAndPosition()
{
  setPlayedNotes(0);
  _currentPage = 1;
  emit currentPageChanged();
}
