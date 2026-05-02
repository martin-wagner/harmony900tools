#include <memory>
//plain c functions -- never use outside cpp file (->global namespace)

#include "concordTest.h"
#include "wrappers/concordWrapper.h"

using namespace std;

ConcordTest::ConcordTest(Context &ctx, QWidget *parent) :
    QWidget(parent), ctx(ctx), concord(
        make_unique<LibConcord::ConcordWrapper>(this))
{
  createWidgets();
  createActions();
}

ConcordTest::~ConcordTest()
{
  cleanup();
}

void ConcordTest::onOpenConnection()
{
  if (connectionIsOpen) {
    onCloseConnection();
  }

  auto ret = concord->initConcord();
  if (ret != 0) {
    auto err = concord->errorToString(ret);
    emit writeLog(LogLevel::Error,
        tr("libconcord connection error: %1 (%2)").arg(err).arg(ret),
        ContentType::PlainText);
    labelConnection->setText(tr("error"));
    return;
  }
  connectionIsOpen = true;
  labelConnection->setText(tr("connected"));
  emit writeMsg(tr("Connection OK"));
}

void ConcordTest::onCloseConnection()
{
  cleanup();
  emit writeMsg(tr("Connection closed"));
  labelConnection->setText(tr("dsconnected"));
}

void ConcordTest::onGetInfo()
{
  if (!connectionIsOpen) {
    return;
  }

  auto ret = concord->getIdentity();
  if (ret != 0) {
    auto err = concord->errorToString(ret);
    emit writeLog(LogLevel::Error,
        tr("libconcord get info error: %1 (%2)").arg(err).arg(ret),
        ContentType::PlainText);
    return;
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
}

void ConcordTest::onGetTime()
{
  if (!connectionIsOpen) {
    return;
  }
  auto ret = concord->getTime();
  if (ret != 0) {
    auto err = concord->errorToString(ret);
    emit writeLog(LogLevel::Error,
        tr("libconcord get time error: %1 (%2)").arg(err).arg(ret),
        ContentType::PlainText);
    return;
  }
  emit writeMsg(tr("Harmony Time: %1").arg(formatTime(true)));
}

void ConcordTest::onSetTime()
{
  if (!connectionIsOpen) {
    return;
  }
  auto ret = concord->setTime();
  if (ret != 0) {
    auto err = concord->errorToString(ret);
    emit writeLog(LogLevel::Error,
        tr("libconcord set time error: %1 (%2)").arg(err).arg(ret),
        ContentType::PlainText);
    return;
  }
  emit writeMsg(tr("Set Harmony Time: %1").arg(formatTime()));
}

void ConcordTest::onReadConfig()
{
  uint8_t *config;
  uint32_t size = 0;

  if (!connectionIsOpen) {
    return;
  }

  //get identity
  auto ret = concord->getIdentity();
  if (ret != 0) {
    auto err = concord->errorToString(ret);
    emit writeLog(LogLevel::Error,
        tr("libconcord get info error: %1 (%2)").arg(err).arg(ret),
        ContentType::PlainText);
    return;
  }

  //read data
  ret = concord->readConfigFromRemote(&config, &size);
  if (ret != 0) {
    auto err = concord->errorToString(ret);
    emit writeLog(LogLevel::Error,
        tr("libconcord read config error: %1 (%2)").arg(err).arg(ret),
        ContentType::PlainText);
    return;
  }
  labelFileSize->setText(tr("%1 bytes").arg(size));

  QString filePathXml = QFileDialog::getSaveFileName(this,
      tr("Save XML + Zip Config"), QDir::homePath(),
      tr("hex Files (*.hex);;All Files (*)"));
  if (!filePathXml.isEmpty()) {
    //write xml + zip file
    ret = concord->writeConfigToFile(config, size,
        filePathXml.toStdString().c_str(), false);
    if (ret != 0) {
      auto err = concord->errorToString(ret);
      emit writeLog(LogLevel::Error,
          tr("libconcord write file error: %1 (%2)").arg(err).arg(ret),
          ContentType::PlainText);
      return;
    }
    emit writeLog(LogLevel::Debug,
        tr("libconcord written %1 bytes to %2").arg(size).arg(filePathXml),
        ContentType::PlainText);
  }

  QString filePathZip = QFileDialog::getSaveFileName(this,
      tr("Save Zip Config"), QDir::homePath(),
      tr("zip Files (*.zip);;All Files (*)"));
  if (!filePathZip.isEmpty()) {
    //write zip file
    ret = concord->writeConfigToFile(config, size,
        filePathZip.toStdString().c_str(), true);
    if (ret != 0) {
      auto err = concord->errorToString(ret);
      emit writeLog(LogLevel::Error,
          tr("libconcord write file error: %1 (%2)").arg(err).arg(ret),
          ContentType::PlainText);
      return;
    }
    emit writeLog(LogLevel::Debug,
        tr("libconcord written %1 bytes to %2").arg(size).arg(filePathZip),
        ContentType::PlainText);
  }

  //todo memleak on error / early return
  concord->freeBlob(config);
}

void ConcordTest::onWriteConfig()
{
  uint32_t size = 0;
  int type;

  if (!connectionIsOpen) {
    return;
  }

  QString filePath = QFileDialog::getOpenFileName(this, tr("Open File"),
      QDir::homePath(), tr("hex Files (*.hex);;All Files (*)"));
  if (filePath.isEmpty()) {
    return;
  }
  size = QFile(filePath).size();

  auto ret = concord->readAndParseFile(filePath.toStdString().c_str(), &type);
  if (ret != 0) {
    auto err = concord->errorToString(ret);
    emit writeLog(LogLevel::Error,
        tr("libconcord read file error: %1 (%2)").arg(err).arg(ret),
        ContentType::PlainText);
    return;
  }
  if (type != 2) { //todo LC_FILE_TYPE_CONFIGURATION fehlt im wrapper
    emit writeLog(LogLevel::Error,
        tr("libconcord file type not config: %1").arg(type),
        ContentType::PlainText);
    return;
  }

  //get identity
  ret = concord->getIdentity();
  if (ret != 0) {
    auto err = concord->errorToString(ret);
    emit writeLog(LogLevel::Error,
        tr("libconcord get info error: %1 (%2)").arg(err).arg(ret),
        ContentType::PlainText);
    return;
  }

  //write data
  ret = concord->updateConfiguration(false);
  if (ret != 0) {
    auto err = concord->errorToString(ret);
    emit writeLog(LogLevel::Error,
        tr("libconcord update config error: %1 (%2)").arg(err).arg(ret),
        ContentType::PlainText);
    return;
  }
  labelFileSize->setText(tr("%1 bytes").arg(size));
  emit writeLog(LogLevel::Debug,
      tr("libconcord sent %1 bytes to remote").arg(size),
      ContentType::PlainText);
}

void ConcordTest::onProgressUpdated(uint32_t stage, uint32_t count,
    uint32_t current, uint32_t total, uint32_t counterType,
    const uint32_t *stages)
{
  emit writeLog(LogLevel::Debug,
      tr("libconcord progress: stage: %1 (%2), count: %3, current: %4, "
          "total: %5, counterType: %6").arg(concord->stageToString(stage)).arg(
          stage).arg(count).arg(current).arg(total).arg(counterType),
      ContentType::PlainText);
}

void ConcordTest::onIpChanged()
{
  emit writeMsg("hello world!");
}

void ConcordTest::createWidgets()
{
  layout = new QGridLayout(this);

  auto label = new QLabel(tr("Test LibConcord"), this);
  layout->addWidget(label, 1, 0, 1, 3);

  buttonOpenConnection = new QPushButton(tr("Open connection"), this);
  layout->addWidget(buttonOpenConnection, 2, 0);
  buttonCloseConnection = new QPushButton(tr("Close connection"), this);
  layout->addWidget(buttonCloseConnection, 2, 1);
//  editSetIpAddress = new QLineEdit("169.254.1.2", this);
//  layout->addWidget(editSetIpAddress, 2, 1);
  labelConnection = new QLabel(tr("Result"), this);
  layout->addWidget(labelConnection, 2, 2);

  buttonGetInfo = new QPushButton(tr("Get remote info"), this);
  layout->addWidget(buttonGetInfo, 3, 0);
  buttonGetTime = new QPushButton(tr("Get time"), this);
  layout->addWidget(buttonGetTime, 3, 1);
  buttonSetTime = new QPushButton(tr("Set time"), this);
  layout->addWidget(buttonSetTime, 3, 2);

  buttonReadConfigFromRemote = new QPushButton(tr("Read Config"), this);
  layout->addWidget(buttonReadConfigFromRemote, 4, 0);
  buttonWriteConfigToRemote = new QPushButton(tr("Write Config"), this);
  layout->addWidget(buttonWriteConfigToRemote, 4, 1);
  labelFileSize = new QLabel(tr("File Size"), this);
  layout->addWidget(labelFileSize, 4, 2);

}

void ConcordTest::createActions()
{
  connect(buttonOpenConnection, &QPushButton::clicked, this,
      &ConcordTest::onOpenConnection);
  connect(buttonCloseConnection, &QPushButton::clicked, this,
      &ConcordTest::onCloseConnection);
  connect(buttonGetInfo, &QPushButton::clicked, this, &ConcordTest::onGetInfo);
  connect(buttonGetTime, &QPushButton::clicked, this, &ConcordTest::onGetTime);
  connect(buttonSetTime, &QPushButton::clicked, this, &ConcordTest::onSetTime);
  connect(buttonReadConfigFromRemote, &QPushButton::clicked, this,
      &ConcordTest::onReadConfig);
  connect(buttonWriteConfigToRemote, &QPushButton::clicked, this,
      &ConcordTest::onWriteConfig);

  connect(concord.get(), &LibConcord::ConcordWrapper::progressUpdated, this,
      &ConcordTest::onProgressUpdated);

//  connect(editSetIpAddress, &QLineEdit::textChanged, this,
//      &ConcordTest::onIpChanged);
}

QString ConcordTest::formatTime(bool fixMonth)
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

  return QString("%1.%2.%3 %4:%5:%6 %7").arg(day, 2, 10, QChar('0')).arg(month,
      2, 10, QChar('0')).arg(year).arg(hour, 2, 10, QChar('0')).arg(min, 2, 10,
      QChar('0')).arg(sec, 2, 10, QChar('0')).arg(tz);
}

void ConcordTest::cleanup()
{
  if (!connectionIsOpen) {
    return;
  }
  connectionIsOpen = false;
  concord->deinitConcord();
}
