// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QApplication>
#include <QSplashScreen>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "mainwindow.h"

const int defaultDebugLevel = 5;

#include <chrono>
#include <thread>

int main(int argc, char *argv[])
{
  int debugLevel = defaultDebugLevel;

  QApplication app(argc, argv);
  QPixmap pixmap(":/res/splash.png");
  QSplashScreen splash(pixmap);
  splash.show();
  app.processEvents();
  QCoreApplication::setOrganizationName("");
  QCoreApplication::setOrganizationDomain(
      "https://github.com/martin-wagner/harmony900tools");
  QCoreApplication::setApplicationName (PROGRAM_NAME);
  QCoreApplication::setApplicationVersion (PROGRAM_VERSION);
  QCommandLineParser parser;
  parser.setApplicationDescription(QCoreApplication::applicationName());
  parser.addHelpOption();
  parser.addVersionOption();
  QCommandLineOption debugOption(QStringList() << "d" << "debug",
      "Activate debug output (0=emergency .. 7=debug, default "
          + QString::number(defaultDebugLevel) + ")", "level",
      QString::number(defaultDebugLevel));
  parser.addOption(debugOption);
  parser.process(app);

  if (parser.isSet(debugOption)) {
    bool ok = false;
    int val = parser.value(debugOption).toInt(&ok);
    if (!ok || val < 0 || val > 7) {
      qCritical() << "Invalid debug level. Must be 0..7.";
      parser.showHelp(1);
    }
    debugLevel = val;
  }

  MainWindow mainWin(debugLevel);
  if (!parser.positionalArguments().isEmpty()) {
    mainWin.loadFile(parser.positionalArguments().first());
  }
  mainWin.show();
  splash.finish(&mainWin);
  return app.exec();
}
