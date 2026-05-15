// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <unistd.h>
#include <string>
#include <vector>

#include "document/domain/enum.h"

namespace document
{
namespace domain
{
namespace item
{

enum class ButtonType { Soft, Hard };

/** stores meta info for a button
 */
class Button
{
  public:
    static constexpr std::string ACTION_POSTFIX = "_Hold";

    Button(ButtonType t, const std::string &name, const std::string &action, int position = -1);
    Button(std::string t, const std::string &name, const std::string &action, int position = -1);

    ButtonType getButtonType()
    {
      return t;
    }

    /** command that will be sent on press */
    const std::string& getAction(int id) const
    {
      return action;
    }

    /** for export \todo ist das hier richtig? */
    const std::string getActionId(int id) const
    {
      return std::to_string(id) + "_" + action + ACTION_POSTFIX;
    }

    /** command that will be sent on press */
    void setAction(const std::string &action)
    {
      this->action = action;
    }

    /** hard button: name, soft button: label */
    const std::string& getName() const
    {
      return name;
    }

    /** hard button: name, soft button: label */
    void setName(const std::string &name)
    {
      this->name = name;
    }

    /** only soft button: channel icon */
    const std::string& getFile() const
    {
      return file;
    }

    /** only soft button: channel icon */
    void setFile(const std::string &file)
    {
      this->file = file;
    }


    /** only soft button: position on lcd, starting at 0. 6 devices per page, so 7 -> pos 0 on second page */
    int getPosition() const
    {
      return position;
    }

    /** only soft button: position on lcd, starting at 0. 6 devices per page, so 7 -> pos 0 on second page */
    void setPosition(int position)
    {
      this->position = position;
    }

  protected:
    ButtonType t;
    std::string name;
    std::string file;
    int position;
    std::string action;
};

}
}
}
