#pragma once

#include <QtWidgets>
#include <QString>

#include "context.h"
#include "comm/concord.h"
#include "logViewer.h"

class ConcordConnection: public QWidget
{
  Q_OBJECT

  public:
    explicit ConcordConnection(Context &ctx, Concord &concord, QWidget *parent = nullptr);
    ~ConcordConnection();

    void addToToolbar(QToolBar *bar);
    void addToMenu(QMenu *menu);

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

    void doImport(const std::vector<uint8_t> &data);
    void requestQuit();

  protected slots:
    void onOpenConnection();
    void onCloseConnection();
    void onGetInfo();
    void onGetTime();
    void onSetTime();
    void onReadConfig();
    void onWriteConfig();
    void onBackupConfig();
    void onBackupConfigRestore();
    void onLearnIrSingle();
    void onLearnIrStream();

    void onUpdateProgress(const QString text, int step, int of);
    void onDisconnected(int time_s);
    void onDone(bool success, const QString &msg);
    void onTime(const QString time);
    void onLearnWindowIsOpen(bool waits);
    void onLearnDone(const binary::TimingStream &t, uint32_t carrier);
    void onBackupConfigDone(bool success);

  protected:
    void createSettings();
    void createWidgets();
    void createActions();

  private:
    Context &ctx;
    Concord &concord;

    bool dataMode = false;

  private:
    void updateActionStates(bool setToBusy = false);
    void cleanup();

  private:
    QGridLayout *layout;

    QPushButton *buttonOpenConnection;
    QPushButton *buttonCloseConnection;
    QLabel *labelConnection;

    QPushButton *buttonGetInfo;
    QPushButton *buttonGetTime;
    QPushButton *buttonSetTime;

    QPushButton *buttonReadConfigFromRemote;
    QPushButton *buttonWriteConfigToRemote;
    QLabel *labelFileSize;

    QPushButton *buttonLearnIrSingle;
    QPushButton *buttonLearnIrStream;
    QLabel *labelIrData;

    QAction *actionConnect = nullptr;
    QAction *actionDisconnect;
    QAction *actionGetInfo;
    QAction *actionSetTime;
    QAction *actionReadConfig;
    QAction *actionBackupConfig;
    QAction *actionBackupConfigRestore;
    QAction *actionWriteConfig;
    QAction *actionLearnIrSingle;
    QAction *actionLearnIrStream;
    QWidgetAction *actionProgress;

    QProgressBar *progressBar;

    QMessageBox* disconnectMsg = nullptr;
    QMessageBox* waitMsg = nullptr;
};
