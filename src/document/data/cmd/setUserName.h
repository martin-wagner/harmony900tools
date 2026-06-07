// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class SetUserNameCommand: public BaseCommand
{
  Q_OBJECT
  public:
    SetUserNameCommand(ConfigData &c, const QString &firstName, const QString &lastName, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

  protected:
    ConfigData &c;

    std::string firstName;
    std::string lastName;
    std::string prevFirstName;
    std::string prevLastName;
};

}
}
