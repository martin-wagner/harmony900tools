// SPDX-License-Identifier: LGPL-2.1-or-later

#include "lib/timestamp.h"
#include "setControllerMetadata.h"

using namespace std;

namespace document
{
namespace data
{

SetControllerMetadataCommand::SetControllerMetadataCommand(ConfigData &c,
    const QString &type, const QString &mnf, const QString &model,
    const QString &label, QUndoCommand *parent):
        BaseCommand(QObject::tr("Set controller metadata)"), parent), c(c), type(
            type.toStdString()), mnf(mnf.toStdString()), model(
                model.toStdString()), label(
                    label.toStdString())
{
  prevType = c.getController().type.get();
  prevMnf = c.getController().mnf.get();
  prevModel = c.getController().model.get();
  prevLabel = c.getController().label.get();
}

void SetControllerMetadataCommand::redo()
{
  c.getController().type.set(type);
  c.getController().mnf.set(mnf);
  c.getController().model.set(model);
  c.getController().label.set(label);
  emit dirtyChanged(true);
}

void SetControllerMetadataCommand::undo()
{
  c.getController().type.set(prevType);
  c.getController().mnf.set(prevMnf);
  c.getController().model.set(prevModel);
  c.getController().label.set(prevLabel);
  emit dirtyChanged(true);
}


}
}
