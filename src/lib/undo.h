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
      pendingMacros.append(text);
    }

    void endMacro()
    {
      if (!pendingMacros.isEmpty()) {
        // Paired with a pending beginMacro that was never flushed — discard it.
        pendingMacros.removeLast();
        return;
      }
      stack.endMacro();
    }

    void push(QUndoCommand *command)
    {
      for (const QString &macroText : pendingMacros) {
        stack.beginMacro(macroText);
      }
      pendingMacros.clear();
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

  private:
    QUndoStack stack;
    QList<QString> pendingMacros;
};

}
