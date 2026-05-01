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
  onCloseConnection();
}

void ConcordTest::onOpenConnection()
{
  auto ret = concord->initConcord();
  if (ret != 0) {
    auto err = QString::fromUtf8(concord->getRawErrorString(ret));
    emit writeLog(LogLevel::Error,
        tr("libconcord connection error: %1 (%2)").arg(err).arg(ret),
        ContentType::PlainText);
    labelConnection->setText(tr("error"));
    return;
  }
  labelConnection->setText(tr("connected"));
  connectionIsOpen = true;
  emit writeMsg(tr("Connection OK"));
}

void ConcordTest::onCloseConnection()
{
  if (!connectionIsOpen) {
    return;
  }
  concord->deinitConcord();
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
    auto err = QString::fromUtf8(concord->getRawErrorString(ret));
    emit writeLog(LogLevel::Error,
        tr("libconcord get info error: %1 (%2)").arg(err).arg(ret),
        ContentType::PlainText);
    return;
  }
}

void ConcordTest::onGetTime()
{
  if (!connectionIsOpen) {
    return;
  }
  auto ret = concord->getTime();
  if (ret != 0) {
    auto err = QString::fromUtf8(concord->getRawErrorString(ret));
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
    auto err = QString::fromUtf8(concord->getRawErrorString(ret));
    emit writeLog(LogLevel::Error,
        tr("libconcord set time error: %1 (%2)").arg(err).arg(ret),
        ContentType::PlainText);
    return;
  }
  emit writeMsg(tr("Set Harmony Time: %1").arg(formatTime()));
}

void ConcordTest::onProgressUpdated(uint32_t stage, uint32_t count,
    uint32_t current, uint32_t total, uint32_t counterType,
    const uint32_t *stages)
{
  emit writeLog(LogLevel::Debug,
      tr("libconcord progress: stage: %1, count: %2, current: %3, total: %4, "
          "counterType: %5").arg(stage).arg(count).arg(current).arg(total).arg(
          counterType), ContentType::PlainText);
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
