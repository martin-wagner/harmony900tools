// SPDX-License-Identifier: LGPL-2.1-or-later

#include "removeChannel.h"

using namespace std;

namespace document
{
namespace data
{

RemoveActivityChannelCommand::RemoveActivityChannelCommand(ConfigData &c,
    uint32_t activityPos, uint32_t channelPos, QUndoCommand *parent) :
    BaseCommand(
        QObject::tr("Remove Channel (from activity: %1)").arg(activityPos),
        parent), c(c), activityPos(activityPos), channelPos(channelPos)
{
  uint32_t channelCount;

  if (activityPos >= c.getActivities().size()) {
    //beyond end
    return;
  }
  channelCount = c.getActivities()[activityPos].getChannels().size();
  if (channelPos >= channelCount) {
    return;
  }

  // copy Channel for undo
  channel = c.getActivities()[activityPos].getChannels()[channelPos];
  isValid = true;
}

void RemoveActivityChannelCommand::redo()
{
  if (!isValid) {
    return;
  }
  QUndoCommand::redo();

  auto &channels = c.getActivities()[activityPos].getChannels();
  channels.erase(channels.begin() + channelPos);
  emit dirtyChanged(true);
}

void RemoveActivityChannelCommand::undo()
{
  if (!isValid) {
    return;
  }

  auto &channels = c.getActivities()[activityPos].getChannels();
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
