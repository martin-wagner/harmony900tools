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

}
}
