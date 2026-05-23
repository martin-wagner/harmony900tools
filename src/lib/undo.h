// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QObject>
#include <QUndoCommand>
#include <QUndoStack>
#include <QString>

namespace lib
{

/**
 * @brief Wrapper around QUndoStack that suppresses empty macros.
 *
 * Defers all beginMacro() calls until the first command is pushed,
 * preserving the full nesting order. Empty macro sequences are discarded.
 *
 * When macrosDisabled is true, macros are not used at all. Instead, a no-op
 * stamp command with the macro name is pushed to the stack for traceability.
 */
class UndoStack: public QObject
{
  Q_OBJECT

  public:
    explicit UndoStack(QObject *parent = nullptr) :
        QObject(parent)
    {
    }
    ~UndoStack();

    void beginMacro(const QString &text)
    {
      if (macrosDisabled) {
        stack.push(new MacroStampCommand("[begin] " + text));
        return;
      }
      pendingMacros.append(text);
    }

    void endMacro()
    {
      if (macrosDisabled) {
        // The stamp for endMacro is pushed only if a matching begin was not pending.
        // Since we never add to pendingMacros when disabled, always stamp here.
        stack.push(new MacroStampCommand("[end]"));
        return;
      }
      if (!pendingMacros.isEmpty()) {
        // Paired with a pending beginMacro that was never flushed — discard it.
        pendingMacros.removeLast();
        return;
      }
      stack.endMacro();
    }

    void push(QUndoCommand *command)
    {
      if (!macrosDisabled) {
        for (const QString &macroText : pendingMacros) {
          stack.beginMacro(macroText);
        }
        pendingMacros.clear();
      }
      stack.push(command);
    }

    void undo()
    {
      stack.undo();
    }
    void redo()
    {
      stack.redo();
    }
    bool canUndo() const
    {
      return stack.canUndo();
    }
    bool canRedo() const
    {
      return stack.canRedo();
    }
    void clear()
    {
      stack.clear();
    }
    void setClean()
    {
      stack.setClean();
    }
    bool isClean() const
    {
      return stack.isClean();
    }

    QUndoStack* getStack()
    {
      return &stack;
    }

    bool getMacrosDisabled() const
    {
      return macrosDisabled;
    }
    void setMacrosDisabled(bool disabled)
    {
      macrosDisabled = disabled;
    }

  private:
    /**
     * @brief No-op command used as a debug stamp when macros are disabled.
     */
    class MacroStampCommand: public QUndoCommand
    {
      public:
        explicit MacroStampCommand(const QString &label) :
            QUndoCommand(label)
        {
        }
        void undo() override
        {
        }
        void redo() override
        {
        }
    };

    QUndoStack stack;
    QList<QString> pendingMacros;
    bool macrosDisabled = false;
};

}
