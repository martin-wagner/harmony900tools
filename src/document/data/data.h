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

    const item::Activity* getActivity(uint32_t idx, uint32_t *pos = nullptr) const;
    item::Activity* getActivity(uint32_t idx, uint32_t *pos = nullptr);

    const std::vector<item::Blob>& getBlobs() const;
    std::vector<item::Blob>& getBlobs();

    const std::vector<binary::irProto::Code>& getCommands() const;
    std::vector<binary::irProto::Code>& getCommands();

    const item::ControllerInfo& getController() const;
    item::ControllerInfo& getController();

    const std::vector<item::Device>& getDevices() const;
    std::vector<item::Device>& getDevices();

    const item::Device* getDevice(uint32_t idx, uint32_t *pos = nullptr) const;
    item::Device* getDevice(uint32_t idx, uint32_t *pos = nullptr);

    bool isIgnoreDeviceLimit() const;
    bool& isIgnoreDeviceLimit();

    const std::vector<binary::irProto::File>& getProtocols() const;
    std::vector<binary::irProto::File>& getProtocols();

    const std::vector<binary::ssIr::File>& getStreams() const;
    std::vector<binary::ssIr::File>& getStreams();

    const item::UserInfo& getUser() const;
    item::UserInfo& getUser();

    /** max 15 devices (limit from the user manual, not actually checked) */
    static constexpr uint32_t DEVICE_LIMIT = 15;

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
    std::vector<binary::ssIr::File> streams; //raw timing streams
    std::vector<binary::irProto::Code> commands; // command data
    std::vector<binary::irProto::File> protocols; // command-to-stream encoder data
};




}
}


