// SPDX-License-Identifier: LGPL-2.1-or-later

#include "setUserName.h"

using namespace std;

namespace document
{
namespace data
{

SetUserNameCommand::SetUserNameCommand(ConfigData &c, const QString &firstName,
    const QString &lastName, QUndoCommand *parent) :
    BaseCommand(QObject::tr("Set user name)"), parent), c(c), firstName(
        firstName.toStdString()), lastName(lastName.toStdString())
{
  prevFirstName = c.getUser().firstName.get();
  prevLastName = c.getUser().lastName.get();
}

void SetUserNameCommand::redo()
{
  c.getUser().firstName.set(firstName);
  c.getUser().lastName.set(lastName);
  emit dirtyChanged(true);
}

void SetUserNameCommand::undo()
{
  c.getUser().firstName.set(prevFirstName);
  c.getUser().lastName.set(prevLastName);
  emit dirtyChanged(true);
}

}
}
