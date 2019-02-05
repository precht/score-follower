// Author:  Jakub Precht

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QQuickStyle>
#include <QApplication>

#include "controller.h"
#include "recorder.h"

#include <cstring>

int main(int argc, char *argv[])
{
  bool isVerbose = false;
  for (int i = 1; i < argc; i++) {
    if (std::strcmp(argv[i], "-v") && std::strcmp(argv[i], "--verbose"))
      qWarning().nospace() << "Unrecognized argument: " << QString(argv[i]) <<".";
    else
      isVerbose = true;
  }

  QQuickStyle::setStyle("org.kde.desktop");
  QQuickStyle::setFallbackStyle("Default");

  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication app(argc, argv);
  QQmlApplicationEngine engine;

  Controller controller(isVerbose);
  if (!controller.createdSuccessfully()) {
    qCritical() << "Aborting...";
    return -1;
  }
  engine.rootContext()->setContextProperty("controller", &controller);

  engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
  if (engine.rootObjects().isEmpty())
    return -1;

  return app.exec();
}
