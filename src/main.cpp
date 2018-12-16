#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFile>
#include <QDebug>

#include "controller.h"
#include "recorder.h"

//#define NOGUI

int main(int argc, char *argv[])
{

#ifdef NOGUI

  QCoreApplication a(argc, argv);

//  QVector<int> scoreNotes = { 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72,
//                              60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72 };

  QVector<int> scoreNotes = { 60, 62, 64, 65, 67, 69, 71, 72,
                              60, 62, 64, 65, 67, 69, 71, 72,
                              60, 62, 64, 65, 67, 69, 71, 72 };

  Recorder r;
  Positioner p;
  p.setScore(scoreNotes);
  QObject::connect(&r, &Recorder::nextNote, &p, &Positioner::processNextRecordedNote);
  QObject::connect(&r, &Recorder::finished, &a, QCoreApplication::quit);

  QTimer timer;
  timer.singleShot(0, &r, &Recorder::run);

  return a.exec();

#else

  //  qputenv("QT_SCALE_FACTOR", "1.5");
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  QGuiApplication app(argc, argv);
  QQmlApplicationEngine engine;

  Controller controller;
  engine.rootContext()->setContextProperty("controller", &controller);

  engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
  if (engine.rootObjects().isEmpty())
    return -1;

  return app.exec();

#endif
}
