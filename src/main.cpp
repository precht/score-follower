#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFile>
#include <QDebug>
#include <QQuickStyle>
#include <QApplication>

#include "controller.h"
#include "recorder.h"
#include "devicesmodel.h"


int main(int argc, char *argv[])
{
//  qputenv("QT_SCALE_FACTOR", "1.5");
//  QQuickStyle::setStyle("org.kde.desktop");
  QQuickStyle::setStyle("Default");

  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication app(argc, argv);
  QQmlApplicationEngine engine;

  Controller controller;
  engine.rootContext()->setContextProperty("controller", &controller);
  DevicesModel devicesModel;
  engine.rootContext()->setContextProperty("devicesModel", &devicesModel);

  engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
  if (engine.rootObjects().isEmpty())
    return -1;

  return app.exec();
}

/*
 * TODO
 * - display multiple pages and automatically change
 * - change input device between pc audio input and output (requires to change QAudioRecorder to QAudioInput ???)
 * - test pitch detection for low and for high notes.
 *    maybe instead of having one _minimalConfidence each notes should have its own cut-off confidence value
*/
