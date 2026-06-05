// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QtWidgets>
#include <QString>
#include <mutex>

#include "bin/timing.h"
#include "context.h"
#include "ui/logViewer.h"

namespace LibConcord
{
   class ConcordWrapper;
}

class Concord: public QWidget
{
  Q_OBJECT

  public:
    explicit Concord(Context &ctx, QWidget *parent = nullptr);
    ~Concord();

    /** check state */
    bool isInitialised();
    /** check state */
    bool isBusy();

    /** get manufacturer string */
    QString mnf();
    /** get model string */
    QString model();
    /** get fw string */
    QString fwVersion();
    /** get hw string */
    QString hwVersion();

    /** take hex file, remove xml header, return data */
    bool stripHeader(const QString &file, std::vector<uint8_t> &data);

    /** all following methods emit "done()" when finished. some provide additional signals */

    /** open connection */
    bool connectRemote();

    /** close connection */
    void disconnectRemote();

    /** set current system time to remote */
    bool setTime();
    /** query current remote time. emits time() on success */
    bool readTime();

    /** starts learning single IR command. emits learnDone() on success */
    bool learnCommand();
    /** starts recording IR data for time_ms milliseconds. emits learnDone on success */
    bool learnStream(int time_ms = 1000);

    /** starts reading the user config from the remote */
    bool readUserConfig();
    /** write user config to file (needs to be read before) */
    int writeUserConfigFile(const QString &file, bool includeHeader = false);
    /** get user config data (needs to be read before) */
    std::vector<uint8_t> getUserConfig();

    /** starts writing user config file to the remote */
    int updateUserConfig(const QString &file);
    /** starts writing user config data to the remote */
    int updateUserConfigData(const std::vector<uint8_t> &data, bool containsHeader = false);

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

    /** progress -- text, step m of n. n < 1 -> unknown */
    void updateProgress(const QString text, int step, int of);
    /** remote has been unplugged */
    void disconnected(int time_s);
    /** command done */
    void done(bool success, const QString &msg);
    /** command getTime() done */
    void time(const QString time);
    /** remote waits for source remote button press
     * true -- remote is ready, false -- remote is done / window closed */
    void learnWindowIsOpen(bool waits);
    /** learn command is done */
    void learnDone(const binary::TimingStream &t, uint32_t carrier);
    /** reading is done */
    void readUserConfigDone(bool success);

  /* private */ signals:
    void doStart();
    void doSetTime();
    void doReadTime();
    void doLearnCommand();
    void doLearnStream(int time_ms);
    void doReadUserConfig();
    void doUpdateUserConfig();

  private slots:
    void onTimeout();

  protected:
    void createActions();
    void createWorkerActions();
    void cleanup();

  private:
    Context &ctx;
    std::shared_ptr<LibConcord::ConcordWrapper> concord;
    std::mutex lock;
    std::atomic<bool> connectionIsOpen = false;

  private:
    QThread workerThread;
    class Worker *worker; //owned by workerThread!
    bool startThread();
    void stopThread();
    QTimer threadSupervisorTimer;
    int counter = 0;
    const int COUNTER_THRESHOLD = 10; // n * timer reset value

};
