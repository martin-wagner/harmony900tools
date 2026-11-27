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

    enum class LearnedCommandMode {
        IR_STREAM,
        IR_FRAME
    };

    /** those two connect the same actions. they work together with "enableLearnMode()" */
    void addToToolbar(QToolBar *bar);
    void addToMenu(QMenu *menu);
    /** those allow >>one<< external consumer to use the learn mode.
     *  result is reported by externalLearnSucceeded() / externalLearnFailed() */
    QAction *getLearnSingleAction() { return actionExternalLearnSingle; };
    QAction *getLearnStreamActiion() { return actionExternalLearnStream; };

    bool isReady() { return concord.isInitialised(); };

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

    void ready(bool state);
    void doImport(const std::vector<uint8_t> &data);
    void requestQuit();

    /** result of the toolbar/menu learn actions */
    void learnedCommand(LearnedCommandMode m, const binary::TimingStream &t, uint32_t carrier);

    /** learning started by one of the external learn actions succeeded */
    void externalLearnSucceeded(LearnedCommandMode m, const binary::TimingStream &t, uint32_t carrier);
    /** learning started by one of the external learn actions failed (timeout, error, no data) */
    void externalLearnFailed(LearnedCommandMode m, const QString &reason);

  public slots:
    void enableLearnMode(bool start);

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
    void onLearnExternalSingle();
    void onLearnExternalStream();

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

    bool learningEnabled = false;
    bool dataMode = false;
    bool busy = false;
    LearnedCommandMode learnMode = LearnedCommandMode::IR_STREAM;

    /** who started the learning that is currently running */
    enum class LearnSource {
        NONE,
        BUILTIN,
        EXTERNAL
    };
    LearnSource learnSource = LearnSource::NONE;

  private:
    void updateActionStates(bool setToBusy = false);
    void startLearn(LearnedCommandMode mode, LearnSource source);
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
    QAction *actionExternalLearnSingle;
    QAction *actionExternalLearnStream;
    QWidgetAction *actionProgress;

    QProgressBar *progressBar;

    QMessageBox* disconnectMsg = nullptr;
    QMessageBox* waitMsg = nullptr;
};
