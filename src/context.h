// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QString>
#include "ui/settings.h"
#include "lib/users.h"
#include "lib/undo.h"

/**
 * @brief Central container for application-wide state and services.
 * Construct once in main/MainWindow, pass by reference to all classes that need it.
 */
class Context {
public:
    Context(Settings &s, lib::UserLevel &u, lib::UndoStack &undo) :
      settings(s), userLevel(u), undo(undo)
    {
    }

    Settings& getSettings() { return settings; }
    const lib::UserLevel& getUserLevel() { return userLevel; }
    lib::UndoStack &getUndoStack() { return undo; }

private:
    Settings &settings;
    lib::UserLevel &userLevel;
    lib::UndoStack &undo;
};
