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
    RawCommand(const std::string &name, uint32_t index = 0) :
        name(name), streamIndex(index)
    {
    }

    const std::string& getName() const
    {
      return name;
    }

    void setName(const std::string &name)
    {
      this->name = name;
    }

    uint32_t getStreamIndex() const
    {
      return streamIndex;
    }

    void setStreamIndex(uint32_t streamIndex)
    {
      this->streamIndex = streamIndex;
    }

  protected:
    std::string name;
    uint32_t streamIndex;
};

/** command builder */
class ProtoCommand
{
  public:
    ProtoCommand(const std::string &name, uint32_t index = 0) :
        name(name), protocolIndex(index)
    {
    }

    const std::string& getName() const
    {
      return name;
    }

    void setName(const std::string &name)
    {
      this->name = name;
    }

    uint32_t getIndex() const
    {
      return protocolIndex;
    }

    void setIndex(uint32_t protocolIndex)
    {
      this->protocolIndex = protocolIndex;
    }

  protected:
    std::string name;
    uint32_t protocolIndex;
};

/** IR command list
 *
 * we support raw / timing stream commands and the proto / command builder
 */
class Commands
{
  public:
    Commands()
    {
    }

    const std::chrono::milliseconds& getHoldInterKey() const
    {
      return holdInterKey;
    }

    void setHoldInterKey(const std::chrono::milliseconds &holdInterKey)
    {
      this->holdInterKey = holdInterKey;
    }

    const std::chrono::milliseconds& getHoldPreSilence() const
    {
      return holdPreSilence;
    }

    void setHoldPreSilence(const std::chrono::milliseconds &holdPreSilence)
    {
      this->holdPreSilence = holdPreSilence;
    }

    const std::chrono::milliseconds& getPressInterKey() const
    {
      return pressInterKey;
    }

    void setPressInterKey(const std::chrono::milliseconds &pressInterKey)
    {
      this->pressInterKey = pressInterKey;
    }

    const std::chrono::milliseconds& getPressPreSilence() const
    {
      return pressPreSilence;
    }

    void setPressPreSilence(const std::chrono::milliseconds &pressPreSilence)
    {
      this->pressPreSilence = pressPreSilence;
    }

    const std::vector<ProtoCommand>& getProtoCommands() const
    {
      return protoCommands;
    }

    void setProtoCommands(const std::vector<ProtoCommand> &protoCommands)
    {
      this->protoCommands = protoCommands;
    }

    const std::vector<RawCommand>& getRawCommands() const
    {
      return rawCommands;
    }

    void setRawCommands(const std::vector<RawCommand> &rawCommands)
    {
      this->rawCommands = rawCommands;
    }

  protected:
    std::chrono::milliseconds pressPreSilence;
    std::chrono::milliseconds pressInterKey;
    std::chrono::milliseconds holdPreSilence;
    std::chrono::milliseconds holdInterKey;

    std::vector<RawCommand> rawCommands;
    std::vector<ProtoCommand> protoCommands;


};

}
}
}
