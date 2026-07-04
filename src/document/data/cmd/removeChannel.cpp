// SPDX-License-Identifier: LGPL-2.1-or-later

#include "removeChannel.h"

using namespace std;

namespace document
{
namespace data
{

RemoveActivityChannelCommand::RemoveActivityChannelCommand(ConfigData &c,
    uint32_t devicePos, uint32_t channelPos, QUndoCommand *parent) :
    BaseCommand(
        QObject::tr("Remove Channel (from activity: %1)").arg(devicePos),
        parent), c(c), devicePos(devicePos), channelPos(channelPos)
{
  uint32_t channelCount;

  if (devicePos >= c.getActivities().size()) {
    //beyond end
    return;
  }
  channelCount = c.getActivities()[devicePos].getChannels().size();
  if (channelPos >= channelCount) {
    return;
  }

  // copy Channel for undo
  channel = c.getActivities()[devicePos].getChannels()[channelPos];
  isValid = true;
}

void RemoveActivityChannelCommand::redo()
{
  if (!isValid) {
    return;
  }
  QUndoCommand::redo();

  auto &channels = c.getActivities()[devicePos].getChannels();
  channels.erase(channels.begin() + channelPos);
  emit dirtyChanged(true);
}

void RemoveActivityChannelCommand::undo()
{
  if (!isValid) {
    return;
  }

  auto &channels = c.getActivities()[devicePos].getChannels();
  channels.insert(channels.begin() + channelPos, channel);
  QUndoCommand::undo();
  emit dirtyChanged(true);
}

bool RemoveActivityChannelCommand::valid() const
{
  return isValid;
}

}
}
