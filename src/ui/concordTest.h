#pragma once

#include <QtWidgets>
#include <QString>

#include "context.h"
#include "logViewer.h"

namespace LibConcord
{
   class ConcordWrapper;
}

class ConcordTest: public QWidget
{
  Q_OBJECT

  public:
    explicit ConcordTest(Context &ctx, QWidget *parent = nullptr);
    ~ConcordTest();

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

  protected slots:
    void onOpenConnection();
    void onCloseConnection();
    void onGetInfo();
    void onGetTime();
    void onSetTime();
    void onReadConfig();
    void onWriteConfig();

    void onProgressUpdated(uint32_t stage, uint32_t count, uint32_t current,
        uint32_t total, uint32_t counterType, const uint32_t *stages);

    void onIpChanged();

  protected:
    void createWidgets();
    void createActions();

  private:
    Context &ctx;
    std::unique_ptr<LibConcord::ConcordWrapper> concord;
    bool connectionIsOpen = false;

  private:
    QString formatTime(bool fixMonth = false);

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

    //QLineEdit *editSetIpAddress;

    QTextBrowser *textBrowser = nullptr;
    QToolBar *toolBar = nullptr;
    QStatusBar *statusBar = nullptr;
    QAction *scrollLock = nullptr;
};
