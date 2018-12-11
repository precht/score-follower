#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFile>
#include <QDebug>
#include "controller.h"

int main(int argc, char *argv[])
{
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
}
