// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class AddDeviceCommand: public BaseCommand
{
  Q_OBJECT
  public:
    AddDeviceCommand(ConfigData &c, QUndoCommand *parent = nullptr);
    AddDeviceCommand(ConfigData &c, uint32_t id, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    uint32_t getUid() const;
    bool valid() const;

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

    void deviceAdded(uint32_t index);
    void deviceAboutToBeRemoved(uint32_t id);
    void deviceRemoved(uint32_t index);

  protected:
    bool isValid = false;
    ConfigData &c;
    uint32_t id;
};

}
}
