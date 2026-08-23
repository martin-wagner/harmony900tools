// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class RemoveIrCommand: public BaseCommand
{
  Q_OBJECT
  public:
  RemoveIrCommand(ConfigData &c, uint32_t devicePos, uint32_t cmdPos, QUndoCommand *parent = nullptr);

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;
    uint32_t cmdPos;

  protected:
    bool checkRemove(const std::string &action);

  private:
    bool doRemove(const std::string &action);
    bool checkDeviceAction(item::DeviceAction &d, const std::string &action);
};

class RemoveIrProtoCommand: public RemoveIrCommand
{
  Q_OBJECT
  public:
    RemoveIrProtoCommand(ConfigData &c, uint32_t devicePos, uint32_t cmdPos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

  protected:
    item::ProtoCommand proto;
};

class RemoveIrRawCommand: public RemoveIrCommand
{
  Q_OBJECT
  public:
    RemoveIrRawCommand(ConfigData &c, uint32_t devicePos, uint32_t cmdPos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

  protected:
    item::RawCommand raw;
};


}
}
