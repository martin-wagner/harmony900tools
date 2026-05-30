// SPDX-License-Identifier: LGPL-2.1-or-later

#include "data.h"

using namespace std;

namespace document
{
namespace data
{

ConfigData::ConfigData()
{
}

ConfigData::~ConfigData()
{
}

const std::vector<item::Activity>& ConfigData::getActivities() const
{
  return activities;
}

std::vector<item::Activity>& ConfigData::getActivities()
{
  return activities;
}

const item::Activity* ConfigData::getActivity(uint32_t idx, uint32_t *pos) const
{
  for (int i = 0; i < activities.size(); i++) {
    if (activities[i].getId() == idx) {
      if (pos != nullptr) {
        *pos = i;
      }
      return &activities[i];
    }
  }
  return nullptr;
}

item::Activity* ConfigData::getActivity(uint32_t idx, uint32_t *pos)
{
  for (int i = 0; i < activities.size(); i++) {
    if (activities[i].getId() == idx) {
      if (pos != nullptr) {
        *pos = i;
      }
      return &activities[i];
    }
  }
  return nullptr;
}

const vector<item::Blob>& ConfigData::getBlobs() const
{
  return blobs;
}

vector<item::Blob>& ConfigData::getBlobs()
{
  return blobs;
}

const vector<binary::irProto::Code>& ConfigData::getCommands() const
{
  return commands;
}

vector<binary::irProto::Code>& ConfigData::getCommands()
{
  return commands;
}

const item::ControllerInfo& ConfigData::getController() const
{
  return controller;
}

item::ControllerInfo& ConfigData::getController()
{
  return controller;
}

const std::vector<item::Device>& ConfigData::getDevices() const
{
  return devices;
}

std::vector<item::Device>& ConfigData::getDevices()
{
  return devices;
}

const item::Device* ConfigData::getDevice(uint32_t idx,
    uint32_t *pos) const
{
  for (int i = 0; i < devices.size(); i++) {
    if (devices[i].getId() == idx) {
      if (pos != nullptr) {
        *pos = i;
      }
      return &devices[i];
    }
  }
  return nullptr;
}

item::Device* ConfigData::getDevice(uint32_t idx, uint32_t *pos)
{
  for (int i = 0; i < devices.size(); i++) {
    if (devices[i].getId() == idx) {
      if (pos != nullptr) {
        *pos = i;
      }
      return &devices[i];
    }
  }
  return nullptr;
}

bool ConfigData::isIgnoreDeviceLimit() const
{
  return ignoreDeviceLimit;
}

bool& ConfigData::isIgnoreDeviceLimit()
{
  return ignoreDeviceLimit;
}

const vector<binary::irProto::File>& ConfigData::getProtocols() const
{
  return protocols;
}

vector<binary::irProto::File>& ConfigData::getProtocols()
{
  return protocols;
}

const vector<binary::ssIr::File>& ConfigData::getStreams() const
{
  return streams;
}

vector<binary::ssIr::File>& ConfigData::getStreams()
{
  return streams;
}

const item::UserInfo& ConfigData::getUser() const
{
  return user;
}

item::UserInfo& ConfigData::getUser()
{
  return user;
}

}
}
