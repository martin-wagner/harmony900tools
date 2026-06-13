// SPDX-License-Identifier: LGPL-2.1-or-later

#include "lib/timestamp.h"
#include "setButtonData.h"

using namespace std;

namespace document
{
namespace data
{

SetActionCommand::SetActionCommand(ConfigData &c, const string &value, item::ButtonType t,
    uint32_t devicePos, int buttonPos, QUndoCommand *parent) :
    SetPropertyBaseCommand<string>(QObject::tr("set button action"),
        [&c, t, devicePos, buttonPos]() {
          if (t == item::ButtonType::Hard) {
            return c.getDevices()[devicePos].getHardButtons()[buttonPos].action.get();
          } else {
            return c.getDevices()[devicePos].getSoftButtons()[buttonPos].action.get();
          }
        },
        [&c, t, devicePos, buttonPos](const string &v) {
          if (t == item::ButtonType::Hard) {
            c.getDevices()[devicePos].getHardButtons()[buttonPos].action.set(v).setIncluded(Include::ALWAYS);
          } else {
            return c.getDevices()[devicePos].getSoftButtons()[buttonPos].action.set(v).setIncluded(Include::ALWAYS);
          }
        }, value, parent)
{
}

SetNameCommand::SetNameCommand(ConfigData &c, const string &value,
    item::ButtonType t, uint32_t devicePos, int buttonPos, QUndoCommand *parent) :
    SetPropertyBaseCommand<string>(QObject::tr("set button name"),
        [&c, t, devicePos, buttonPos]() {
          if (t == item::ButtonType::Hard) {
            return c.getDevices()[devicePos].getHardButtons()[buttonPos].name.get();
          } else {
            return c.getDevices()[devicePos].getSoftButtons()[buttonPos].name.get();
          }
        },
        [&c, t, devicePos, buttonPos](const string &v) {
          if (t == item::ButtonType::Hard) {
            c.getDevices()[devicePos].getHardButtons()[buttonPos].name.set(v).setIncluded(Include::ALWAYS);
          } else {
            c.getDevices()[devicePos].getSoftButtons()[buttonPos].name.set(v).setIncluded(Include::ALWAYS);
          }
        }, value, parent)
{
}

SetFileCommand::SetFileCommand(ConfigData &c, const string &value,
    item::ButtonType t, uint32_t devicePos, int buttonPos, QUndoCommand *parent) :
    SetPropertyBaseCommand<string>(QObject::tr("set button file"),
        [&c, t, devicePos, buttonPos]() {
          if (t == item::ButtonType::Hard) {
            return c.getDevices()[devicePos].getHardButtons()[buttonPos].file.get();
          } else {
            return c.getDevices()[devicePos].getSoftButtons()[buttonPos].file.get();
          }
        },
        [&c, t, devicePos, buttonPos](const string &v) {
          if (t == item::ButtonType::Hard) {
            c.getDevices()[devicePos].getHardButtons()[buttonPos].file.set(v).setIncluded(Include::ALWAYS);
          } else {
            c.getDevices()[devicePos].getSoftButtons()[buttonPos].file.set(v).setIncluded(Include::ALWAYS);
          }
        }, value, parent)
{
}

SetPositionCommand::SetPositionCommand(ConfigData &c, const int32_t &value,
    item::ButtonType t, uint32_t devicePos, int buttonPos, QUndoCommand *parent) :
    SetPropertyBaseCommand<int32_t>(QObject::tr("set button position"),
        [&c, t, devicePos, buttonPos]() {
          if (t == item::ButtonType::Hard) {
            return c.getDevices()[devicePos].getHardButtons()[buttonPos].position.get();
          } else {
            return c.getDevices()[devicePos].getSoftButtons()[buttonPos].position.get();
          }
        },
        [&c, t, devicePos, buttonPos](const int32_t &v) {
          if (t == item::ButtonType::Hard) {
            c.getDevices()[devicePos].getHardButtons()[buttonPos].position.set(v).setIncluded(Include::ALWAYS);
          } else {
            c.getDevices()[devicePos].getSoftButtons()[buttonPos].position.set(v).setIncluded(Include::ALWAYS);
          }
        }, value, parent)
{
}

}
}
