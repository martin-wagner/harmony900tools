// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QObject>

#include "lib/undo.h"
#include "data/data.h"

namespace document
{

/** one config file object
 *
 * contains data storage, importer, exporter
 */
class Config : public QObject
{
  Q_OBJECT
  public:
    /** create new, empty config */
    static Config create();
    /** import zipped config from remote / backup */
    static Config create(const std::vector<uint8_t> &zip);
    /** read config */
    static Config create(const QString &path);

    ~Config();

    /** check modified */
    bool isDirty();

    /** get current project file path */
    QString getPath();

    /** write project to disk */
    bool save();
    /** write copy to disk */
    bool saveAs(const QString &path);
    /** generate export */
    bool dumpZip(std::vector<uint8_t> &zip);

  signals:
    void deviceChanged(int index);
    void deviceAdded(int index);
    void deviceRemoved(int index);
    void activityChanged(int index);
    void activityAdded(int index);
    void activityRemoved(int index);
    void dirtyChanged(bool dirty);

  protected:
    Config();

  protected:
    data::Config configData;
    lib::UndoStack stack;

    bool dirty = false;
};



}
