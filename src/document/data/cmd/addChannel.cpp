// SPDX-License-Identifier: LGPL-2.1-or-later

#include "addChannel.h"

using namespace std;

namespace document
{
namespace data
{

AddActivityChannelCommand::AddActivityChannelCommand(ConfigData &c,
    uint32_t activityPos, int channelPos, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Add Channel (to activity : %1)").arg(activityPos),
        parent), c(c), activityPos(activityPos)
{
  uint32_t channelCount;

  if (activityPos >= c.getActivities().size()) {
    return;
  }
  channelCount = c.getActivities()[activityPos].getChannels().size();
  if (channelPos < 0) {
    //append
    channelPos = channelCount;
  }
  if (channelPos > channelCount) {
    return;
  }
  this->channelPos = channelPos;
  isValid = true;
}

void AddActivityChannelCommand::redo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeAdded(Item::ACTIVITY_CHANNEL, channelPos);
  auto &channels = c.getActivities()[activityPos].getChannels();
  channels.insert(channels.begin() + channelPos, item::Channel());
  emit itemAdded(Item::ACTIVITY_CHANNEL, channelPos);
  emit dirtyChanged(true);
}

void AddActivityChannelCommand::undo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeRemoved(Item::ACTIVITY_CHANNEL, channelPos);
  auto &channels = c.getActivities()[activityPos].getChannels();
  channels.erase(channels.begin() + channelPos);
  emit itemRemoved(Item::ACTIVITY_CHANNEL, channelPos);
  emit dirtyChanged(true);
}

bool AddActivityChannelCommand::valid() const
{
  return isValid;
}

}
}
