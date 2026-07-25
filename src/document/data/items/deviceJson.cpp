// SPDX-License-Identifier: LGPL-2.1-or-later

#include "deviceJson.h"

#include "jsonSerialise.h"

using namespace std;

namespace document
{
namespace data
{
namespace serialiser
{

//---------------------------------------------------------------------------
// button.h
//---------------------------------------------------------------------------

std::string buttonTypeToString(item::ButtonType t)
{
  switch (t) {
    case item::ButtonType::Soft:
      return "Soft";
    case item::ButtonType::Hard:
      return "Hard";
  }
  return "Soft";
}

item::ButtonType buttonTypeFromString(const std::string &s)
{
  if (s == "Hard") {
    return item::ButtonType::Hard;
  }
  return item::ButtonType::Soft;
}

void toJson(ordered_json &out, const item::Button &button)
{
  toJson(out, "Action", button.action);
  toJson(out, "Name", button.name);
  toJson(out, "File", button.file);
  toJson(out, "Position", button.position);
}

void fromJson(const ordered_json &in, item::Button &button)
{
  fromJson(in, "Action", button.action);
  fromJson(in, "Name", button.name);
  fromJson(in, "File", button.file);
  fromJson(in, "Position", button.position);
}

//---------------------------------------------------------------------------
// action.h
//---------------------------------------------------------------------------

void toJson(ordered_json &out, const item::SequenceItem &seqItem)
{
  toJson(out, "Opcode", seqItem.opcode);
  toJson(out, "Cmd", seqItem.cmd);
  toJson(out, "DeviceId", seqItem.deviceId);
  toJson(out, "DelayMs", seqItem.delayMs);
  toJson(out, "StateName", seqItem.stateName);
  toJson(out, "StateValue", seqItem.stateValue);
  toJson(out, "Modifier", seqItem.mod);

  toJson(out, "UnknownParams", seqItem.getUnknownParams());
}

void fromJson(const ordered_json &in, item::SequenceItem &seqItem)
{
  fromJson(in, "Opcode", seqItem.opcode);
  fromJson(in, "Cmd", seqItem.cmd);
  fromJson(in, "DeviceId", seqItem.deviceId);
  fromJson(in, "DelayMs", seqItem.delayMs);
  fromJson(in, "StateName", seqItem.stateName);
  fromJson(in, "StateValue", seqItem.stateValue);
  fromJson(in, "Modifier", seqItem.mod);

  fromJson(in, "UnknownParams", seqItem.getUnknownParams());
}

void toJson(ordered_json &out, const item::DeviceAction &action)
{
  toJson(out, "ActionType", action.actionType);
  toJson(out, "RepeatWillNotHarm", action.repeatWillNotHarm);

  toJsonVec(out, "Sequence", action.sequence);
}

void fromJson(const ordered_json &in, item::DeviceAction &action)
{
  fromJson(in, "ActionType", action.actionType);
  fromJson(in, "RepeatWillNotHarm", action.repeatWillNotHarm);

  fromJsonVec(in, "Sequence", action.sequence);
}

//---------------------------------------------------------------------------
// state.h
//---------------------------------------------------------------------------

void toJson(ordered_json &out, const item::DiscreteActions &actions)
{
  out["States"] = actions.states;

  ordered_json arr = ordered_json::array();
  for (const auto &action : actions.enterStateAction) {
    ordered_json actionJson;
    toJson(actionJson, action);
    arr.push_back(actionJson);
  }
  out["EnterStateAction"] = arr;
}

void fromJson(const ordered_json &in, item::DiscreteActions &actions)
{
  actions.states = in.value("States", std::vector<std::string>());

  actions.enterStateAction.clear();
  auto it = in.find("EnterStateAction");
  if (it != in.end()) {
    for (const auto &actionJson : *it) {
      item::DeviceAction action;
      fromJson(actionJson, action);
      actions.enterStateAction.push_back(action);
    }
  }
}

void toJson(ordered_json &out, const item::RelativeActions &actions)
{
  toJsonOpt(out, "ResetAction", actions.resetAction);
  out["States"] = actions.states;
  toJsonOpt(out, "NextStateAction", actions.nextStateAction);
  toJsonOpt(out, "PrevStateAction", actions.prevStateAction);
}

void fromJson(const ordered_json &in, item::RelativeActions &actions)
{
  fromJsonOpt(in, "ResetAction", actions.resetAction);
  actions.states = in.value("States", std::vector<std::string>());
  fromJsonOpt(in, "NextStateAction", actions.nextStateAction);
  fromJsonOpt(in, "PrevStateAction", actions.prevStateAction);
}

void toJson(ordered_json &out, const item::StateMachine &sm)
{
  toJson(out, "SmType", sm.smType);
  toJson(out, "DelayMs", sm.delayMs);

  toJsonOpt(out, "StartAction", sm.startAction);
  toJsonOpt(out, "FinishAction", sm.finishAction);

  if (!sm.discrete.empty()) {
    ordered_json discreteJson;
    toJson(discreteJson, sm.discrete);
    out["Discrete"] = discreteJson;
  }

  if (!sm.relative.empty()) {
    ordered_json relativeJson;
    toJson(relativeJson, sm.relative);
    out["Relative"] = relativeJson;
  }
}

void fromJson(const ordered_json &in, item::StateMachine &sm)
{
  fromJson(in, "SmType", sm.smType);
  fromJson(in, "DelayMs", sm.delayMs);

  fromJsonOpt(in, "StartAction", sm.startAction);
  fromJsonOpt(in, "FinishAction", sm.finishAction);

  auto discreteIt = in.find("Discrete");
  if (discreteIt != in.end()) {
    fromJson(*discreteIt, sm.discrete);
  }

  auto relativeIt = in.find("Relative");
  if (relativeIt != in.end()) {
    fromJson(*relativeIt, sm.relative);
  }
}

//---------------------------------------------------------------------------
// digits.h
//---------------------------------------------------------------------------

void toJson(ordered_json &out, const item::Digits &digits)
{
  ordered_json arr = ordered_json::array();
  for (const auto &action : digits) {
    ordered_json actionJson;
    toJson(actionJson, action);
    arr.push_back(actionJson);
  }
  out = arr;
}

void fromJson(const ordered_json &in, item::Digits &digits)
{
  if (!in.is_array()) {
    return;
  }

  size_t i = 0;
  for (const auto &actionJson : in) {
    if (i >= digits.size()) {
      break;
    }
    fromJson(actionJson, digits[i]);
    i++;
  }
}

void toJson(ordered_json &out, const item::Numpad &numpad)
{
  toJson(out, "FixedDigits", numpad.fixedDigits);

  if (numpad.first.has_value()) {
    ordered_json digitsJson;
    toJson(digitsJson, numpad.first.value());
    out["First"] = digitsJson;
  }
  if (numpad.middle.has_value()) {
    ordered_json digitsJson;
    toJson(digitsJson, numpad.middle.value());
    out["Middle"] = digitsJson;
  }
  if (numpad.last.has_value()) {
    ordered_json digitsJson;
    toJson(digitsJson, numpad.last.value());
    out["Last"] = digitsJson;
  }
  toJsonOpt(out, "Finish", numpad.finish);
}

void fromJson(const ordered_json &in, item::Numpad &numpad)
{
  fromJson(in, "FixedDigits", numpad.fixedDigits);

  auto readDigits = [&in](const char *key,
      std::optional<item::Digits> &target) {
        auto it = in.find(key);
        if (it == in.end()) {
          target.reset();
          return;
        }
        item::Digits digits {};
        fromJson(*it, digits);
        target = digits;
      };

  readDigits("First", numpad.first);
  readDigits("Middle", numpad.middle);
  readDigits("Last", numpad.last);

  fromJsonOpt(in, "Finish", numpad.finish);
}

//---------------------------------------------------------------------------
// commands.h
//---------------------------------------------------------------------------

void toJson(ordered_json &out, const item::RawCommand &cmd)
{
  toJson(out, "Name", cmd.name);
  toJson(out, "StreamIndex", cmd.streamIndex);
}

void fromJson(const ordered_json &in, item::RawCommand &cmd)
{
  fromJson(in, "Name", cmd.name);
  fromJson(in, "StreamIndex", cmd.streamIndex);
}

void toJson(ordered_json &out, const item::ProtoCommand &cmd)
{
  toJson(out, "UsesParentInfo", cmd.usesParentInfo);
  toJson(out, "Field3", cmd.field3);
  toJson(out, "Field4", cmd.field4);

  toJson(out, "Name", cmd.name);
  toJson(out, "ProtocolIndex", cmd.protocolIndex);

  //binary IR stream, base64 encoded
  if (cmd.data.isIncluded() == Used::YES) {
    out["Data"] = base64Encode(cmd.data.get());
  }
}

void fromJson(const ordered_json &in, item::ProtoCommand &cmd)
{
  fromJson(in, "UsesParentInfo", cmd.usesParentInfo);
  fromJson(in, "Field3", cmd.field3);
  fromJson(in, "Field4", cmd.field4);

  fromJson(in, "Name", cmd.name);
  fromJson(in, "ProtocolIndex", cmd.protocolIndex);

  auto it = in.find("Data");
  if (it != in.end()) {
    cmd.data.set(base64Decode(it->get<std::string>()));
    cmd.data.setIncluded(Used::YES);
  } else {
    cmd.data.setIncluded(Used::NO);
  }
}

void toJson(ordered_json &out, const item::Commands &commands)
{
  toJson(out, "PressPreSilenceMs", commands.pressPreSilenceMs);
  toJson(out, "PressInterKeyMs", commands.pressInterKeyMs);
  toJson(out, "HoldPreSilenceMs", commands.holdPreSilenceMs);
  toJson(out, "HoldInterKeyMs", commands.holdInterKeyMs);

  toJsonVec(out, "RawCommands", commands.getRawCommands());
  toJsonVec(out, "ProtoCommands", commands.getProtoCommands());

  toJson(out, "CodeType", commands.codeType);
  toJson(out, "Field0", commands.field0);
  toJson(out, "Field1", commands.field1);
  toJson(out, "Field2", commands.field2);

  toJson(out, "UnknownProperties", commands.getUnknownProperties());
}

void fromJson(const ordered_json &in, item::Commands &commands)
{
  fromJsonVec(in, "RawCommands", commands.getRawCommands());
  fromJsonVec(in, "ProtoCommands", commands.getProtoCommands());

  fromJson(in, "PressPreSilenceMs", commands.pressPreSilenceMs);
  fromJson(in, "PressInterKeyMs", commands.pressInterKeyMs);
  fromJson(in, "HoldPreSilenceMs", commands.holdPreSilenceMs);
  fromJson(in, "HoldInterKeyMs", commands.holdInterKeyMs);

  fromJson(in, "CodeType", commands.codeType);
  fromJson(in, "Field0", commands.field0);
  fromJson(in, "Field1", commands.field1);
  fromJson(in, "Field2", commands.field2);

  fromJson(in, "UnknownProperties", commands.getUnknownProperties());
}

//---------------------------------------------------------------------------
// device.h
//---------------------------------------------------------------------------

void toJson(ordered_json &out, const item::Device &device)
{
  ordered_json softButtonsArr = ordered_json::array();
  ordered_json hardButtonsArr = ordered_json::array();
  ordered_json buttonsObj;

  out["Id"] = device.getId();

  toJson(out, "Type", device.type);
  toJson(out, "Mnf", device.mnf);
  toJson(out, "Model", device.model);
  toJson(out, "Label", device.label);

  toJson(out, "ManualPower", device.manualPower);
  toJson(out, "AlwaysOn", device.alwaysOn);
  toJson(out, "AutoPower", device.autoPower);

  toJson(out, "AudioSwitch", device.audioSwitch);
  toJson(out, "Dimmer", device.dimmer);
  toJson(out, "HasBands", device.hasBands);
  toJson(out, "HasPresets", device.hasPresets);
  toJson(out, "IsNewDevice", device.isNewDevice);
  toJson(out, "IsDisplayDevice", device.isDisplayDevice);
  toJson(out, "MenuOnDevice", device.menuOnDevice);
  toJson(out, "NumDiscs", device.numDiscs);
  toJson(out, "NumLights", device.numLights);
  toJson(out, "OnScreenGuide", device.onScreenGuide);
  toJson(out, "PvrType", device.pvrType);
  toJson(out, "RecordMediaFixedDisc", device.recordMediaFixedDisc);
  toJson(out, "RecordMediaRemovableVideotape",
      device.recordMediaRemovableVideotape);
  toJson(out, "RevertInput", device.revertInput);
  toJson(out, "Scart", device.scart);
  toJson(out, "TunerInput", device.tunerInput);
  toJson(out, "VideoSwitch", device.videoSwitch);

  toJson(out, "UnknownProperties", device.getUnknownProperties());

  for (const auto &button : device.getButtons()) {
    ordered_json buttonJson;
    toJson(buttonJson, button);

    if (button.getButtonType() == item::ButtonType::Soft) {
      softButtonsArr.push_back(buttonJson);
    } else {
      hardButtonsArr.push_back(buttonJson);
    }
  }
  if (!softButtonsArr.empty()) {
    buttonsObj["Soft"] = softButtonsArr;
  }
  if (!hardButtonsArr.empty()) {
    buttonsObj["Hard"] = hardButtonsArr;
  }
  if (!buttonsObj.empty()) {
    out["Buttons"] = buttonsObj;
  }
  toJsonVec(out, "StateMachines", device.getStateMachines());

  if (device.getNumpad().has_value()) {
    ordered_json numpadJson;
    toJson(numpadJson, device.getNumpad().value());
    out["Numpad"] = numpadJson;
  }

  ordered_json commandsJson;
  toJson(commandsJson, device.getIrCommands());
  out["IrCommands"] = commandsJson;
}

void fromJson(const ordered_json &in, item::Device &device)
{
  auto readButtons = [&device](const ordered_json &in,
      item::ButtonType type) {
        for (const auto &buttonJson : in) {
          item::Button button(type);
          fromJson(buttonJson, button);
          device.getButtons().push_back(button);
        }
      };

  fromJson(in, "Type", device.type);
  fromJson(in, "Mnf", device.mnf);
  fromJson(in, "Model", device.model);
  fromJson(in, "Label", device.label);

  fromJson(in, "ManualPower", device.manualPower);
  fromJson(in, "AlwaysOn", device.alwaysOn);
  fromJson(in, "AutoPower", device.autoPower);

  fromJson(in, "AudioSwitch", device.audioSwitch);
  fromJson(in, "Dimmer", device.dimmer);
  fromJson(in, "HasBands", device.hasBands);
  fromJson(in, "HasPresets", device.hasPresets);
  fromJson(in, "IsNewDevice", device.isNewDevice);
  fromJson(in, "IsDisplayDevice", device.isDisplayDevice);
  fromJson(in, "MenuOnDevice", device.menuOnDevice);
  fromJson(in, "NumDiscs", device.numDiscs);
  fromJson(in, "NumLights", device.numLights);
  fromJson(in, "OnScreenGuide", device.onScreenGuide);
  fromJson(in, "PvrType", device.pvrType);
  fromJson(in, "RecordMediaFixedDisc", device.recordMediaFixedDisc);
  fromJson(in, "RecordMediaRemovableVideotape",
      device.recordMediaRemovableVideotape);
  fromJson(in, "RevertInput", device.revertInput);
  fromJson(in, "Scart", device.scart);
  fromJson(in, "TunerInput", device.tunerInput);
  fromJson(in, "VideoSwitch", device.videoSwitch);

  fromJson(in, "UnknownProperties", device.getUnknownProperties());

  //note: device.id is set at construction time by the caller (Device has no
  //default constructor / setter), so it is not read back here.

  auto buttonsIt = in.find("Buttons");
  if (buttonsIt != in.end()) {
    auto softIt = buttonsIt->find("Soft");
    if (softIt != buttonsIt->end()) {
      readButtons(*softIt, item::ButtonType::Soft);
    }

    auto hardIt = buttonsIt->find("Hard");
    if (hardIt != buttonsIt->end()) {
      readButtons(*hardIt, item::ButtonType::Hard);
    }
  }

  fromJsonVec(in, "StateMachines", device.getStateMachines());

  auto numpadIt = in.find("Numpad");
  if (numpadIt != in.end()) {
    item::Numpad numpad;
    fromJson(*numpadIt, numpad);
    device.getNumpad() = numpad;
  } else {
    device.getNumpad().reset();
  }

  auto irCommandsIt = in.find("IrCommands");
  if (irCommandsIt != in.end()) {
    item::Commands commands;
    fromJson(*irCommandsIt, commands);
    device.setIrCommands(commands);
  }
}

}
}
}
