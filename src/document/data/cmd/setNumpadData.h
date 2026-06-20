// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"

namespace document
{
namespace data
{

class SetNumpadFixedDigitsCommand: public SetPropertyBaseCommand<uint32_t>
{
  public:
    SetNumpadFixedDigitsCommand(ConfigData &c, uint32_t value,
        uint32_t devicePos, QUndoCommand *parent = nullptr);
};

}
}
