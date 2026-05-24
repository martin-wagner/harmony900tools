// SPDX-License-Identifier: LGPL-2.1-or-later

#include "base.h"

namespace document
{
namespace data
{

BaseCommand::BaseCommand(const QString &text, QUndoCommand *parent) :
    QObject(nullptr),
    QUndoCommand(text, parent)
{
}

}
}
