// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QString>
#include "ui/settings.h"
#include "lib/users.h"
#include "lib/undo.h"

class Concord;
namespace document { class Config; }

/**
 * @brief Central container for application-wide state and services.
 * Construct once in main/MainWindow, pass by reference to all classes that need it.
 *
 * Constructor use:
 * - reference: always usable
 * - pointer: check befor use
 *
 * Runtime use:
 * - reference: doesn't change, can be duplicated for convenience
 * - pointer: can change, always use getter.
 */
class Context {
  friend class MainWindow;

  public:
    Context(Settings &s, lib::UserLevel &u, lib::UndoStack &undo) :
      settings_(s), userLevel_(u), undo_(undo)
    {
    }

    Settings& settings() { return settings_; }
    const lib::UserLevel& userLevel() { return userLevel_; }
    lib::UndoStack &undoStack() { return undo_; }
    const Concord* concord() const { return concord_; }
    document::Config* config() { return config_; }
    const document::Config* config() const { return config_; }

  protected:
    void setConcord(Concord *concord = nullptr) { concord_ = concord; }
    void setConfig(document::Config *config = nullptr) { config_ = config; }

  private:
    Settings &settings_;
    lib::UserLevel &userLevel_;
    lib::UndoStack &undo_;
    Concord *concord_ = nullptr;
    document::Config *config_ = nullptr;

};
