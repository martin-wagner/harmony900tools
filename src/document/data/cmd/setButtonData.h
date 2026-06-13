// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class SetActionCommand: public SetPropertyBaseCommand<std::string>
{
  public:
    SetActionCommand(ConfigData &c, const std::string &value,
        item::ButtonType t, uint32_t devicePos, int buttonPos,
        QUndoCommand *parent = nullptr);
};

class SetNameCommand: public SetPropertyBaseCommand<std::string>
{
  public:
    SetNameCommand(ConfigData &c, const std::string &value,
        item::ButtonType t, uint32_t devicePos, int buttonPos,
        QUndoCommand *parent = nullptr);
};

class SetFileCommand: public SetPropertyBaseCommand<std::string>
{
  public:
    SetFileCommand(ConfigData &c, const std::string &value,
        item::ButtonType t, uint32_t devicePos, int buttonPos,
        QUndoCommand *parent = nullptr);
};

class SetPositionCommand: public SetPropertyBaseCommand<int32_t>
{
  public:
    SetPositionCommand(ConfigData &c, const int32_t &value,
        item::ButtonType t, uint32_t devicePos, int buttonPos,
        QUndoCommand *parent = nullptr);
};

}
}
