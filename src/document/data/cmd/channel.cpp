// SPDX-License-Identifier: LGPL-2.1-or-later

#include "channel.h"

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

  emit itemAboutToBeRemoved(Item::ACTIVITY_CHANNEL, channelPos);
  auto &channels = c.getActivities()[activityPos].getChannels();
  channels.erase(channels.begin() + channelPos);
  emit itemRemoved(Item::ACTIVITY_CHANNEL, channelPos);
  emit dirtyChanged(true);
}

void RemoveActivityChannelCommand::undo()
{
  if (!isValid) {
    return;
  }

  emit itemAboutToBeAdded(Item::ACTIVITY_CHANNEL, channelPos);
  auto &channels = c.getActivities()[activityPos].getChannels();
  channels.insert(channels.begin() + channelPos, channel);
  emit itemAdded(Item::ACTIVITY_CHANNEL, channelPos);
  emit dirtyChanged(true);
}

bool RemoveActivityChannelCommand::valid() const
{
  return isValid;
}


static item::Channel* getChannel(ConfigData &c, uint32_t activityPos,
    uint32_t chPos)
{
  try {
    return &c.getActivities().at(activityPos).getChannels().at(chPos);
  } catch (std::out_of_range&) {
  }
  return nullptr;
}

SetActivityChannelCommand::SetActivityChannelCommand(ConfigData &c,
    uint32_t activityPos, uint32_t chPos, const item::Channel &act,
    QUndoCommand *parent) :
    BaseCommand(QObject::tr("Edit Activity channel"), parent), c(c), activityPos(
        activityPos), chPos(chPos), act(act)
{
  auto *channel = getChannel(c, activityPos, chPos);
  if (channel == nullptr) {
    return;
  }
  oldAct = *channel;

  isValid = true;
}
void SetActivityChannelCommand::redo()
{
  if (!isValid) {
    return;
  }

  auto *channel = getChannel(c, activityPos, chPos);
  *channel = act;
  emit itemChanged(Item::ACTIVITY_CHANNEL, activityPos);
  emit dirtyChanged(true);
}
void SetActivityChannelCommand::undo()
{
  if (!isValid) {
    return;
  }

  auto *channel = getChannel(c, activityPos, chPos);
  *channel = oldAct;
  emit itemChanged(Item::ACTIVITY_CHANNEL, activityPos);
  emit dirtyChanged(true);
}
bool SetActivityChannelCommand::valid() const
{
  return isValid;
}

}
}
