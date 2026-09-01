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
  s.tab = "Debug";
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

inline const SettingDef undoMacros()
{
  QList<QPair<QString, QVariant>> options;
  SettingDef s;

  s.key = "undoMacro";
  s.label = QObject::tr("Disable Undo Macro");
  s.helpText = QObject::tr("Disable undo macros, instead print all commands separately");
  s.type = SettingType::Bool;
  s.defaultValue = false;
  s.tab = "Debug";
  s.minValue = QVariant { };
  s.maxValue = QVariant { };

  return s;
};

inline const SettingDef loadLastUsed()
{
  QList<QPair<QString, QVariant>> options;
  SettingDef s;

  s.key = "reopen";
  s.label = QObject::tr("Open last document");
  s.helpText = QObject::tr("Opens the document that was last used (if available)");
  s.type = SettingType::Bool;
  s.defaultValue = true;
  s.tab = "";
  s.minValue = QVariant { };
  s.maxValue = QVariant { };

  return s;
};

static constexpr int DEFAULT_COLUMN_WIDTH = 100;
inline const SettingDef columWithFactor()
{
  QList<QPair<QString, QVariant>> options;
  SettingDef s;

  s.key = "columnWidthFactor";
  s.label = QObject::tr("Column Width");
  s.helpText = QObject::tr("Makes the table columns more or less wide. 1 = default, >1 => wider, <1 => narrower");
  s.type = SettingType::Double;
  s.defaultValue = 1.0;
  s.tab = "";
  s.minValue = QVariant { 0.5 };
  s.maxValue = QVariant { 5.0 };

  return s;
};

inline const SettingDef stackedView()
{
  QList<QPair<QString, QVariant>> options;
  SettingDef s;

  s.key = "stackedView";
  s.label = QObject::tr("View mode (needs restart)");
  s.helpText = QObject::tr("Device / Activity detail view.\n\nUse stacked view for large display or tabs for small display. Needs restart to apply.");
  s.type = SettingType::MultiSelection;
  s.defaultValue = 1;
  s.tab = "";
  s.minValue = QVariant { };
  s.maxValue = QVariant { };
  s.options = options = { { QObject::tr("Stacked"), 0 }, { QObject::tr("Tabs"),
      1 } };

  return s;
};

inline const SettingDef learnStreamTimeout()
{
  QList<QPair<QString, QVariant>> options;
  SettingDef s;

  s.key = "streamTimeout";
  s.label = QObject::tr("Stream / macro timeout (ms)");
  s.helpText = QObject::tr("Max. recording time for stream / macros. H900 software maxed out a 1000ms, try more at your own risk!");
  s.type = SettingType::Int;
  s.defaultValue = 1000;
  s.tab = "Learning";
  s.minValue = QVariant { 250 };
  s.maxValue = QVariant { 10000 };

  return s;
};


// @formatter:off
//  settings->addSetting({
//      .key          = "username",
//      .label        = "Username",
//      .helpText     = "Your display name in the application.",
//      .type         = SettingType::String,
//      .defaultValue = "user",
//  });
//
//  settings->addSetting({
//      .key          = "darkMode",
//      .label        = "Dark mode",
//      .helpText     = "Enable dark colour scheme.",
//      .type         = SettingType::Bool,
//      .defaultValue = false,
//  });
//
//  // ── Network tab ───────────────────────────────────────────────────────────
//
//  settings->addSetting({
//      .key          = "port",
//      .label        = "Port",
//      .helpText     = "TCP port to listen on.",
//      .type         = SettingType::Int,
//      .defaultValue = 8080,
//      .tab          = "Network",
//      .minValue     = 1024,
//      .maxValue     = 65535,
//  });
//
//  settings->addSetting({
//      .key          = "timeout",
//      .label        = "Timeout (s)",
//      .helpText     = "Connection timeout in seconds.",
//      .type         = SettingType::Double,
//      .defaultValue = 30.0,
//      .tab          = "Network",
//      .minValue     = 0.1,
//      .maxValue     = 300.0,
//  });
//
//  settings->addSetting({
//      .key      = "protocol",
//      .label    = "Protocol",
//      .helpText = "Transport protocol.",
//      .type     = SettingType::MultiSelection,
//      .defaultValue = 0,   // matches itemData below
//      .tab      = "Network",
//      .options  = {
//          { "TCP",  0 },
//          { "UDP",  1 },
//          { "QUIC", 2 },
//      },
//  }); todo
// @formatter:on


}

