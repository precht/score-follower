// Author:  Jakub Precht

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
//  QQuickStyle::setStyle("org.kde.desktop");
  QQuickStyle::setStyle("Default");

  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication app(argc, argv);
  QQmlApplicationEngine engine;

  Controller controller;
  if (!controller.createdSuccessfully()) {
    qCritical() << "Aborting...";
    return -1;
  }
  engine.rootContext()->setContextProperty("controller", &controller);
  DevicesModel devicesModel;
  engine.rootContext()->setContextProperty("devicesModel", &devicesModel);

  engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
  if (engine.rootObjects().isEmpty())
    return -1;

  return app.exec();
}
