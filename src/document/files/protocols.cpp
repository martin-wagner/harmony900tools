// SPDX-License-Identifier: LGPL-2.1-or-later

#include <fstream>
#include <nlohmann/json.hpp>
#include <QFile>

#include "protocols.h"
#include "document/data/items/irProtoJson.h"

using namespace std;
using namespace nlohmann;

namespace document
{
namespace files
{

ProtocolCatalogue::ProtocolCatalogue()
{
}

ProtocolCatalogue::ProtocolCatalogue(const QString &path) :
    jsonPath(path)
{
}

binary::irProto::IrProto ProtocolCatalogue::get(const QString &name) const
{
  try {
    return getFromJson(name);
  } catch (const exception &e) {
  }
  return binary::irProto::IrProto();
}

binary::irProto::IrProto ProtocolCatalogue::getFromJson(
    const QString &name) const
{
  int i;
  json j;

  QFile f(jsonPath);
  if (!f.open(QIODevice::ReadOnly)) {
    return binary::irProto::IrProto();
  }

  try {
    j = json::parse(f.readAll().constData());
  } catch (const nlohmann::json::parse_error &e) {
    return binary::irProto::IrProto();
  } catch (const nlohmann::json::exception &e) {
    return binary::irProto::IrProto();
  }

  auto version = j["Version"].get<uint32_t>();
  if (version != jsonVersion) {
    return binary::irProto::IrProto();

  }
  for (i = 0; i < j["IrProtocols"].size(); i++) {
    if (j["IrProtocols"][i]["Name"] != name.toStdString()) {
      continue;
    }
    binary::irProto::IrProto prot;
    data::serialiser::fromJson(j["IrProtocols"][i], prot);
    return prot;
  }

  //not found
  return binary::irProto::IrProto();
}

}
}
