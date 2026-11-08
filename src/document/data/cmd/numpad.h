// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "base.h"
#include "document/data/items/button.h"

namespace document
{
namespace data
{

class AddNumpadCommand: public BaseCommand
{
  Q_OBJECT
  public:
    AddNumpadCommand(ConfigData &c, uint32_t devicePos, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;
};

class AddDigitsCommand: public BaseCommand
{
  Q_OBJECT
  public:
    AddDigitsCommand(ConfigData &c, uint32_t devicePos, item::DigitSection s, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;
    item::DigitSection s;
};

class RemoveNumpadCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveNumpadCommand(ConfigData &c, uint32_t devicePos,  QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;
    item::Numpad pad;
};

class RemoveDigitsCommand: public BaseCommand
{
  Q_OBJECT
  public:
    RemoveDigitsCommand(ConfigData &c, uint32_t devicePos, item::DigitSection s, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;
    item::DigitSection s;
    item::Digits d;
    item::DeviceAction a;
};


class SetNumpadActionCommand: public BaseCommand
{
  Q_OBJECT
  public:
    SetNumpadActionCommand(ConfigData &c, uint32_t devicePos, item::DigitSection s, uint32_t digit, const item::DeviceAction &act, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    bool valid() const;

  protected:
    bool isValid = false;

    ConfigData &c;
    uint32_t devicePos;
    item::DigitSection s;
    uint32_t digit;
    item::DeviceAction act;
    item::DeviceAction oldAct;
};

}
}
