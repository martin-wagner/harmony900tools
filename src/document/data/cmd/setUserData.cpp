// SPDX-License-Identifier: LGPL-2.1-or-later

#include "lib/timestamp.h"
#include "setUserData.h"

using namespace std;

namespace document
{
namespace data
{

SetUserMetadataCommand::SetUserMetadataCommand(ConfigData &c,
    const QString &user, const QString &creationTimestamp,
    const QString &modificationTimestamp, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Set user metadata)"), parent), c(c), user(
        user.toStdString()), creation(creationTimestamp.toStdString()), modification(
        modificationTimestamp.toStdString())
{
  prevUser = c.getUser().osUserName.get();
  prevCreation = c.getUser().fileCreationDate.get();
  prevModification = c.getUser().fileModificationDate.get();
}

SetUserMetadataCommand::SetUserMetadataCommand(ConfigData &c,
    QUndoCommand *parent) :
    BaseCommand(QObject::tr("Create/update user metadata)"), parent), c(c)
{
  user = qgetenv("USER");
  if (user.empty()) {
    user = qgetenv("USERNAME");
  }
  modification = lib::writeDateTime();
  creation = c.getUser().fileCreationDate.get(); //keep value if available
  if (creation.empty()) {
    creation = modification;
  }
}

void SetUserMetadataCommand::redo()
{
  c.getUser().osUserName.set(user);
  c.getUser().fileCreationDate.set(creation);
  c.getUser().fileModificationDate.set(modification);
  emit itemChanged(Item::USER, 0);
  emit dirtyChanged(true);
}

void SetUserMetadataCommand::undo()
{
  c.getUser().osUserName.set(prevUser);
  c.getUser().fileCreationDate.set(prevCreation);
  c.getUser().fileModificationDate.set(prevModification);
  emit itemChanged(Item::USER, 0);
  emit dirtyChanged(true);
}

}
}
