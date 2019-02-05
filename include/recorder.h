// Author:  Jakub Precht

#ifndef RECORDER_H
#define RECORDER_H

#include <essentia/algorithmfactory.h>

#include <QTimer>
#include <QAudioRecorder>
#include <QAudioProbe>
#include <QAudioInput>

class Settings;

class Recorder : public QObject
{
  Q_OBJECT

public:
  Recorder(QObject *parent = nullptr);
  bool initialize();
  void setScore(const QVector<int> &scoreNotes);
  void resetDtw();
  void setSettings(const Settings *settings);
  void setAudioInput(QString audioInput);

public slots:
  void startFollowing();
  void stopFollowing();
  void processBuffer(const QAudioBuffer buffer);

signals:
  void positionChanged(int position);
  void levelChanged(float level);

private:
  void initializePitchDetector();
  int findNoteFromPitch(float pitch);
  void calculatePosition();
  void calculateMaxAmplitude();
  void updateLevel(const QAudioBuffer &buffer);
  void convertBufferToAudio(const QAudioBuffer &buffer);
  void processFrame();
  void setMaxAmplitude(const QAudioFormat &format);

  // ----------

  // recording

  const Settings *_settings;
  bool _isFollowing = false;
  QTimer *_timer = nullptr;
  QAudioProbe *_probe = nullptr;
  QAudioRecorder *_recorder = nullptr;
  QAudioInput *_audioInput = nullptr;
  QAudioFormat _currentFormat;
  QAudioEncoderSettings _recorderSettings;

  float _maxAmplitude = 1;
  float _level = 0;
  int _levelCount = 0;
  const int _maxLevelCount = 16;

  // position

  int _position = 0;
  qint64 _currentSecond = 0;
  int _samplesInCurrentSecond = 0;
  QVector<int> _scoreNotes;
  QVector<int64_t> _dtwRow;
  QVector<int64_t> _nextRow;

  // essentia

  const int64_t _infinity = std::numeric_limits<int64_t>::max();
  int _bufferSize = 0;
  int _currentNoteNumber = 0;
  float _currentPitch = 0;
  float _currentConfidence = 0;
  bool _lastWasSkipped = false;
  int _lastSkippedNote = 0;
  int _skippedCount = 0;

  std::deque<float> _memory;
  std::vector<float> _audioFrame;
  std::vector<float> _spectrum;
  std::vector<float> _windowedframe;

  essentia::standard::Algorithm* _windowCalculator;
  essentia::standard::Algorithm* _spectrumCalculator;
  essentia::standard::Algorithm* _pitchDetector;
};

#endif // RECORDER_H
