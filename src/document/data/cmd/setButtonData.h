// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class SetButtonActionCommand: public SetPropertyBaseCommand<std::string>
{
  public:
    SetButtonActionCommand(ConfigData &c, const std::string &value,
        item::ButtonType t, uint32_t devicePos, int buttonPos,
        QUndoCommand *parent = nullptr);
};

class SetButtonNameCommand: public SetPropertyBaseCommand<std::string>
{
  public:
    SetButtonNameCommand(ConfigData &c, const std::string &value,
        item::ButtonType t, uint32_t devicePos, int buttonPos,
        QUndoCommand *parent = nullptr);
};

class SetButtonFileCommand: public SetPropertyBaseCommand<std::string>
{
  public:
    SetButtonFileCommand(ConfigData &c, const std::string &value,
        item::ButtonType t, uint32_t devicePos, int buttonPos,
        QUndoCommand *parent = nullptr);
};

class SetButtonPositionCommand: public SetPropertyBaseCommand<int32_t>
{
  public:
    SetButtonPositionCommand(ConfigData &c, const int32_t &value,
        item::ButtonType t, uint32_t devicePos, int buttonPos,
        QUndoCommand *parent = nullptr);
};

}
}
