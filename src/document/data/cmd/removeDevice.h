// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class RemoveDeviceFromActivityCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveDeviceFromActivityCommand(std::set<uint32_t> ids, item::Activity &a, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

    void activityAdded(uint32_t id);
    void activityAboutToBeRemoved(uint32_t id);
    void activityRemoved(uint32_t id);
};

class RemoveDeviceCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveDeviceCommand(ConfigData &c, uint32_t id, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

    void deviceAdded(uint32_t index);
    void deviceAboutToBeRemoved(uint32_t id);
    void deviceRemoved(uint32_t index);

    void activityAdded(uint32_t id);
    void activityAboutToBeRemoved(uint32_t id);
    void activityRemoved(uint32_t id);

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t id;
    item::Device device;
};

}
}
