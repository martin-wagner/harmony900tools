// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "bin/ssIr/file.h"
#include "bin/irProto/file.h"
#include "bin/irProto/code.h"
#include "items/activity.h"
#include "items/blob.h"
#include "items/controllerInfo.h"
#include "items/device.h"
#include "items/userInfo.h"


namespace document
{
namespace data
{

/** this contains a full config data elemnent */
class ConfigData
{
  public:
    ConfigData();
    ~ConfigData();

    const std::vector<item::Activity>& getActivities() const;
    std::vector<item::Activity>& getActivities();

    const item::Activity* getActivity(uint32_t id, uint32_t *pos = nullptr) const;
    item::Activity* getActivity(uint32_t id, uint32_t *pos = nullptr);

    const std::vector<item::Blob>& getBlobs() const;
    std::vector<item::Blob>& getBlobs();

    const std::vector<binary::irProto::Code>& getCommands() const;
    std::vector<binary::irProto::Code>& getCommands();

    const item::ControllerInfo& getController() const;
    item::ControllerInfo& getController();

    const std::vector<item::Device>& getDevices() const;
    std::vector<item::Device>& getDevices();

    const item::Device* getDevice(uint32_t id, uint32_t *pos = nullptr) const;
    item::Device* getDevice(uint32_t id, uint32_t *pos = nullptr);

    const std::vector<uint32_t> getDeviceIds() const;
    const std::vector<std::string> getDeviceLabels() const;

    bool isIgnoreDeviceLimit() const;
    bool& isIgnoreDeviceLimit();

    const binary::irProto::File& getProtocolLib() const;
    binary::irProto::File& getProtocolLib();

    const binary::ssIr::File& getStreamLib() const;
    binary::ssIr::File& getStreamLib();

    const item::UserInfo& getUser() const;
    item::UserInfo& getUser();

    /** max 15 devices (limit from the user manual, not actually checked) */
    static constexpr uint32_t DEVICE_LIMIT = 15; //todo use this

  protected:
    /** device limit. set once, clear never */
    bool ignoreDeviceLimit = false;

    /** user config */
    item::UserInfo user;
    item::ControllerInfo controller;
    std::vector<item::Device> devices;
    std::vector<item::Activity> activities;
    std::vector<item::Blob> blobs;

    /** ir command data */
    binary::ssIr::File streamLib; //raw timing streams
    std::vector<binary::irProto::Code> commands; // command data
    binary::irProto::File protocolLib; // command-to-stream encoder data
};




}
}


