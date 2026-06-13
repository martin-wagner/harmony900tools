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
  modification = lib::writeTime();
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
  emit dirtyChanged(true);
}

void SetUserMetadataCommand::undo()
{
  c.getUser().osUserName.set(prevUser);
  c.getUser().fileCreationDate.set(prevCreation);
  c.getUser().fileModificationDate.set(prevModification);
  emit dirtyChanged(true);
}

SetUserNewDeviceFoundCommand::SetUserNewDeviceFoundCommand(ConfigData &c,
    bool value, QUndoCommand *parent) :
    SetPropertyBaseCommand<bool>(QObject::tr("set new device found"),
        [&c]() {return c.getUser().newDeviceFound.get();}, [&c](const bool &v) {
          c.getUser().newDeviceFound.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetUserTrainingWheelsCommand::SetUserTrainingWheelsCommand(ConfigData &c,
    bool value, QUndoCommand *parent) :
    SetPropertyBaseCommand<bool>(QObject::tr("set training wheels"),
        [&c]() {return c.getUser().trainingWheels.get();}, [&c](const bool &v) {
          c.getUser().trainingWheels.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetUserLocaleCommand::SetUserLocaleCommand(ConfigData &c,
    const Enum<Locale> &value, QUndoCommand *parent) :
    SetPropertyBaseCommand<Enum<Locale>>(QObject::tr("set locale"),
        [&c]() {return c.getUser().locale.get();}, [&c](const Enum<Locale> &v) {
          c.getUser().locale.set(v).setIncluded(Include::ALWAYS);}, value,
        parent)
{
}

SetUserTimeFormatCommand::SetUserTimeFormatCommand(ConfigData &c,
    const Enum<TimeFormat> &value, QUndoCommand *parent) :
    SetPropertyBaseCommand<Enum<TimeFormat>>(QObject::tr("set time format"),
        [&c]() {return c.getUser().timeFormat.get();},
        [&c](const Enum<TimeFormat> &v) {
          c.getUser().timeFormat.set(v).setIncluded(Include::ALWAYS);}, value,
        parent)
{
}

}
}
