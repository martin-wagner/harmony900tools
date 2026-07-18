// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <unistd.h>
#include <string>
#include <vector>
#include <chrono>

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
    PropertyU16 streamIndex { 0 };

};

/** command builder */
class ProtoCommand
{
  public:
    //command creator. false = read from xml
    PropertyBool usesParentInfo { false, Used::NO };
    PropertyU32 field3 {0, Used::NO};
    PropertyU32 field4 {0, Used::NO};

    //for serialise
    PropertyString name { "Unknown" };
    PropertyU32 protocolIndex { 0 };
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

    PropertyU32 pressPreSilenceMs {0, Used::YES};
    PropertyU32 pressInterKeyMs {0, Used::YES};
    PropertyU32 holdPreSilenceMs {0, Used::YES};
    PropertyU32 holdInterKeyMs {0, Used::YES};

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
