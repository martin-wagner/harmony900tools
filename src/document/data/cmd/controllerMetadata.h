// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class SetControllerMetadataCommand: public BaseCommand
{
  Q_OBJECT
  public:
    SetControllerMetadataCommand(ConfigData &c, const QString &type, const QString &mnf, const QString &model, const QString &label, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

  protected:
    ConfigData &c;

    std::string type;
    std::string mnf;
    std::string model;
    std::string label;
    std::string prevType;
    std::string prevMnf;
    std::string prevModel;
    std::string prevLabel;
};

}
}
