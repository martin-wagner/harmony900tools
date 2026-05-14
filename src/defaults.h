// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "lib/logLevel.h"
#include "ui/settings.h"

namespace defaults
{

inline static const QByteArray adsDock = {"<?xml version=\"1.0\" encoding=\"UTF-8\"?><QtAdvancedDockingSystem Version=\"1\" UserVersion=\"0\" Containers=\"1\"><Container Floating=\"0\"><Splitter Orientation=\"|\" Count=\"2\"><Area Tabs=\"1\" Current=\"Left Panel\"><Widget Name=\"Left Panel\" Closed=\"0\"/></Area><Area Tabs=\"1\" Current=\"Right Panel\"><Widget Name=\"Right Panel\" Closed=\"0\"/></Area><Sizes>715 524 </Sizes></Splitter></Container></QtAdvancedDockingSystem>"};

inline const SettingDef loglevel()
{
  QList<QPair<QString, QVariant>> options;
  SettingDef s;

  s.key = "logLevel";
  s.label = QObject::tr("Log level");
  s.helpText = QObject::tr("Sets the minimum log level");
  s.type = SettingType::MultiSelection;
  s.defaultValue = 5;
  s.tab = "";
  s.minValue = QVariant { };
  s.maxValue = QVariant { };
  for (int i = 0; i <= (int)LogLevel::Debug; i++) {
    options.push_back({logLevelName((LogLevel)i), i });
  }
  s.options = options;

  return s;
};

inline const SettingDef userlevel()
{
  QList<QPair<QString, QVariant>> options;
  SettingDef s;

  s.key = "userLevel";
  s.label = QObject::tr("User Level");
  s.helpText = QObject::tr("Controls which features are accessible.");
  s.type = SettingType::MultiSelection;
  s.defaultValue = 0;
  s.tab = "";
  s.minValue = QVariant { };
  s.maxValue = QVariant { };
  s.options = options = { { QObject::tr("Simple"), 0 }, { QObject::tr("Expert"),
      1 }, { QObject::tr("Developer"), 2 } };

  return s;
};


}

