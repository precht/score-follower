// Author:  Jakub Precht

#include "recorder.h"
#include "settings.h"

#include <QThread>
#include <QUrl>
#include <qendian.h>
#include <QDateTime>

#include <essentia/algorithmfactory.h>

using namespace essentia;
using namespace standard;

Recorder::Recorder(QObject *parent)
  : QObject(parent)
{
  essentia::init();
}

bool Recorder::initialize()
{
  initializePitchDetector();

  _recorder = new QAudioRecorder(this);
  _recorderSettings.setChannelCount(1);
  _recorderSettings.setSampleRate(_settings->sampleRate());
  _recorder->setEncodingSettings(_recorderSettings);
  _recorder->setOutputLocation(QString("/dev/null"));

  _probe = new QAudioProbe(this);
  connect(_probe, &QAudioProbe::audioBufferProbed, this, &Recorder::processBuffer);
  _probe->setSource(_recorder);

  _recorder->record();
  if (_recorder->state() != QAudioRecorder::RecordingState) {
    qWarning().nospace() << "Failed to create audio input device. Propably unsupported sample rate "
                         << _settings->sampleRate() << ".";
    return false;
  } else {
    return true;
  }
}

void Recorder::initializePitchDetector()
{
  AlgorithmFactory& factory = AlgorithmFactory::instance();

  _windowCalculator = factory.create("Windowing", "type", "hann", "zeroPadding", 0);
  _spectrumCalculator = factory.create("Spectrum", "size", _settings->frameSize());
  _pitchDetector = factory.create("PitchYinFFT",
                                  "frameSize", _settings->frameSize(),
                                  "sampleRate", _settings->sampleRate());

  _windowCalculator->input("frame").set(_audio);
  _windowCalculator->output("frame").set(_windowedframe);

  _spectrumCalculator->input("frame").set(_windowedframe);
  _spectrumCalculator->output("spectrum").set(_spectrum);

  _pitchDetector->input("spectrum").set(_spectrum);
  _pitchDetector->output("pitch").set(_currentPitch);
  _pitchDetector->output("pitchConfidence").set(_currentConfidence);
}

void Recorder::processBuffer(const QAudioBuffer buffer)
{
//    qInfo() << QDateTime::currentDateTime() << buffer.sampleCount();

  if (buffer.format() != _currentFormat) {
    _currentFormat = buffer.format();
    setMaxAmplitude(buffer.format());
  }

  updateLevel(buffer);
  if (!_isFollowing)
    return;

  convertBufferToAudio(buffer);
  if (_audio.size() < static_cast<size_t>(_settings->frameSize()))
    return;

  bool isSkipped = false;
  float skippedValue = 0;
  if (_audio.size() % 2 == 1) {
    skippedValue = _audio.back();
    _audio.pop_back();
    isSkipped = true;
  }

  _windowCalculator->compute();
  _spectrumCalculator->compute();
  _pitchDetector->compute();

  auto &notesBoundry = _settings->notesFrequencyBoundry();
  if (_currentPitch < notesBoundry[_noteNumber].first || _currentPitch > notesBoundry[_noteNumber].second) {
    int newNoteNumber = findNoteFromPitch(_currentPitch);

    if (_currentConfidence >= _settings->minimalConfidence()[newNoteNumber]) {
      _noteNumber = newNoteNumber;
      calculatePosition();
      qInfo().nospace() << "Detected note " << newNoteNumber << ".";
    }
//    else {
//      qInfo().nospace() << "Note " << newNoteNumber << " skipped (" << _currentConfidence << " < "
//                        << _settings->minimalConfidence()[newNoteNumber] << ").";
//    }
  }

  _audio.erase(_audio.begin(), _audio.begin() + _settings->hopSize());
  if (isSkipped)
    _audio.push_back(skippedValue);
}

void Recorder::updateLevel(const QAudioBuffer &buffer)
{
  const unsigned char *ptr = reinterpret_cast<const unsigned char*>(buffer.data());
  const auto format = buffer.format();

  if (format.sampleSize() == 8) {
    _level += qAbs(*reinterpret_cast<const qint8*>(ptr) / _maxAmplitude);
  } else if (format.sampleSize() == 16 ) {
    if (format.byteOrder() == QAudioFormat::LittleEndian)
      _level += qAbs(qFromLittleEndian<qint16>(ptr) / _maxAmplitude);
    else
      _level += qAbs(qFromBigEndian<qint16>(ptr) / _maxAmplitude);
  } else if (format.sampleSize() == 32 && (format.sampleType() == QAudioFormat::UnSignedInt
                                           || format.sampleType() == QAudioFormat::SignedInt)) {
    if (format.byteOrder() == QAudioFormat::LittleEndian)
      _level += qAbs(qFromLittleEndian<quint32>(ptr) / _maxAmplitude);
    else
      _level += qAbs(qFromBigEndian<quint32>(ptr) / _maxAmplitude);
  } else if (format.sampleSize() == 32 && format.sampleType() == QAudioFormat::Float) {
    _level += qAbs(*reinterpret_cast<const float*>(ptr) / _maxAmplitude);
  }

  _levelCount++;
  if (_levelCount == _maxLevelCount) {
    emit levelChanged(_level / _maxLevelCount);
    _level = 0;
    _levelCount = 0;
  }
}

void Recorder::convertBufferToAudio(const QAudioBuffer &buffer)
{
  const unsigned char *ptr = reinterpret_cast<const unsigned char*>(buffer.data());
  const auto format = buffer.format();
  const int channelBytes = format.sampleSize() / 8;

  for (int i = 0; i < buffer.sampleCount(); i++) {
    float value = 0;

    if (format.sampleSize() == 8) {
      value = *reinterpret_cast<const qint8*>(ptr);
    } else if (format.sampleSize() == 16 ) {
      if (format.byteOrder() == QAudioFormat::LittleEndian)
        value = qFromLittleEndian<qint16>(ptr);
      else
        value = qFromBigEndian<qint16>(ptr);
    } else if (format.sampleSize() == 32 && (format.sampleType() == QAudioFormat::UnSignedInt
                                             || format.sampleType() == QAudioFormat::SignedInt)) {
      if (format.byteOrder() == QAudioFormat::LittleEndian)
        value = qFromLittleEndian<quint32>(ptr);
      else
        value = qFromBigEndian<quint32>(ptr);
    } else if (format.sampleSize() == 32 && format.sampleType() == QAudioFormat::Float) {
      value = *reinterpret_cast<const float*>(ptr);
    } else {
      qDebug() << "Unsupported audio format.";
    }

    _audio.push_back(value);
    ptr += channelBytes;
  }
}

void Recorder::resetDtw()
{
  _position = -1;
  _dtwRow.fill(0);
}

void Recorder::calculatePosition()
{
  // dtw algorithm
  _nextRow[0] = qAbs(_noteNumber - _scoreNotes[0]) + _dtwRow[0];

  int position = 0;
  int64_t minValue = _nextRow[0];
  for (int i = 1; i < _dtwRow.size(); i++) {
    _nextRow[i] = qAbs(_noteNumber - _scoreNotes[i]) + qMin(_nextRow[i - 1], qMin(_dtwRow[i], _dtwRow[i-1]));
    if (_nextRow[i] < minValue) {
      position = i;
      minValue = _nextRow[i];
    }
  }

  _dtwRow.swap(_nextRow); // fast swap
  if (position != _position)
    emit positionChanged(position + 1);
  _position = position;
}

void Recorder::setSettings(const Settings *settings)
{
  _settings = settings;
}

void Recorder::setMaxAmplitude(const QAudioFormat &format)
{
  switch (format.sampleSize()) {
  case 8:
    switch (format.sampleType()) {
    case QAudioFormat::UnSignedInt:
      _maxAmplitude = 255;
      break;
    case QAudioFormat::SignedInt:
      _maxAmplitude = 127;
      break;
    default:
      break;
    }
    break;
  case 16:
    switch (format.sampleType()) {
    case QAudioFormat::UnSignedInt:
      _maxAmplitude = 65535;
      break;
    case QAudioFormat::SignedInt:
      _maxAmplitude = 32767;
      break;
    default:
      break;
    }
    break;

  case 32:
    switch (format.sampleType()) {
    case QAudioFormat::UnSignedInt:
      _maxAmplitude = 0xffffffff;
      break;
    case QAudioFormat::SignedInt:
      _maxAmplitude = 0x7fffffff;
      break;
    case QAudioFormat::Float:
      _maxAmplitude = 1;
      break;
    default:
      break;
    }
    break;

  default:
    break;
  }
  _maxAmplitude /= 4;
}

void Recorder::setScore(const QVector<int> &scoreNotes)
{
  _scoreNotes = scoreNotes;
  _dtwRow.resize(_scoreNotes.size());
  _nextRow.resize(_scoreNotes.size());
  resetDtw();
}

int Recorder::findNoteFromPitch(float pitch)
{
  auto &notesBoundry = _settings->notesFrequencyBoundry();
  int beg = 0, end = notesBoundry.size() - 1;
  while (beg < end) {
    int m = (beg + end) / 2;
    if (notesBoundry[m].second < pitch)
      beg = m + 1;
    else if (notesBoundry[m].first > pitch)
      end = m - 1;
    else
      beg = end = m;
  }
  return beg;
}

void Recorder::startFollowing()
{
  resetDtw();
  _isFollowing = true;
  emit positionChanged(0);
}

void Recorder::stopFollowing()
{
  _isFollowing = false;
}
