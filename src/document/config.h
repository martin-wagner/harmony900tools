// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <memory>
#include <QObject>
#include <QTemporaryDir>

#include "context.h"
#include "lib/undo.h"
#include "lib/zip.h"
#include "data/data.h"
#include "data/catalogue.h"
#include "ui/logViewer.h"

namespace document
{

enum class Type {
    UNKNOWN,
    H900 = 900,
    //todo more?
};

/** one config file object
 *
 * contains data storage, importer, exporter
 */
class Config : public QObject
{
  Q_OBJECT
  public:
    /** new config. init = true -> create a new, empty config */
    Config(Context &ctx, bool init = false);

    /** create new, empty config */
    bool create();
    /** import zipped config from remote / backup */
    bool read(const std::vector<uint8_t> &zip, Type t);
    /** read config */
    bool read(const QString &path);
    /** unload config */
    bool reset();

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
    bool dumpZip(std::vector<uint8_t> &zip, Type t);

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

    void deviceChanged(uint32_t id);
    void deviceAdded(uint32_t id);
    void deviceAboutToBeRemoved(uint32_t id);
    void deviceRemoved(uint32_t id);
    void activityChanged(uint32_t id);
    void activityAdded(uint32_t id);
    void activityAboutToBeRemoved(uint32_t id);
    void activityRemoved(uint32_t id);
    void dirtyChanged(bool dirty);

  protected:
    std::unique_ptr<data::ConfigData> configData;
    lib::UndoStack &stack;
    data::CmdCatalogue *worker = nullptr;

    bool dirty = false;
    Type type = Type::UNKNOWN;

    static constexpr uint32_t UidStartValue = 10000000;

  protected:
      QString savePath;
      QString workPath;
      std::unique_ptr<QTemporaryDir> tempDir;

};



}
