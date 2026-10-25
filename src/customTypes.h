// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QMetaType>

#include "bin/irProto/file.h"
#include "bin/ssIr/file.h"
#include "document/files/database/keyCode.h"

Q_DECLARE_METATYPE(binary::ssIr::SerialStreamIr);
Q_DECLARE_METATYPE(binary::irProto::IrProto);
Q_DECLARE_METATYPE(binary::irProto::Code);
Q_DECLARE_METATYPE(document::files::KeyCodeNumber)

inline void registerTypes()
{
  qRegisterMetaType<binary::ssIr::SerialStreamIr>();
  qRegisterMetaType<binary::irProto::IrProto>();
  qRegisterMetaType<binary::irProto::Code>();
  qRegisterMetaType<document::files::KeyCodeNumber>();
}
