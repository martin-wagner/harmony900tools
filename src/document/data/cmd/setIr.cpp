// SPDX-License-Identifier: LGPL-2.1-or-later

#include "setIr.h"

using namespace std;

namespace document
{
namespace data
{

SetIrCommand::SetIrCommand(ConfigData &c, uint32_t devicePos,
    item::ProtoCommand &cmd, int cmdPos, bool overwrite, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Set Proto Cmd (device: %1)").arg(devicePos),
        parent), isValid(false), overwrite(overwrite), c(c), devicePos(
        devicePos), cmdPos(cmdPos), proto(cmd)
{
  if (devicePos >= c.getDevices().size()) {
    return;
  }
  if (cmd.name.get().empty()) {
    return;
  }
  auto &cmds = c.getDevices()[devicePos].getIrCommands();
  if (!overwrite && cmds.nameExists(cmd.name.get())) {
    emit writeMsg(
        tr("Command %1 already exists").arg(
            QString::fromStdString(cmd.name.get())));
    return;
  }

  uint32_t cmdCount = cmds.getProtoCommands().size();

  if (overwrite) {
    if ((cmdPos < 0) || (static_cast<uint32_t>(cmdPos) >= cmdCount)) {
      return;
    }
    prevProto = cmds.getProtoCommands()[cmdPos];
  } else {
    if (cmdPos < 0) {
      cmdPos = static_cast<int>(cmdCount);
    }
    if (static_cast<uint32_t>(cmdPos) > cmdCount) {
      return;
    }
  }

  this->cmdPos = cmdPos;
  isValid = true;
}

SetIrCommand::SetIrCommand(ConfigData &c, uint32_t devicePos,
    item::RawCommand &cmd, int cmdPos, bool overwrite, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Set Raw Cmd (device: %1)").arg(devicePos), parent), overwrite(
        overwrite), c(c), devicePos(devicePos), raw(cmd)
{
  if (devicePos >= c.getDevices().size()) {
    return;
  }
  if (cmd.name.get().empty()) {
    return;
  }
  auto &cmds = c.getDevices()[devicePos].getIrCommands();
  if (!overwrite && cmds.nameExists(cmd.name.get())) {
    emit writeMsg(
        tr("Command %1 already exists").arg(
            QString::fromStdString(cmd.name.get())));
    return;
  }

  uint32_t cmdCount = cmds.getRawCommands().size();

  if (overwrite) {
    if ((cmdPos < 0) || (static_cast<uint32_t>(cmdPos) >= cmdCount)) {
      return;
    }
    prevRaw = cmds.getRawCommands()[cmdPos];
  } else {
    if (cmdPos < 0) {
      cmdPos = static_cast<int>(cmdCount);
    }
    if (static_cast<uint32_t>(cmdPos) > cmdCount) {
      return;
    }
  }

  this->cmdPos = cmdPos;
  isValid = true;
}

void SetIrCommand::redo()
{
  if (!isValid) {
    return;
  }

  if (!overwrite) {
    emit itemAboutToBeAdded(Item::DEVICE_IR_DATA, cmdPos);
  }

  if (proto.has_value()) {
    auto &cmds = c.getDevices()[devicePos].getIrCommands().getProtoCommands();
    if (overwrite) {
      cmds[cmdPos] = proto.value();
    } else {
      cmds.insert(cmds.begin() + cmdPos, proto.value());
    }
  } else if (raw.has_value()) {
    auto &cmds = c.getDevices()[devicePos].getIrCommands().getRawCommands();
    if (overwrite) {
      cmds[cmdPos] = raw.value();
    } else {
      cmds.insert(cmds.begin() + cmdPos, raw.value());
    }
  }

  if (overwrite) {
    emit itemChanged(Item::DEVICE_IR_DATA, cmdPos);
  } else {
    emit itemAdded(Item::DEVICE_IR_DATA, cmdPos);
  }
  emit dirtyChanged(true);
}

void SetIrCommand::undo()
{
  if (!isValid) {
    return;
  }

  if (!overwrite) {
    emit itemAboutToBeRemoved(Item::DEVICE_IR_DATA, cmdPos);
  }

  if (proto.has_value()) {
    auto &cmds = c.getDevices()[devicePos].getIrCommands().getProtoCommands();
    if (overwrite) {
      cmds[cmdPos] = prevProto.value();
    } else {
      cmds.erase(cmds.begin() + cmdPos);
    }
  } else if (raw.has_value()) {
    auto &cmds = c.getDevices()[devicePos].getIrCommands().getRawCommands();
    if (overwrite) {
      cmds[cmdPos] = prevRaw.value();
    } else {
      cmds.erase(cmds.begin() + cmdPos);
    }
  }

  if (overwrite) {
    emit itemChanged(Item::DEVICE_IR_DATA, cmdPos);
  } else {
    emit itemRemoved(Item::DEVICE_IR_DATA, cmdPos);
  }
  emit dirtyChanged(true);
}

bool SetIrCommand::valid() const
{
  return isValid;
}

}
}
