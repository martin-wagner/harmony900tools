// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QObject>

#include "ui/logViewer.h"
#include "bin/irProto/file.h"

namespace document
{

namespace files
{

class ProtocolCatalogue : public QObject
{
  Q_OBJECT
  public:
    ProtocolCatalogue();

    /** get a protocol from the name. check "ret.isEmpty()" for success */
    binary::irProto::IrProto get(const QString &name) const;

  public:
    const QString jsonPath = ":/res/ir_protocols.json";
    const uint32_t jsonVersion = 1;

  private:
    binary::irProto::IrProto getFromJson(const QString &name) const;
};

}
}


