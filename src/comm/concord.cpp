// SPDX-License-Identifier: LGPL-2.1-or-later

#include "concord.h"

#include <memory>
//plain c functions -- never use outside cpp file (->global namespace)
#include "wrappers/concordWrapper.h"

using namespace std;

class Worker: public QObject
{
  Q_OBJECT
  public:
    Worker(shared_ptr<LibConcord::ConcordWrapper> concord, mutex &lock,
        QObject *parent = nullptr) :
        concord(concord), lock(lock)
    {
    }

    ~Worker()
    {
      cleanup();
    }

    bool isConnected()
    {
      return connectionIsReady;
    }

    bool isBusy()
    {
      if (lock.try_lock()) {
        lock.unlock();
        return false;
      }
      return true;
    }

    //use while mutex is locked!
    unique_ptr<vector<uint8_t>> getReadConfig()
    {
      return move(readConfig);
    }

  signals:
    void writeLog(LogLevel level, const QString &message,
        ContentType contentType);
    void writeMsg(const QString &message);
    void updateProgress(const QString text, int step, int of);
    void done(bool success, const QString &msg);
    void time(const QString time);
    void learnWindowIsOpen(bool waits);
    void learnDone(const binary::TimingStream &t, uint32_t carrier);
    void readUserConfigDone(bool success);
    void pingResponse();

  public slots:
    void run()
    {
      QEventLoop loop;

      if (concord == nullptr) {
        emit writeLog(LogLevel::Error, tr("libconcord nullptr"),
            ContentType::PlainText);
        return;
      }

      pingTimer = new QTimer(this); //here so it is part of the thread!
      pingTimer->setTimerType(Qt::VeryCoarseTimer);
      pingTimer->setSingleShot(true);
      connect(pingTimer, &QTimer::timeout, this, &Worker::onPingTimeout);

      createActions();

      auto ret = connectRemote();
      if (!ret) {
        return;
      }

      while (!QThread::currentThread()->isInterruptionRequested()) {
        loop.processEvents(QEventLoop::AllEvents);
      }
    }

    void setTime()
    {
      lock_guard<mutex> m(lock);

      auto ret = concord->setTime();
      if (ret != 0) {
        auto err = concord->errorToString(ret);
        emit writeLog(LogLevel::Error,
            tr("libconcord set time error: %1 (%2)").arg(err).arg(ret),
            ContentType::PlainText);
        emit done(false, QString(err));
        return;
      }
      emit done(true, tr("OK"));
    }

    void readTime()
    {
      lock_guard<mutex> m(lock);

      auto ret = concord->getTime();
      if (ret != 0) {
        auto err = concord->errorToString(ret);
        emit writeLog(LogLevel::Error,
            tr("libconcord get time error: %1 (%2)").arg(err).arg(ret),
            ContentType::PlainText);
        emit done(false, QString(err));
        return;
      }
      auto strTime = formatTime(true);
      emit writeLog(LogLevel::Debug, tr("Harmony Time: %1").arg(strTime),
          ContentType::PlainText);
      emit done(true, tr("OK"));
      emit time(strTime);
    }

    void learnCommand()
    {
      uint32_t carrier;
      uint32_t *data_ret;
      uint32_t data_len;

      lock_guard<mutex> m(lock);

      emit learnWindowIsOpen(true);
      concord->setLearningMode(LibConcord::LearnSingle, 0);
      auto ret = concord->learnFromRemote(&carrier, &data_ret, &data_len);
      emit learnWindowIsOpen(false);
      if (ret != 0) {
        auto err = concord->errorToString(ret);
        emit writeLog(LogLevel::Error,
            tr("libconcord learn error: %1 (%2)").arg(err).arg(ret),
            ContentType::PlainText);
        emit done(false, QString(err));
        return;
      }

      finaliseLearnedCommand(carrier, data_ret, data_len);
    }

    void learnStream(int time_ms)
    {
      uint32_t carrier;
      uint32_t *data_ret;
      uint32_t data_len;

      lock_guard<mutex> m(lock);

      emit learnWindowIsOpen(true);
      concord->setLearningMode(1, time_ms);
      auto ret = concord->learnFromRemote(&carrier, &data_ret, &data_len);
      emit learnWindowIsOpen(false);
      if (ret != 0) {
        auto err = concord->errorToString(ret);
        emit writeLog(LogLevel::Error,
            tr("libconcord stream error: %1 (%2)").arg(err).arg(ret),
            ContentType::PlainText);
        emit done(false, QString(err));
        return;
      }

      finaliseLearnedCommand(carrier, data_ret, data_len);
    }

    void readUserConfig()
    {
      uint8_t *config;
      uint32_t size = 0;

      lock_guard<mutex> m(lock);

      auto ret = concord->readConfigFromRemote(&config, &size);
      if (ret != 0) {
        auto err = concord->errorToString(ret);
        emit writeLog(LogLevel::Error,
            tr("libconcord read config error: %1 (%2)").arg(err).arg(ret),
            ContentType::PlainText);
        emit readUserConfigDone(false);
        emit done(false, QString(err));
        return;
      }
      if (config == nullptr) {
        emit writeLog(LogLevel::Error, tr("libconcord nullptr"),
            ContentType::PlainText);
        emit readUserConfigDone(false);
        emit done(false, tr("Config data nullptr"));
        return;
      }

      readConfig = make_unique<vector<uint8_t>>(config, config + size);
      concord->freeBlob(config);

      emit readUserConfigDone(true);
      emit done(true, tr("OK"));
    }

    void updateUserConfig()
    {
      lock_guard<mutex> m(lock);

      auto ret = concord->updateConfiguration(false);
      if (ret != 0) {
        auto err = concord->errorToString(ret);
        emit writeLog(LogLevel::Error,
            tr("libconcord update config error: %1 (%2)").arg(err).arg(ret),
            ContentType::PlainText);
        emit done(false, QString(err));
        return;
      }
      emit writeLog(LogLevel::Debug, tr("libconcord sent config to remote"),
          ContentType::PlainText);
      emit done(true, tr("OK"));
    }

  protected slots:
    void onPingTimeout()
    {
      int ret;

      if (isBusy()) {
        resetTimer();
        return;
      }
      // ping remote. if disconnected, this will block for infinte time. on broken pipe
      // will return error code

      {
        lock_guard<mutex> m(lock);
        ret = concord->getTime();
        resetTimer();
        if (ret == 0) {
          emit pingResponse();
        }
      }

      if (ret != 0) { //no useful error code
        emit writeLog(LogLevel::Error,
            tr("libconcord error, resetting connection"),
            ContentType::PlainText);
        cleanup();
        connectRemote();
      }
    }

    void onProgressUpdated(uint32_t stage, uint32_t count, uint32_t current,
        uint32_t total, uint32_t counterType, const uint32_t *stages)
    {
      emit writeLog(LogLevel::Debug,
          tr("libconcord progress: stage: %1 (%2), count: %3, current: %4, "
              "total: %5, counterType: %6").arg(concord->stageToString(stage)).arg(
              stage).arg(count).arg(current).arg(total).arg(counterType),
          ContentType::PlainText);

      auto str = tr("Step: %1").arg(concord->stageToString(stage));
      emit updateProgress(str, current, total);
    }

  protected:

    void resetTimer()
    {
      pingTimer->start(1000 + rand() % 100); //don't be too precise
    }

    void createActions()
    {
      connect(concord.get(), &LibConcord::ConcordWrapper::progressUpdated, this,
          &Worker::onProgressUpdated);
      connect(this, &Worker::done, [this](bool success) {
        if (!success) {
          return;
        }
        resetTimer();
        emit pingResponse();
      });
      connect(this, &Worker::updateProgress, [this]() {
        resetTimer();
        emit pingResponse();
      });
    }

    bool connectRemote()
    {
      lock_guard<mutex> m(lock);

      auto ret = concord->getIdentity();
      if (ret != 0) {
        auto err = concord->errorToString(ret);
        emit writeLog(LogLevel::Error,
            tr("libconcord get info error: %1 (%2)").arg(err).arg(ret),
            ContentType::PlainText);
        return false;
      }
      // @formatter:off
      emit writeLog(LogLevel::Debug, tr("libconcord OK"), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("mfg: %1").arg(concord->getMfg()), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("model: %1").arg(concord->getModel()), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("codename: %1").arg(concord->getCodename()), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("skin: %1").arg(concord->getSkin()), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("fw version: %1.%2").arg(concord->getFwVerMaj()).arg(concord->getFwVerMin()), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("fw type: %1").arg(concord->getFwType()), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("hw version: %1.%2.%3").arg(concord->getHwVerMaj()).arg(concord->getHwVerMin()).arg(concord->getHwVerMic()), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("flash size: %1").arg(concord->getFlashSize()), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("flash mfg: %1").arg(concord->getFlashMfg()), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("flash id: %1").arg(concord->getFlashId()), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("flash part num: %1").arg(concord->getFlashPartNum()), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("arch: %1").arg(concord->getArch()), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("proto: %1").arg(concord->getProto()), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("HID mfg: %1").arg(concord->getHidMfgStr()), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("HID product: %1").arg(concord->getHidProdStr()), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("HID IRL: %1").arg(concord->getHidIrl()), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("HID ORL: %1").arg(concord->getHidOrl()), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("HID FRL: %1").arg(concord->getHidFrl()), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("USB VID: 0x%1").arg(concord->getUsbVid(), 4, 16, QChar('0')), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("USB PID: 0x%1").arg(concord->getUsbPid(), 4, 16, QChar('0')), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("USB BCD: 0x%1").arg(concord->getUsbBcd(), 4, 16, QChar('0')), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("serial: %1").arg(concord->getSerial(false)), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("config bytes used: %1 / %2").arg(concord->getConfigBytesUsed()).arg(concord->getConfigBytesTotal()), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("config dump supported: %1").arg(concord->isConfigDumpSupported()), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("config update supported: %1").arg(concord->isConfigUpdateSupported()), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("firmware dump supported: %1").arg(concord->isFirmwareDumpSupported()), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("firmware update supported (direct): %1").arg(concord->isFirmwareUpdateSupported(true)), ContentType::PlainText);
      emit writeLog(LogLevel::Debug, tr("firmware update supported (indirect): %1").arg(concord->isFirmwareUpdateSupported(false)), ContentType::PlainText);
// @formatter:on

      emit writeMsg(tr("Remote is ready"));

      resetTimer();

      connectionIsReady = true;
      emit done(true, "OK");
      return true;
    }

    void cleanup()
    {
      if (pingTimer != nullptr) {
        pingTimer->stop();
      }
      connectionIsReady = false;
      readConfig = nullptr;
    }

    QString formatTime(bool fixMonth)
    {
      int sec = concord->getTimeSecond();
      int min = concord->getTimeMinute();
      int hour = concord->getTimeHour();
      int day = concord->getTimeDay();
      //int dow   = concord->getTimeDow();
      int month = concord->getTimeMonth();
      int year = concord->getTimeYear();
      const char *tz = concord->getTimeTimezone();
      if (fixMonth) {
        month = month + 1;
      }

      return QString("%1.%2.%3 %4:%5:%6 %7").arg(day, 2, 10, QChar('0')).arg(
          month, 2, 10, QChar('0')).arg(year).arg(hour, 2, 10, QChar('0')).arg(
          min, 2, 10, QChar('0')).arg(sec, 2, 10, QChar('0')).arg(tz);
    }

    void finaliseLearnedCommand(uint32_t carrier, uint32_t *data_ret,
        uint32_t data_len)
    {
      std::vector<uint16_t> data;

      if (data_ret == nullptr) {
        emit writeLog(LogLevel::Error, tr("libconcord nullptr"),
            ContentType::PlainText);
        emit done(false, tr("IR data nullptr"));
        return;
      }

      emit writeLog(LogLevel::Debug,
          tr("libconcord received %1 ir words").arg(data_len),
          ContentType::PlainText);

      for (int i = 0; i < data_len; ++i) {
        data.push_back(data_ret[i]);
      }
      concord->freeIrSignal(data_ret);

      emit writeLog(LogLevel::Debug,
          tr("libconcord IR Carrier %1kHz").arg((double) carrier / 1000.0),
          ContentType::PlainText);

      auto stream = binary::TimingStream::fromMarkPause(data);
      auto visual = stream.convertAsciiPlot(250);

      emit writeLog(LogLevel::Debug,
          tr("libconcord IR Data: %1").arg(QString::fromStdString(visual)),
          ContentType::PlainText);

      emit done(true, tr("OK"));
      emit learnDone(stream, carrier);
    }

    shared_ptr<LibConcord::ConcordWrapper> concord;
    mutex &lock;

    unique_ptr<vector<uint8_t>> readConfig;
    atomic<bool> connectionIsReady = false;

    QTimer *pingTimer = nullptr;
};

Concord::Concord(Context &ctx, QWidget *parent) :
    QWidget(parent), ctx(ctx)
{
  createActions();
}

Concord::~Concord()
{
  cleanup();
}

bool Concord::isInitialised()
{
  return connectionIsOpen;
}

bool Concord::isBusy()
{
  if (isInitialised()) {
    return worker->isBusy();
  }
  return false;
}

QString Concord::mnf()
{
  if (!isInitialised()) {
    return "";
  }

  lock_guard<mutex> m(lock);

  return concord->getMfg();
}

QString Concord::model()
{
  if (!isInitialised()) {
    return "";
  }

  lock_guard<mutex> m(lock);

  return concord->getModel();
}

QString Concord::fwVersion()
{
  if (!isInitialised()) {
    return "";
  }

  lock_guard<mutex> m(lock);

  return QString::number(concord->getFwVerMaj()) + "."
      + QString::number(concord->getFwVerMin());
}

QString Concord::hwVersion()
{
  if (!isInitialised()) {
    return "";
  }

  lock_guard<mutex> m(lock);

  return QString::number(concord->getHwVerMaj()) + "."
      + QString::number(concord->getHwVerMin()) + "."
      + QString::number(concord->getHwVerMic());
}

bool Concord::connectRemote()
{
  if (isInitialised()) {
    disconnectRemote();
  }

  concord = make_unique<LibConcord::ConcordWrapper>(this);
  auto ret = concord->initConcord();
  if (ret != 0) {
    auto err = concord->errorToString(ret);
    emit writeLog(LogLevel::Error,
        tr("libconcord connection error: %1 (%2)").arg(err).arg(ret),
        ContentType::PlainText);
    return false;
  }
  emit writeMsg(tr("Init OK"));

  ret = startThread();
  if (!ret) {
    emit writeLog(LogLevel::Error, tr("concord start thread failed"),
        ContentType::PlainText);
    return false;
  }
  connectionIsOpen = true;
  return true;
}

void Concord::disconnectRemote()
{
  cleanup();
  emit writeMsg(tr("Connection closed"));
}

bool Concord::setTime()
{
  if (!isInitialised()) {
    return false;
  }

  emit doSetTime();
  return true;
}

bool Concord::readTime()
{
  if (!isInitialised()) {
    return false;
  }

  emit doReadTime();
  return true;
}

bool Concord::learnCommand()
{
  if (!isInitialised()) {
    return false;
  }

  emit doLearnCommand();
  return true;
}

bool Concord::learnStream(int time_ms)
{
  if (!isInitialised()) {
    return false;
  }

  emit doLearnStream(time_ms);
  return true;
}

bool Concord::readUserConfig()
{
  if (!isInitialised()) {
    return false;
  }

  emit doReadUserConfig();
  return true;
}

int Concord::writeUserConfigFile(const QString &file, bool includeHeader)
{
  if (!isInitialised()) {
    return -1;
  }

  lock_guard<mutex> m(lock);

  auto data = worker->getReadConfig();
  if (data == nullptr) {
    return -2;
  }

  auto ret = concord->writeConfigToFile(data->data(), data->size(),
      file.toStdString().c_str(), !includeHeader);
  if (ret != 0) {
    auto err = concord->errorToString(ret);
    emit writeLog(LogLevel::Error,
        tr("libconcord write file error: %1 (%2)").arg(err).arg(ret),
        ContentType::PlainText);
    return -3;
  }
  emit writeLog(LogLevel::Debug, tr("libconcord written file %1").arg(file),
      ContentType::PlainText);

  return 0;
}

std::vector<uint8_t> Concord::getUserConfig()
{
  if (!isInitialised()) {
    return std::vector<uint8_t>();
  }

  lock_guard<mutex> m(lock);

  auto data = worker->getReadConfig();
  if (data == nullptr) {
    return std::vector<uint8_t>();
  }
  return (*data);
}

int Concord::updateUserConfig(const QString &file)
{
  int type;

  if (!isInitialised()) {
    return -1;
  }

  lock_guard<mutex> m(lock);

  auto ret = concord->readAndParseFile(file.toStdString().c_str(), &type);
  if (ret != 0) {
    auto err = concord->errorToString(ret);
    emit writeLog(LogLevel::Error,
        tr("libconcord read file error: %1 (%2)").arg(err).arg(ret),
        ContentType::PlainText);
    return -2;
  }
  if (type != LibConcord::FileTypeConfiguration) {
    emit writeLog(LogLevel::Error,
        tr("libconcord file type not config: %1").arg(type),
        ContentType::PlainText);
    return -3;
  }

  emit doUpdateUserConfig();
  return 0;
}

int Concord::updateUserConfigData(const std::vector<uint8_t> &data,
    bool containsHeader)
{
  int type;

  if (!isInitialised()) {
    return -1;
  }

  QTemporaryFile file;
  if (!file.open()) {
    emit writeLog(LogLevel::Error,
        tr("concord temp file creation error: %1").arg(file.errorString()),
        ContentType::PlainText);
    return -2;
  }

  lock_guard<mutex> m(lock);

  //write temp file...
  auto ret = concord->writeConfigToFile(data.data(), data.size(),
      file.fileName().toStdString().c_str(), containsHeader);
  if (ret != 0) {
    auto err = concord->errorToString(ret);
    emit writeLog(LogLevel::Error,
        tr("libconcord write file error: %1 (%2)").arg(err).arg(ret),
        ContentType::PlainText);
    return -3;
  }

  //...just to read it here again.
  ret = concord->readAndParseFile(file.fileName().toStdString().c_str(), &type);
  if (ret != 0) {
    auto err = concord->errorToString(ret);
    emit writeLog(LogLevel::Error,
        tr("libconcord read file error: %1 (%2)").arg(err).arg(ret),
        ContentType::PlainText);
    return -4;
  }
  if (type != LibConcord::FileTypeConfiguration) {
    emit writeLog(LogLevel::Error,
        tr("libconcord file type not config: %1").arg(type),
        ContentType::PlainText);
    return -5;
  }

  emit doUpdateUserConfig();
  return 0;
}

void Concord::onTimeout()
{
  if (!isInitialised()) {
    threadSupervisorTimer.stop();
    counter = 0;
    return;
  }

  counter++;

  if (counter > 2) {
    emit writeLog(LogLevel::Warning,
        tr("concord pings missed: %1").arg(counter), ContentType::PlainText);
  }

  if (counter > COUNTER_THRESHOLD) {
    emit disconnected(counter);
  }
}

void Concord::createActions()
{
}

void Concord::createWorkerActions()
{
  connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
  connect(this, &Concord::doStart, worker, &Worker::run);

  connect(this, &Concord::doSetTime, worker, &Worker::setTime);
  connect(this, &Concord::doReadTime, worker, &Worker::readTime);
  connect(this, &Concord::doLearnCommand, worker, &Worker::learnCommand);
  connect(this, &Concord::doLearnStream, worker, &Worker::learnStream);
  connect(this, &Concord::doReadUserConfig, worker, &Worker::readUserConfig);
  connect(this, &Concord::doUpdateUserConfig, worker,
      &Worker::updateUserConfig);

  connect(worker, &Worker::writeLog, this, &Concord::writeLog);
  connect(worker, &Worker::writeMsg, this, &Concord::writeMsg);

  connect(worker, &Worker::updateProgress, this, &Concord::updateProgress);
  connect(worker, &Worker::done, this, &Concord::done);
  connect(worker, &Worker::time, this, &Concord::time);
  connect(worker, &Worker::learnWindowIsOpen, this,
      &Concord::learnWindowIsOpen);
  connect(worker, &Worker::learnDone, this, &Concord::learnDone);
  connect(worker, &Worker::readUserConfigDone, this,
      &Concord::readUserConfigDone);
  connect(worker, &Worker::pingResponse, [this]() {counter = 0;});

  connect(&threadSupervisorTimer, &QTimer::timeout, this, &Concord::onTimeout);
}

void Concord::cleanup()
{
  if (!isInitialised()) {
    return;
  }
  connectionIsOpen = false;
  stopThread();
  concord->deinitConcord();
  concord = nullptr;
}

bool Concord::startThread()
{
  worker = new Worker(concord, lock, this);
  worker->moveToThread(&workerThread);
  createWorkerActions();
  workerThread.start();
  emit doStart();

  counter = 0;
  QTimer::singleShot(10000, this, [this]() {
    threadSupervisorTimer.start(1000);
  });
  return true;
}

void Concord::stopThread()
{
  threadSupervisorTimer.stop();
  disconnect(&threadSupervisorTimer, &QTimer::timeout, this,
      &Concord::onTimeout);

  workerThread.requestInterruption();
  workerThread.quit();
  workerThread.wait(5000);
  worker = nullptr;
}

#include "concord.moc"
