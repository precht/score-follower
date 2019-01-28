#ifndef RECORDER_H
#define RECORDER_H

#include <QObject>
#include <QTimer>
#include <QAudioRecorder>
#include <QAudioProbe>
#include <QScopedPointer>
#include <QAudioDeviceInfo>
#include <QFile>
#include <QAudioInput>

#include <map>
#include <essentia/algorithmfactory.h>
#include <essentia/essentiamath.h> // for the isSilent function
#include <essentia/pool.h>

class Recorder : public QObject
{
  Q_OBJECT

public:
  Recorder(QObject *parent = nullptr);
  void setScore(const QVector<int> &scoreNotes);
  void resetDtw();

public slots:
//  bool setDevice(const QString &deviceName);
  void startRecording();
  void stopRecording();
  void processBuffer(const QAudioBuffer buffer);

signals:
  void positionChanged(int position);
  void levelChanged(float level);

private:
  void initializeDevices();
  void initializeNotes();
  void initializeMinimalConfidence();
  void initializePitchDetector(int bufferSize, int bufferSampleRate);
  int findNoteFromPitch(float pitch);
  void calculatePosition();
  void calculateMaxAmplitude();
  void updateLevel(const QAudioBuffer &buffer);
  void convertBufferToAudio(const QAudioBuffer &buffer);

  void setMaxAmplitude(const QAudioFormat &format);

  // -----

  // recording
//  const uint _recordingDuration = 60000; // in ms

  bool _isRunning = false;
  QTimer *_timer = nullptr;
  QAudioProbe *_probe = nullptr;
  QAudioRecorder *_recorder = nullptr;
  QAudioInput *_audioInput = nullptr;
  QAudioFormat _defaultFormat;
  QAudioFormat _currentFormat;
  QAudioEncoderSettings _recorderSettings;

  QString _notesFrequencyFileName = ":/other/notes-frequency";
  QString _notesConfidenceFileName = ":/other/notes-confidence";
  QVector<float> _notesFrequency;
  QVector<QPair<float, float>> _notesBoundry;
  QMap<QString, QAudioDeviceInfo> _devices;

  float _maxAmplitude = 1;
  float _level = 0;
  int _levelCount = 0;
  const int _maxLevelCount = 16;

  // position
  int _position = 0;
  QVector<int> _scoreNotes;
  QVector<int64_t> _dtwRow;
  QVector<int64_t> _nextRow;

  // essentia
  const int _sampleRate = 480 * 100; // 100 buffers per second, 480 samples per buffer
  const uint _essentiaFrameSize = 480 * 20;
  const int _essentiaHopSize = 480 * 5;

  const float _minimalConfidenceCoefficient = 0.95f;
  QVector<float> _minimalConfidence;

  const int64_t _infinity = std::numeric_limits<int64_t>::max();
  int _bufferSize = 0;
  int _bufferSampleRate = 0;
  int _noteNumber = 0;
  float _currentPitch = 0;
  float _currentConfidence = 0;

  std::vector<float> _audio;
  std::vector<float> _spectrum;
  std::vector<float> _windowedframe;

  essentia::standard::Algorithm* _windowCalculator;
  essentia::standard::Algorithm* _spectrumCalculator;
  essentia::standard::Algorithm* _pitchDetector;
};

#endif // RECORDER_H
