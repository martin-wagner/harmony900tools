// SPDX-License-Identifier: LGPL-2.1-or-later

#include "role.h"

using namespace std;

namespace document
{
namespace data
{

SetRoleCommand::SetRoleCommand(ConfigData &c, uint32_t activityPos,
    item::Role &role, int rolePos, bool overwrite, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Set Proto role (activity: %1)").arg(activityPos),
        parent), isValid(false), overwrite(overwrite), c(c), activityPos(
        activityPos), rolePos(rolePos), role(role)
{
  if (activityPos >= c.getActivities().size()) {
    return;
  }
  auto &roles = c.getActivities()[activityPos].getRoles();
  //roles must be unique!
  for (const auto &existing : roles) {
    if (role.role.getValue() == existing.role.getValue()) {
      emit writeMsg(tr("Role %1 already exists").arg(role.role.getQString()));
      return;
    }
  }

  uint32_t roleCount = roles.size();

  if (overwrite) {
    if ((rolePos < 0) || (static_cast<uint32_t>(rolePos) >= roleCount)) {
      return;
    }
    prevRole = roles[rolePos];
  } else {
    if (rolePos < 0) {
      rolePos = static_cast<int>(roleCount);
    }
    if (static_cast<uint32_t>(rolePos) > roleCount) {
      return;
    }
  }

  this->rolePos = rolePos;
  isValid = true;
}

void SetRoleCommand::redo()
{
  if (!isValid) {
    return;
  }

  auto &roles = c.getActivities()[activityPos].getRoles();
  if (overwrite) {
    roles[rolePos] = role;
  } else {
    roles.insert(roles.begin() + rolePos, role);
  }

  emit dirtyChanged(true);
}

void SetRoleCommand::undo()
{
  if (!isValid) {
    return;
  }

  auto &roles = c.getActivities()[activityPos].getRoles();
  if (overwrite) {
    roles[rolePos] = prevRole;
  } else {
    roles.erase(roles.begin() + rolePos);
  }

  emit dirtyChanged(true);
}

bool SetRoleCommand::valid() const
{
  return isValid;
}



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
