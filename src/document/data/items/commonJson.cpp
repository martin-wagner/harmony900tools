// SPDX-License-Identifier: LGPL-2.1-or-later


#include "lib/uid.h"
#include "commonJson.h"

#include "jsonSerialise.h"

using namespace std;

namespace document
{
namespace data
{
namespace serialiser
{

//---------------------------------------------------------------------------
// userInfo.h
//---------------------------------------------------------------------------

void toJson(ordered_json &out, const item::UserInfo &user)
{
  out["Id"] = user.getId();

  toJson(out, "FirstName", user.firstName);
  toJson(out, "LastName", user.lastName);
  toJson(out, "OsUserName", user.osUserName);
  toJson(out, "FileCreationDate", user.fileCreationDate);
  toJson(out, "FileModificationDate", user.fileModificationDate);

  toJson(out, "NewDeviceFound", user.newDeviceFound);
  toJson(out, "TrainingWheels", user.trainingWheels);
  toJson(out, "Locale", user.locale);
  toJson(out, "TimeFormat", user.timeFormat);

  toJson(out, "UnknownProperties", user.getUnknownProperties());
}

void fromJson(const ordered_json &in, item::UserInfo &user)
{
  uint32_t id;

  auto it = in.find("Id");
  if (it != in.end()) {
    id = it->get<uint32_t>();
  } else {
    id = item::UserInfo::DEFAULT_USERID;
  }
  user.setId(id);
  lib::UidGenerator::getInstance().markUsed(id);

  fromJson(in, "FirstName", user.firstName);
  fromJson(in, "LastName", user.lastName);
  fromJson(in, "OsUserName", user.osUserName);
  fromJson(in, "FileCreationDate", user.fileCreationDate);
  fromJson(in, "FileModificationDate", user.fileModificationDate);

  fromJson(in, "NewDeviceFound", user.newDeviceFound);
  fromJson(in, "TrainingWheels", user.trainingWheels);
  fromJson(in, "Locale", user.locale);
  fromJson(in, "TimeFormat", user.timeFormat);

  fromJson(in, "UnknownProperties", user.getUnknownProperties());
}

//---------------------------------------------------------------------------
// controllerInfo.h
//---------------------------------------------------------------------------

void toJson(ordered_json &out, const item::ControllerInfo &controller)
{
  out["Id"] = controller.getId();

  toJson(out, "Type", controller.type);
  toJson(out, "Mnf", controller.mnf);
  toJson(out, "Model", controller.model);
  toJson(out, "Label", controller.label);

  toJson(out, "UnknownProperties", controller.getUnknownProperties());
}

void fromJson(const ordered_json &in, item::ControllerInfo &controller)
{
  uint32_t id;

  auto it = in.find("Id");
  if (it != in.end()) {
    id = it->get<uint32_t>();
  } else {
    id = item::ControllerInfo::DEFAULT_ID;
  }
  controller.setId(id);
  lib::UidGenerator::getInstance().markUsed(id);

  fromJson(in, "Type", controller.type);
  fromJson(in, "Mnf", controller.mnf);
  fromJson(in, "Model", controller.model);
  fromJson(in, "Label", controller.label);

  fromJson(in, "UnknownProperties", controller.getUnknownProperties());
}

//---------------------------------------------------------------------------
// items/blob.h
//---------------------------------------------------------------------------

void toJson(ordered_json &out, const item::Blob &blob)
{
  out["File"] = blob.getFile();
  out["Permissions"] = blob.getPermissions();

  //genuinely binary payload (e.g. embedded image) -- base64 is the
  //appropriate exception here, unlike the readable structured fields
  //elsewhere in this file.
  QByteArray bytes(reinterpret_cast<const char*>(blob.getData().data()), int(blob.getData().size()));
  out["Data"] = bytes.toBase64().toStdString();
}

item::Blob fromJson(const ordered_json &in)
{
  std::string file = in.value("File", std::string());

  item::Permissions permissions{6, 6, 6};
  auto permIt = in.find("Permissions");
  if (permIt != in.end() && permIt->is_array() && permIt->size() == permissions.size()) {
    for (size_t i = 0; i < permissions.size(); i++) {
      permissions[i] = (*permIt)[i].get<uint8_t>();
    }
  }

  std::vector<uint8_t> data;
  auto dataIt = in.find("Data");
  if (dataIt != in.end()) {
    QByteArray decoded = QByteArray::fromBase64(QByteArray::fromStdString(dataIt->get<std::string>()));
    data.assign(decoded.begin(), decoded.end());
  }

  return item::Blob(file, data, permissions);
}

}
}
}
