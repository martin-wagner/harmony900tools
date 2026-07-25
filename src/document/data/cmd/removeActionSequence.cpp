// SPDX-License-Identifier: LGPL-2.1-or-later

#include "setActionData.h"
#include "removeActionSequence.h"

using namespace std;

namespace document
{
namespace data
{

RemoveDeviceActionSequenceCommand::RemoveDeviceActionSequenceCommand(
    ConfigData &c, uint32_t devicePos, uint32_t smPos, uint32_t actPos,
    item::StateMachineAction t, uint32_t seqPos, QUndoCommand *parent) :
    BaseCommand(
        QObject::tr("Remove Sequence from Action/SM (device: %1)").arg(
            devicePos), parent), item(Item::DEVICE_STATEMACHINE), itemPos(smPos)
{
  auto *act = getActionFromSmRef(c, devicePos, smPos, t, actPos);
  if (act == nullptr) {
    return;
  }
  if (seqPos >= act->sequence.size()) {
    return;
  }

  getSeq = [&c, devicePos, smPos, t, actPos]() -> vector<item::SequenceItem>& {
    return getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence;
  };
  this->seqPos = seqPos;
  sequenceItem = getSeq()[seqPos];
  isValid = true;
}

RemoveDeviceActionSequenceCommand::RemoveDeviceActionSequenceCommand(
    ConfigData &c, uint32_t devicePos, item::DigitSection s, uint32_t digit,
    uint32_t seqPos, QUndoCommand *parent) :
    BaseCommand(
        QObject::tr("Remove Sequence from Action/NUM (device: %1)").arg(
            devicePos), parent), item(Item::DEVICE_NUMPAD), itemPos(0)
{
  auto *act = getActionFromNumpadRef(c, devicePos, s, digit);
  if (act == nullptr) {
    return;
  }
  if (seqPos >= act->sequence.size()) {
    return;
  }

  getSeq = [&c, devicePos, s, digit]() -> vector<item::SequenceItem>& {
    return getActionFromNumpadRef(c, devicePos, s, digit)->sequence;
  };
  this->seqPos = seqPos;
  sequenceItem = getSeq()[seqPos];
  isValid = true;
}

document::data::RemoveDeviceActionSequenceCommand::RemoveDeviceActionSequenceCommand(
    ConfigData &c, uint32_t activityPos, item::ActivityAction t,
    uint32_t actionPos, uint32_t seqPos, QUndoCommand *parent) :
    BaseCommand(
        QObject::tr("Remove Sequence from Action (activity:  %1)").arg(
            activityPos), parent), item(Item::ACTIVITY_ACTION), itemPos(
        activityPos)
{
  auto *action = getActionFromActivity(c, activityPos, t, actionPos);
  if (action == nullptr) {
    return;
  }
  if (seqPos >= action->sequence.size()) {
    return;
  }

  getSeq = [&c, activityPos, t, actionPos]() -> vector<item::SequenceItem>& {
    return getActionFromActivity(c, activityPos, t, actionPos)->sequence;
  };
  this->seqPos = seqPos;
  sequenceItem = getSeq()[seqPos];
  isValid = true;
}

void RemoveDeviceActionSequenceCommand::redo()
{
  if (!isValid) {
    return;
  }

  auto &seq = getSeq();
  seq.erase(seq.begin() + seqPos);
  emit itemChanged(item, itemPos);
  emit dirtyChanged(true);
}

void RemoveDeviceActionSequenceCommand::undo()
{
  if (!isValid) {
    return;
  }

  auto &seq = getSeq();
  seq.insert(seq.begin() + seqPos, sequenceItem);
  emit itemChanged(item, itemPos);
  emit dirtyChanged(true);
}

bool RemoveDeviceActionSequenceCommand::valid() const
{
  return isValid;
}

}
}
