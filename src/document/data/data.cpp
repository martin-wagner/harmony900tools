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

const map<uint32_t, item::Activity>& ConfigData::getActivities() const
{
  return activities;
}

map<uint32_t, item::Activity>& ConfigData::getActivities()
{
  return activities;
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

const map<uint32_t, item::Device>& ConfigData::getDevices() const
{
  return devices;
}

map<uint32_t, item::Device>& ConfigData::getDevices()
{
  return devices;
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
