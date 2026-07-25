// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QMainWindow>

#include <DockManager.h>

#include "ui/logViewer.h"
#include "ui/settings.h"
#include "ui/concordConnection.h"
#include "ui/deviceEditor.h"
#include "ui/activityEditor.h"
#include "lib/users.h"
#include "lib/undo.h"
#include "context.h"
#include "document/config.h"
#include "models/deviceListModel.h"
#include "models/activityListModel.h"

class QAction;
class QMenu;
class QPlainTextEdit;
class QSessionManager;
class Concord;

class MainWindow: public QMainWindow
{
  Q_OBJECT

  protected:
    int logLevel = -1;

  public:
    MainWindow(bool haveLogLevel, int logLevel);

    void loadFile(const QString &fileName);

  protected:
    void closeEvent(QCloseEvent *event) override;

  public slots:
    void onRequestQuit();

  private slots:
    void newFile();
    void open();
    bool save();
    bool saveAs();
    void import();
    void about();
    void documentWasModified();
    void commitData(QSessionManager&);
    void showSettings();
    void applySettings();
    void resetSettings();
    //ads view stuff
    void onLockUI(bool locked);
    void onSaveView();
    void onLoadView();
    void onLoadDefaultView();
    void onDeleteView();
    void onCopyViewToClipboard();
    void onDoImport(const std::vector<uint8_t> &data);

  private:
    void createStatusBar();
    void createLog();
    void createData();
    void createAds();
    void createWidgets();
    void createActions();
    void readSettings();
    void writeSettings();
    bool maybeSave();
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    QString infoText();
    QString strippedName(const QString &fullFileName);
    void updateModelView();
    void setTitle(const QString &append = "");
    bool importData(const std::vector<uint8_t> &data);

  private:
    ads::CDockManager* dockManager;
    QMenu *dockMenu;
    QAction *lockAction;

  private:
    LogViewer *log;
    Settings *settings;
    lib::UserLevel *user;
    std::unique_ptr<Context> ctx;

    lib::UndoStack undo;

    Concord *concord;
    document::Config *config;
    QString curFile;

    editors::DeviceEditor *deviceEditor = nullptr;
    editors::ActivityEditor *activityEditor = nullptr;
    models::DeviceModel *deviceModel = nullptr;
    models::ActivityModel *activityModel = nullptr;

    ConcordConnection *concordConnection;
};

