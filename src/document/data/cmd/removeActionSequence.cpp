// SPDX-License-Identifier: LGPL-2.1-or-later

#include "setActionData.h"
#include "removeActionSequence.h"

using namespace std;

namespace document
{
namespace data
{

RemoveDeviceSmActionSequenceCommand::RemoveDeviceSmActionSequenceCommand(ConfigData &c,
    uint32_t devicePos, uint32_t smPos, uint32_t actPos,
    item::StateTransitionAction t, uint32_t seqPos, QUndoCommand *parent) :
    BaseCommand(
        QObject::tr("Remove Sequence from Action (Pos: %1)").arg(devicePos),
        parent)
{
  auto *act = getActionFromSmRef(c, devicePos, smPos, t, actPos);
  if (act == nullptr) {
    return;
  }
  if (seqPos >= act->sequence.size()) {
    return;
  }

  getSeq =
      [&c, devicePos, smPos, t, actPos]() -> vector<item::SequenceItem>& {
        return getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence;
      };
  this->seqPos = seqPos;
  sequenceItem = getSeq()[seqPos];
  isValid = true;
}

void RemoveDeviceSmActionSequenceCommand::redo()
{
  if (!isValid) {
    return;
  }

  auto &seq = getSeq();
  seq.erase(seq.begin() + seqPos);
  emit dirtyChanged(true);
}

void RemoveDeviceSmActionSequenceCommand::undo()
{
  if (!isValid) {
    return;
  }

  auto &seq = getSeq();
  seq.insert(seq.begin() + seqPos, sequenceItem);
  emit dirtyChanged(true);
}

bool RemoveDeviceSmActionSequenceCommand::valid() const
{
  return isValid;
}

}
}
