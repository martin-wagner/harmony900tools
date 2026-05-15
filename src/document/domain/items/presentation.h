// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <unistd.h>
#include <string>
#include <vector>

#include "button.h"
#include "document/domain/enum.h"

namespace document
{
namespace domain
{
namespace item
{

/** stores how softbuttons / pushbuttons work
 */
class Presentation
{
  public:
    const std::vector<Button>& getHardButtons() const
    {
      return hardButtons;
    }

    void setHardButtons(const std::vector<Button> &hardButtons)
    {
      this->hardButtons = hardButtons;
    }

    const std::string& getName() const
    {
      return name;
    }

    void setName(const std::string &name = "Generic")
    {
      this->name = name;
    }

    const std::vector<Button>& getSoftButtons() const
    {
      return softButtons;
    }

    void setSoftButtons(const std::vector<Button> &softButtons)
    {
      this->softButtons = softButtons;
    }

  protected:
    std::string name = "Generic";
    std::vector<Button> softButtons;
    std::vector<Button> hardButtons;
};

}
}
}
