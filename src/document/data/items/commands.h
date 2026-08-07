// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <unistd.h>
#include <string>
#include <vector>
#include <chrono>

#include "bin/ssIr/file.h"
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
    //command creator. false = as read from xml
    PropertyBool usesParentInfo { false, Used::NO };
    PropertyU32 field3 {0, Used::NO};
    PropertyU32 field4 {0, Used::NO};

    //for serialise
    PropertyString name { "Unknown" };
    PropertyU32 protocolIndex { 0 };
    PropertyU32 commandIndex { 0 };
    Property<std::vector<uint8_t>> data {{}}; //encoded IR protocol config stream
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

    /** Info for building IR commands. IR commands seem to use max two static
     * params + command. We have space for three params. */
    PropertyEnum<CodeType> codeType { CodeType::None, Used::NO} ;
    PropertyU32 field0 {0, Used::NO};
    PropertyU32 field1 {0, Used::NO};
    PropertyU32 field2 {0, Used::NO};

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
