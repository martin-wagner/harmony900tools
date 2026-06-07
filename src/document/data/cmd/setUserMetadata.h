// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class SetUserMetadataCommand: public BaseCommand
{
  Q_OBJECT
  public:
    SetUserMetadataCommand(ConfigData &c, const QString &user, const QString &creationTimestamp, const QString &modificationTimestamp, QUndoCommand *parent = nullptr);
    //fill others
    SetUserMetadataCommand(ConfigData &c, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

  protected:
    ConfigData &c;

    std::string user;
    std::string creation;
    std::string modification;
    std::string prevUser;
    std::string prevCreation;
    std::string prevModification;
};

class SetUserNewDeviceFoundCommand: public SetPropertyBaseCommand<bool>
{
  public:
    SetUserNewDeviceFoundCommand(ConfigData &c, bool value, QUndoCommand *parent = nullptr);
};


class SetUserTrainingWheelsCommand: public SetPropertyBaseCommand<bool>
{
  public:
    SetUserTrainingWheelsCommand(ConfigData &c, bool value, QUndoCommand *parent = nullptr);
};


class SetUserLocaleCommand: public SetPropertyBaseCommand<Enum<Locale>>
{
  public:
    SetUserLocaleCommand(ConfigData &c, const Enum<Locale> &value, QUndoCommand *parent = nullptr);
};

class SetUserTimeFormatCommand: public SetPropertyBaseCommand<Enum<TimeFormat>>
{
  public:
    SetUserTimeFormatCommand(ConfigData &c, const Enum<TimeFormat> &value, QUndoCommand *parent = nullptr);
};



}
}
