// SPDX-License-Identifier: LGPL-2.1-or-later

#include "data.h"

using namespace std;

namespace document
{
namespace data
{

Config::Config()
{
}

Config::~Config()
{
}

const map<uint32_t, item::Activity>& Config::getActivities() const
{
  return activities;
}

map<uint32_t, item::Activity>& Config::getActivities()
{
  return activities;
}

const vector<item::Blob>& Config::getBlobs() const
{
  return blobs;
}

vector<item::Blob>& Config::getBlobs()
{
  return blobs;
}

const vector<binary::irProto::Code>& Config::getCommands() const
{
  return commands;
}

vector<binary::irProto::Code>& Config::getCommands()
{
  return commands;
}

const item::ControllerInfo& Config::getController() const
{
  return controller;
}

item::ControllerInfo& Config::getController()
{
  return controller;
}

const map<uint32_t, item::Device>& Config::getDevices() const
{
  return devices;
}

map<uint32_t, item::Device>& Config::getDevices()
{
  return devices;
}

bool Config::isIgnoreDeviceLimit() const
{
  return ignoreDeviceLimit;
}

bool& Config::isIgnoreDeviceLimit()
{
  return ignoreDeviceLimit;
}

const vector<binary::irProto::File>& Config::getProtocols() const
{
  return protocols;
}

vector<binary::irProto::File>& Config::getProtocols()
{
  return protocols;
}

const vector<binary::ssIr::File>& Config::getStreams() const
{
  return streams;
}

vector<binary::ssIr::File>& Config::getStreams()
{
  return streams;
}

const item::UserInfo& Config::getUser() const
{
  return user;
}

item::UserInfo& Config::getUser()
{
  return user;
}

}
}
