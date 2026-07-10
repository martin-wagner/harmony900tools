// SPDX-License-Identifier: LGPL-2.1-or-later

#include "removeRole.h"

using namespace std;

namespace document
{
namespace data
{

RemoveRoleCommand* RemoveRoleCommand::fromId(ConfigData &c,
    uint32_t activityPos, uint32_t deviceId, QUndoCommand *parent)
{
  auto &roles = c.getActivities()[activityPos].getRoles();
  for (uint32_t i = 0; i < roles.size(); i++) {
    if (roles[i].deviceId.get() == deviceId) {
      new RemoveRoleCommand(c, activityPos, i, parent);
    }
  }
  return nullptr;
}

RemoveRoleCommand::RemoveRoleCommand(ConfigData &c, uint32_t activityPos,
    uint32_t rolePos, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Remove Role Cmd (from activity: %1)").arg(activityPos),
        parent), c(c), activityPos(activityPos), rolePos(rolePos)
{
  uint32_t roleCount;

  if (activityPos >= c.getActivities().size()) {
    //beyond end
    return;
  }

  auto &roles = c.getActivities()[activityPos].getRoles();
  roleCount = roles.size();
  if (rolePos >= roleCount) {
    return;
  }
  role = roles[rolePos];
  isValid = true;
}

void RemoveRoleCommand::redo()
{
  if (!isValid) {
    return;
  }

  auto &roles = c.getActivities()[activityPos].getRoles();
  roles.erase(roles.begin() + rolePos);
  emit dirtyChanged(true);
}

void RemoveRoleCommand::undo()
{
  if (!isValid) {
    return;
  }

  auto &roles = c.getActivities()[activityPos].getRoles();
  roles.insert(roles.begin() + rolePos, role);

  emit dirtyChanged(true);
}

bool RemoveRoleCommand::valid() const
{
  return isValid;
}


}
}
