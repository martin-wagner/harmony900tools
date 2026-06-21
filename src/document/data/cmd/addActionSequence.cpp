// SPDX-License-Identifier: LGPL-2.1-or-later

#include "setActionData.h"
#include "addActionSequence.h"

using namespace std;

namespace document
{
namespace data
{

AddDeviceSmActionSequenceCommand::AddDeviceSmActionSequenceCommand(ConfigData &c,
    uint32_t devicePos, uint32_t smPos, uint32_t actPos,
    item::StateTransitionAction t, int seqPos, QUndoCommand *parent) :
    BaseCommand(
        QObject::tr("Add Sequence to Action (to device: %1)").arg(devicePos),
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

  getSeq =
      [&c, devicePos, smPos, t, actPos]() -> vector<item::SequenceItem>& {
        return getActionFromSmRef(c, devicePos, smPos, t, actPos)->sequence;
      };
  this->seqPos = seqPos;
  isValid = true;
}

void AddDeviceSmActionSequenceCommand::redo()
{
  if (!isValid) {
    return;
  }

  auto &seq = getSeq();
  seq.insert(seq.begin() + seqPos, item::SequenceItem());
  emit dirtyChanged(true);
}

void AddDeviceSmActionSequenceCommand::undo()
{
  if (!isValid) {
    return;
  }

  auto &seq = getSeq();
  seq.erase(seq.begin() + seqPos);
  emit dirtyChanged(true);
}

bool AddDeviceSmActionSequenceCommand::valid() const
{
  return isValid;
}

}
}
