
#pragma once

#include <QMainWindow>

#include <DockManager.h>

#include "ui/logViewer.h"
#include "ui/settings.h"

class QAction;
class QMenu;
class QPlainTextEdit;
class QSessionManager;

class MainWindow: public QMainWindow
{
  Q_OBJECT

  protected:
    int logLevel;

  public:
    MainWindow(int logLevel);

    void loadFile(const QString &fileName);

  protected:
    void closeEvent(QCloseEvent *event) override;

  private slots:
    void newFile();
    void open();
    bool save();
    bool saveAs();
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

  private:
    void createStatusBar();
    void createAds();
    void createWidgets();
    void createActions();
    void readSettings();
    void writeSettings();
    bool maybeSave();
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);

  private:
    ads::CDockManager* dockManager;
    QMenu *dockMenu;
    QAction *lockAction;

    QByteArray dockDefault = {"<?xml version=\"1.0\" encoding=\"UTF-8\"?><QtAdvancedDockingSystem Version=\"1\" UserVersion=\"0\" Containers=\"1\"><Container Floating=\"0\"><Splitter Orientation=\"|\" Count=\"2\"><Area Tabs=\"1\" Current=\"Left Panel\"><Widget Name=\"Left Panel\" Closed=\"0\"/></Area><Area Tabs=\"1\" Current=\"Right Panel\"><Widget Name=\"Right Panel\" Closed=\"0\"/></Area><Sizes>715 524 </Sizes></Splitter></Container></QtAdvancedDockingSystem>"};

  private:
    LogViewer *log;
    Settings *settings;

    QPlainTextEdit *textEdit;
    QString curFile;
};

