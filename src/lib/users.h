// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QObject>
#include "ui/settings.h"
#include "defaults.h"

namespace lib {

class UserLevel: public QObject
{
  Q_OBJECT

  public:
    enum class Level
    {
      Simple = 0, Expert = 1, Developer = 2
    };

    explicit UserLevel(Settings &settings, QObject *parent = nullptr) :
        QObject(parent), settings(settings)
    {
      settings.addSetting(defaults::userlevel());
    }

    Level getLevel() const
    {
      int val = settings.value("userLevel").toInt();
      switch (val) {
        case static_cast<int>(Level::Expert):
          return Level::Expert;
        case static_cast<int>(Level::Developer):
          return Level::Developer;
        default:
          return Level::Simple;
      }
    }

    void setLevel(Level level)
    {
      if (getLevel() == level) {
        return;
      }
      QVariant val = static_cast<int>(level);
      settings.setValue("userLevel", val);
      emit levelChanged(level);
    }

    bool validate(Level required) const
    {
      Level current = getLevel();
      switch (required) {
        case Level::Simple:
          return current == Level::Simple || current == Level::Expert
              || current == Level::Developer;
        case Level::Expert:
          return current == Level::Expert || current == Level::Developer;
        case Level::Developer:
          return current == Level::Developer;
      }
      return false;
    }

    QString levelToString() const
    {
      switch (getLevel()) {
        case Level::Simple:
          return tr("Simple");
        case Level::Expert:
          return tr("Expert");
        case Level::Developer:
          return tr("Developer");
      }
      return tr("Unknown");
    }

  signals:
    void levelChanged(Level level);

  private:
    Settings &settings;
};

}
