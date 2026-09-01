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
    inline static const QString defaultFilePostfix = "hzip";
    inline static const QString resourceH900PlatformConfig = ":/res/h900platform";

  public:
    /** new config. init = true -> create a new, empty config */
    Config(Context &ctx, bool init = false, QObject *parent = nullptr);

    /** create new, empty config */
    bool create();
    /** import zipped config from remote / backup */
    bool read(const std::vector<uint8_t> &zip, Type t);
    /** read config */
    bool read(const QString &file);
    /** unload config */
    bool reset();

    ~Config();

    /** check modified */
    bool isDirty();

    /** write copy to disk */
    bool saveAs(const QString &file);
    /** generate export */
    bool dumpZip(std::vector<uint8_t> &zip, Type t) const;

  public:
    //model access
    /** read data */
    const data::ConfigData &data() const;
    /** modify data */
    data::CmdCatalogue &modify();
    /** start undo macro */
    void beginMacro(const QString &text);
    /** end undo macro */
    void endMacro();

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType) const;
    void writeMsg(const QString &message) const;

    //model observers
    void itemChanged(data::Item item, uint32_t pos);
    void itemAboutToBeAdded(data::Item item, uint32_t pos);
    void itemAdded(data::Item item, uint32_t pos);
    void itemAboutToBeRemoved(data::Item item, uint32_t pos);
    void itemRemoved(data::Item item, uint32_t pos);
    void dirtyChanged(bool dirty);

  protected:
    std::unique_ptr<data::ConfigData> configData;
    lib::UndoStack &stack;
    Settings &settings;
    data::CmdCatalogue *worker = nullptr;

    bool dirty = false;

    static constexpr uint32_t UidStartValue = 10000000;

  protected:
      QString workPath;
      QString importPath;
      QString exportPath;
      std::unique_ptr<QTemporaryDir> tempDir;

  private:
      void copyIcons() const;

};



}
