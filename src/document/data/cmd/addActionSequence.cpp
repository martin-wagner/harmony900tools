// SPDX-License-Identifier: LGPL-2.1-or-later

#include "setActionData.h"
#include "addActionSequence.h"

using namespace std;

namespace document
{
namespace data
{

AddActionSequenceCommand::AddActionSequenceCommand(ConfigData &c,
    uint32_t devicePos, uint32_t smPos, uint32_t actPos,
    item::StateTransitionAction t, int seqPos, QUndoCommand *parent) :
    BaseCommand(
        QObject::tr("Add Sequence to Action/SM (to device: %1)").arg(devicePos),
        parent)
{
  uint32_t seqCount;

  auto *act = getActionFromSmRef(c, devicePos, smPos, t, actPos);
  if (act == nullptr) {
    return;
  }
  seqCount = act->sequence.size();
  if (seqPos < 0) {
    seqPos = seqCount;
  }
  if (seqPos > static_cast<int>(seqCount)) {
    return;
  }

  getSeq = [&c, devicePos, smPos, t, actPos]() -> vector<item::SequenceItem>& {
    return getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence;
  };
  this->seqPos = seqPos;
  isValid = true;
}

AddActionSequenceCommand::AddActionSequenceCommand(ConfigData &c,
    uint32_t devicePos, item::DigitSection s, uint32_t digit, int seqPos,
    QUndoCommand *parent) :
    BaseCommand(
        QObject::tr("Add Sequence to Action/NUM (to device: %1)").arg(
            devicePos), parent)
{
  uint32_t seqCount;

  auto *act = getActionFromNumpadRef(c, devicePos, s, digit);
  if (act == nullptr) {
    return;
  }
  seqCount = act->sequence.size();
  if (seqPos < 0) {
    seqPos = seqCount;
  }
  if (seqPos > static_cast<int>(seqCount)) {
    return;
  }

  getSeq = [&c, devicePos, s, digit]() -> vector<item::SequenceItem>& {
    return getActionFromNumpadRef(c, devicePos, s, digit)->sequence;
  };
  this->seqPos = seqPos;
  isValid = true;
}

AddActionSequenceCommand::AddActionSequenceCommand(ConfigData &c,
    uint32_t activityPos, item::ActivityAction t, uint32_t actionPos,
    int seqPos, QUndoCommand *parent) :
    BaseCommand(
        QObject::tr("Add Sequence to Action (to activity: %1)").arg(
            activityPos), parent)
{
  uint32_t seqCount;

  auto *action = getActionFromActivity(c, activityPos, t, actionPos);
  if (action == nullptr) {
    return;
  }
  seqCount = action->sequence.size();
  if (seqPos < 0) {
    seqPos = seqCount;
  }
  if (seqPos > static_cast<int>(seqCount)) {
    return;
  }

  getSeq = [&c, activityPos, t, actionPos]() -> vector<item::SequenceItem>& {
    return getActionFromActivity(c, activityPos, t, actionPos)->sequence;
  };
  this->seqPos = seqPos;
  isValid = true;
}

void AddActionSequenceCommand::redo()
{
  if (!isValid) {
    return;
  }

  auto &seq = getSeq();
  seq.insert(seq.begin() + seqPos, item::SequenceItem());
  emit dirtyChanged(true);
}

void AddActionSequenceCommand::undo()
{
  if (!isValid) {
    return;
  }

  auto &seq = getSeq();
  seq.erase(seq.begin() + seqPos);
  emit dirtyChanged(true);
}

bool AddActionSequenceCommand::valid() const
{
  return isValid;
}

}
}
