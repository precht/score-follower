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

  _windowCalculator->input("frame").set(_audioFrame);
  _windowCalculator->output("frame").set(_windowedframe);

  _spectrumCalculator->input("frame").set(_windowedframe);
  _spectrumCalculator->output("spectrum").set(_spectrum);

  _pitchDetector->input("spectrum").set(_spectrum);
  _pitchDetector->output("pitch").set(_currentPitch);
  _pitchDetector->output("pitchConfidence").set(_currentConfidence);
}

void Recorder::processBuffer(const QAudioBuffer buffer)
{
  if (_settings->verbose()) {
    auto time = QDateTime::currentDateTime();
    if (_currentSecond != time.toSecsSinceEpoch()) {
      if (_currentSecond != 0) {
        qInfo().nospace().noquote() << time.toString("hh:mm:ss") << ": " << _samplesInCurrentSecond << " samples.";
        _samplesInCurrentSecond = 0;
      } else {
        qInfo() << "Buffer size: " <<  buffer.sampleCount();
      }
      _currentSecond = time.toSecsSinceEpoch();
    }
    _samplesInCurrentSecond += buffer.sampleCount();
  }

  if (buffer.format() != _currentFormat) {
    _currentFormat = buffer.format();
    setMaxAmplitude(buffer.format());
  }

  updateLevel(buffer);
  if (!_isFollowing)
    return;

  convertBufferToAudio(buffer);

  const size_t requiredSize = static_cast<size_t>(_settings->frameSize());
  while (_audioFrame.size() + _memory.size() >= requiredSize) {
    while (_audioFrame.size() < requiredSize) {
      _audioFrame.push_back(_memory.front());
      _memory.pop_front();
    }
    processFrame();
    _audioFrame.erase(_audioFrame.begin(), _audioFrame.begin() + _settings->hopSize());
  }
  while (!_memory.empty()) {
    _audioFrame.push_back(_memory.front());
    _memory.pop_front();
  }
}

void Recorder::processFrame()
{
  _windowCalculator->compute();
  _spectrumCalculator->compute();
  _pitchDetector->compute();

  auto &notesBoundry = _settings->notesFrequencyBoundry();
  if (_currentPitch < notesBoundry[_currentNoteNumber].first || _currentPitch > notesBoundry[_currentNoteNumber].second) {
    int noteNumber = findNoteFromPitch(_currentPitch);

    if (_currentConfidence >= _settings->minimalConfidence()[noteNumber]) {
      _currentNoteNumber = noteNumber;
      calculatePosition();

      if (_settings->verbose() && _lastWasSkipped) {
        qInfo().nospace() << "Note " << _lastSkippedNote << " was skipped " << _skippedCount  << " times (<"
                          << _settings->minimalConfidence()[noteNumber] << ").";
        _lastWasSkipped = false;
      }

      qInfo().nospace() << "Detected note " << noteNumber << ".";
    }
    else if (_settings->verbose()){
      if (_lastWasSkipped && _lastSkippedNote != noteNumber) {
        qInfo().nospace() << "Note " << _lastSkippedNote << " was skipped " << _skippedCount  << " times (<"
                          << _settings->minimalConfidence()[noteNumber] << ").";
        _skippedCount = 0;
      }
      _lastWasSkipped = true;
      _lastSkippedNote = noteNumber;
      _skippedCount++;
    }
  }
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

  const size_t requiredSize = static_cast<size_t>(_settings->frameSize());
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
    ptr += channelBytes;

    if (_audioFrame.size() < requiredSize)
      _audioFrame.push_back(value);
    else
      _memory.push_back(value);
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
  _nextRow[0] = qAbs(_currentNoteNumber - _scoreNotes[0]) + _dtwRow[0];

  int position = 0;
  int64_t minValue = _nextRow[0];
  for (int i = 1; i < _dtwRow.size(); i++) {
    _nextRow[i] = qAbs(_currentNoteNumber - _scoreNotes[i]) + qMin(_nextRow[i - 1], qMin(_dtwRow[i], _dtwRow[i-1]));
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
  _maxAmplitude /= 4; // because this way level bar changes are more visible
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
  int beginning = 0, end = notesBoundry.size() - 1;
  while (beginning < end) {
    int middle = (beginning + end) / 2;
    if (notesBoundry[middle].second < pitch)
      beginning = middle + 1;
    else if (notesBoundry[middle].first > pitch)
      end = middle - 1;
    else
      beginning = end = middle;
  }
  return beginning;
}

void Recorder::startFollowing()
{
  resetDtw();
  _isFollowing = true;
  emit positionChanged(0);
  qInfo() << "Started score following.";
}

void Recorder::stopFollowing()
{
  _isFollowing = false;
  qInfo() << "Stopped score following.";
}
