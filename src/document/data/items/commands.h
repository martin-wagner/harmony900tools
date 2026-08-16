// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <unistd.h>
#include <string>
#include <vector>
#include <chrono>

#include "bin/ssIr/file.h"
#include "bin/irProto/code.h"
#include "action.h"


namespace document
{
namespace data
{
namespace item
{

/** raw / timing stream command */
class RawCommand
{
  public:
    PropertyString name { "Unknown" };
    binary::ssIr::SerialStreamIr stream;
};

/** command builder */
class ProtoCommand
{
  public:
    //empty
    ProtoCommand() {};

    //create from binary
    ProtoCommand(uint32_t protocolIndex, const std::vector<uint8_t> &data, CodeType t = CodeType::Proprietary) :
      codeType(t), protocolIndex(protocolIndex), data(data)
    {
      status = command.parse(std::vector<uint8_t>(data));
      if (status == binary::irProto::Status::OK) {
        canDecode.set(true).setIncluded(Used::YES);
      } else {
        this->data.setIncluded(Used::YES);
      }
    }
    //get status after constructor call
    binary::irProto::Status getStatus() { return status; };

    // from parent data
    ProtoCommand(uint32_t protocolIndex, CodeType t) :
      codeType(t), canDecode(true)
    {
    }

    //protocol
    PropertyString name { "Unknown" };
    PropertyEnum<CodeType> codeType { CodeType::None} ;
    PropertyU32 protocolIndex { 0 };
    //command writer. false = data as read from xml, decoding not possible
    PropertyBool canDecode { false, Used::YES };

    //data for command writer
    PropertyString libName { "Unknown" }; //name assigned by the IR (de)coder lib. just pass trough
    PropertyU32 irAddress {0} ;   //device address, decoded
    PropertyU32 irCommand {0} ;   //device command, decoded
    PropertyU8 irBitCount {0} ;  //bit count in bitData
    PropertyU64 irBitData {0};   //ir command raw bit stream, not decoded

    //command writer output
    binary::irProto::Code command;
    //raw data output (canDecode == false)
    Property<std::vector<uint8_t>> data {{}, Used::NO};

  private:
    binary::irProto::Status status = binary::irProto::Status::OK;

};

/** IR command list
 *
 * we support raw / timing stream commands and the proto / command builder
 */
class Commands
{
  public:
    const std::vector<RawCommand>& getRawCommands() const
    {
      return rawCommands;
    }

    std::vector<RawCommand>& getRawCommands()
    {
      return rawCommands;
    }

    const std::vector<ProtoCommand>& getProtoCommands() const
    {
      return protoCommands;
    }

    std::vector<ProtoCommand>& getProtoCommands()
    {
      return protoCommands;
    }

    bool nameExists(const std::string &name) const
    {
      for (const RawCommand &cmd : rawCommands) {
        if (cmd.name.get() == name) {
          return true;
        }
      }

      for (const ProtoCommand &cmd : protoCommands) {
        if (cmd.name.get() == name) {
          return true;
        }
      }

      return false;
    }

    std::vector<std::string> getAvailableCommands() const
    {
      std::vector<std::string> cmds;

      for (const auto &cmd : rawCommands) {
        cmds.push_back(cmd.name.get());
      }
      for (const auto &cmd : protoCommands) {
        cmds.push_back(cmd.name.get());
      }
      return cmds;
    }

  public:
    //limits for various delays between commands
    static constexpr int SILENCE_MIN = 50;
    static constexpr int SILENCE_MAX = 5000;

    PropertyU32 pressPreSilenceMs {300, Used::YES};
    PropertyU32 pressInterKeyMs {100, Used::YES};
    PropertyU32 holdPreSilenceMs {50, Used::YES};
    PropertyU32 holdInterKeyMs {100, Used::YES};

    const std::vector<UnknownElement> &getUnknownProperties() const
    {
      return unknownProperties;
    }

    std::vector<UnknownElement> &getUnknownProperties()
    {
      return unknownProperties;
    }

  protected:
    std::vector<RawCommand> rawCommands;
    std::vector<ProtoCommand> protoCommands;

    std::vector<UnknownElement> unknownProperties;
};

}
}
}
