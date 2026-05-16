// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Unit tests for UndoStack — deferred macro wrapper around QUndoStack.
 * Skips pass-through methods (undo, redo, canUndo, canRedo, clear, setClean, isClean).
 */
#include <gtest/gtest.h>
#include <QUndoCommand>
#include "lib/undo.h"

using namespace lib;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

class DummyCommand: public QUndoCommand
{
  public:
    explicit DummyCommand(const QString &text = "cmd") :
        QUndoCommand(text)
    {
    }
    void redo() override
    {
    }
    void undo() override
    {
    }
};

static int macroCount(QUndoStack *stack)
{
  // Each top-level entry on the stack is either a plain command or a macro.
  return stack->count();
}

// ---------------------------------------------------------------------------
// SingleMacro — basic begin/push/end
// ---------------------------------------------------------------------------

TEST(UndoStackSingleMacro, PushInsideMacroCreatesSingleEntry)
{
  UndoStack s;
  s.beginMacro("m");
  s.push(new DummyCommand());
  s.endMacro();

  EXPECT_EQ(macroCount(s.getStack()), 1);
}

TEST(UndoStackSingleMacro, EmptyMacroIsDiscarded)
{
  UndoStack s;
  s.beginMacro("empty");
  s.endMacro();

  EXPECT_EQ(macroCount(s.getStack()), 0);
}

TEST(UndoStackSingleMacro, CommandWithoutMacroIsAddedDirectly)
{
  UndoStack s;
  s.push(new DummyCommand());

  EXPECT_EQ(macroCount(s.getStack()), 1);
}

// ---------------------------------------------------------------------------
// NestedMacrosAllUsed — open N macros, push, close all
// ---------------------------------------------------------------------------

TEST(UndoStackNestedMacrosAllUsed, ThreeLevelsAllClosed)
{
  UndoStack s;
  s.beginMacro("outer");
  s.beginMacro("middle");
  s.beginMacro("inner");
  s.push(new DummyCommand());
  s.endMacro(); // inner
  s.endMacro();// middle
  s.endMacro();// outer

  // All three macros were flushed and closed — one top-level entry.
  EXPECT_EQ(macroCount(s.getStack()), 1);
}

TEST(UndoStackNestedMacrosAllUsed, CommandPushedAfterAllMacrosOpened)
{
  UndoStack s;
  s.beginMacro("a");
  s.beginMacro("b");
  s.push(new DummyCommand());
  s.endMacro();
  s.endMacro();

  EXPECT_EQ(macroCount(s.getStack()), 1);
}

// ---------------------------------------------------------------------------
// NestedMacrosAllEmpty — open N macros, close all without pushing
// ---------------------------------------------------------------------------

TEST(UndoStackNestedMacrosAllEmpty, ThreeEmptyLevelsDiscarded)
{
  UndoStack s;
  s.beginMacro("a");
  s.beginMacro("b");
  s.beginMacro("c");
  s.endMacro();
  s.endMacro();
  s.endMacro();

  EXPECT_EQ(macroCount(s.getStack()), 0);
}

// ---------------------------------------------------------------------------
// NestedMacrosInnerUnused — close the innermost unused, then push
// ---------------------------------------------------------------------------

TEST(UndoStackNestedMacrosInnerUnused, InnerClosedEmptyThenCommandInOuter)
{
  UndoStack s;
  s.beginMacro("outer");
  s.beginMacro("inner-unused");
  s.endMacro(); // inner discarded — never flushed
  s.push(new DummyCommand());
  s.endMacro();// outer

  // Only "outer" macro should appear.
  EXPECT_EQ(macroCount(s.getStack()), 1);
}

// ---------------------------------------------------------------------------
// NestedMacrosOnlyOuterRemains — close all but first, then push
// ---------------------------------------------------------------------------

TEST(UndoStackNestedMacrosOnlyOuterRemains, MiddleAndInnerClosedBeforePush)
{
  UndoStack s;
  s.beginMacro("outer");
  s.beginMacro("middle");
  s.beginMacro("inner");
  s.endMacro(); // inner  — still pending, discarded
  s.endMacro();// middle — still pending, discarded
  s.push(new DummyCommand());
  s.endMacro();// outer

  EXPECT_EQ(macroCount(s.getStack()), 1);
}

// ---------------------------------------------------------------------------
// MultipleSeparateMacros — sequential macros on the same stack
// ---------------------------------------------------------------------------

TEST(UndoStackMultipleSeparateMacros, TwoMacrosProduceTwoEntries)
{
  UndoStack s;

  s.beginMacro("first");
  s.push(new DummyCommand());
  s.endMacro();

  s.beginMacro("second");
  s.push(new DummyCommand());
  s.endMacro();

  EXPECT_EQ(macroCount(s.getStack()), 2);
}

TEST(UndoStackMultipleSeparateMacros, EmptyThenUsedProducesOneEntry)
{
  UndoStack s;

  s.beginMacro("empty");
  s.endMacro();

  s.beginMacro("used");
  s.push(new DummyCommand());
  s.endMacro();

  EXPECT_EQ(macroCount(s.getStack()), 1);
}

TEST(UndoStackMultipleSeparateMacros, UsedThenEmptyProducesOneEntry)
{
  UndoStack s;

  s.beginMacro("used");
  s.push(new DummyCommand());
  s.endMacro();

  s.beginMacro("empty");
  s.endMacro();

  EXPECT_EQ(macroCount(s.getStack()), 1);
}

// ---------------------------------------------------------------------------
// MultiplePushesInMacro — more than one command inside a macro
// ---------------------------------------------------------------------------

TEST(UndoStackMultiplePushes, TwoCommandsInMacroStillOneEntry)
{
  UndoStack s;
  s.beginMacro("m");
  s.push(new DummyCommand());
  s.push(new DummyCommand());
  s.endMacro();

  EXPECT_EQ(macroCount(s.getStack()), 1);
}

TEST(UndoStackMultiplePushes, PushBeforeAndAfterMacro)
{
  UndoStack s;
  s.push(new DummyCommand()); // plain

  s.beginMacro("m");
  s.push(new DummyCommand());
  s.endMacro();

  s.push(new DummyCommand());// plain

  EXPECT_EQ(macroCount(s.getStack()), 3);
}
