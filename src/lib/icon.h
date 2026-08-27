// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QIcon>

namespace lib
{

inline QIcon getIcon(const QString &icon, const QString &fromTheme = "")
{
  return QIcon::fromTheme(fromTheme, QIcon(icon));
  //for testing
  //return QIcon(icon);
}

inline QIcon getDeleteIcon()
{
  return lib::getIcon(
      ":/res/icons/BreezeConverted/64x64/actions/edit-delete.png",
      "edit-delete");
}

inline QIcon getAddIcon()
{
  return lib::getIcon(":/res/icons/BreezeConverted/64x64/actions/list-add.png",
      "list-add");
}

inline QIcon getEditIcon()
{
  return lib::getIcon(":/res/icons/BreezeConverted/64x64/actions/text-field.png",
      "accessories-text-editor");
}

}
