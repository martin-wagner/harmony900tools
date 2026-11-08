// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class SetIrCommand: public BaseCommand
{
    Q_OBJECT
  public:
    // cmdPos -1 = append (insert mode only).
    // overwrite: replace existing item at cmdPos; insert: shift items at cmdPos.
    SetIrCommand(ConfigData &c, uint32_t devicePos, item::ProtoCommand &cmd, int cmdPos = -1, bool overwrite = false, QUndoCommand *parent = nullptr);
    SetIrCommand(ConfigData &c, uint32_t devicePos, item::RawCommand &cmd, int cmdPos = -1, bool overwrite = false, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;
    bool overwrite = false;

    ConfigData &c;
    uint32_t devicePos;
    int cmdPos = -1;
    std::optional<item::ProtoCommand> proto;
    std::optional<item::ProtoCommand> prevProto;
    std::optional<item::RawCommand> raw;
    std::optional<item::RawCommand> prevRaw;

  protected:
    void updateActions(const std::string &oldAction, const std::string &newAction);

  private:
    void updateDeviceAction(item::DeviceAction &d, const std::string &oldAction, const std::string &newAction, Item event, int eventPos = 0);


};

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
