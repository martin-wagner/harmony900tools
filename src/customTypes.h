// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QMetaType>

#include "bin/irProto/file.h"
#include "bin/ssIr/file.h"

Q_DECLARE_METATYPE(binary::ssIr::SerialStreamIr);
Q_DECLARE_METATYPE(binary::irProto::IrProto);
Q_DECLARE_METATYPE(binary::irProto::Code);

inline void registerTypes()
{
  qRegisterMetaType<binary::ssIr::SerialStreamIr>();
  qRegisterMetaType<binary::irProto::IrProto>();
  qRegisterMetaType<binary::irProto::Code>();
}
