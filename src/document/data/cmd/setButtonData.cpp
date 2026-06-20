// SPDX-License-Identifier: LGPL-2.1-or-later

#include "lib/timestamp.h"
#include "setButtonData.h"

using namespace std;

namespace document
{
namespace data
{

auto& getButtonRef(ConfigData &c, item::ButtonType t, uint32_t devicePos,
    int buttonPos)
{
  if (t == item::ButtonType::Hard) {
    return c.getDevices()[devicePos].getHardButtons()[buttonPos];
  } else {
    return c.getDevices()[devicePos].getSoftButtons()[buttonPos];
  }
}

SetButtonActionCommand::SetButtonActionCommand(ConfigData &c, const string &value, item::ButtonType t,
    uint32_t devicePos, int buttonPos, QUndoCommand *parent) :
    SetPropertyBaseCommand<string>(QObject::tr("set button action"),
        [&c, t, devicePos, buttonPos]() {
          return getButtonRef(c, t, devicePos, buttonPos).action.get();
        },
        [&c, t, devicePos, buttonPos](const string &v) {
          getButtonRef(c, t, devicePos, buttonPos).action.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetButtonNameCommand::SetButtonNameCommand(ConfigData &c, const string &value,
    item::ButtonType t, uint32_t devicePos, int buttonPos, QUndoCommand *parent) :
    SetPropertyBaseCommand<string>(QObject::tr("set button name"),
        [&c, t, devicePos, buttonPos]() {
          return getButtonRef(c, t, devicePos, buttonPos).name.get();
        },
        [&c, t, devicePos, buttonPos](const string &v) {
          getButtonRef(c, t, devicePos, buttonPos).name.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetButtonFileCommand::SetButtonFileCommand(ConfigData &c, const string &value,
    item::ButtonType t, uint32_t devicePos, int buttonPos, QUndoCommand *parent) :
    SetPropertyBaseCommand<string>(QObject::tr("set button file"),
        [&c, t, devicePos, buttonPos]() {
          return getButtonRef(c, t, devicePos, buttonPos).file.get();
        },
        [&c, t, devicePos, buttonPos](const string &v) {
          getButtonRef(c, t, devicePos, buttonPos).file.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

SetButtonPositionCommand::SetButtonPositionCommand(ConfigData &c, const int32_t &value,
    item::ButtonType t, uint32_t devicePos, int buttonPos, QUndoCommand *parent) :
    SetPropertyBaseCommand<int32_t>(QObject::tr("set button position"),
        [&c, t, devicePos, buttonPos]() {
          return getButtonRef(c, t, devicePos, buttonPos).position.get();
        },
        [&c, t, devicePos, buttonPos](const int32_t &v) {
          getButtonRef(c, t, devicePos, buttonPos).position.set(v).setIncluded(Include::ALWAYS);
        }, value, parent)
{
}

}
}
