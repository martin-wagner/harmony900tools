#include "concordTest.h"

#include <memory>

//plain c functions -- never use outside cpp file (->global namespace)
#include "wrappers/concordWrapper.h"
#include "bin/data.h"

using namespace std;

ConcordTest::ConcordTest(Context &ctx, Concord &concord, QWidget *parent) :
    QWidget(parent), ctx(ctx), concord(concord)
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
  if (concord.isInitialised()) {
    return;
  }

  onUpdateProgress("Connecting...", 0, 0);

  auto ret = concord.connectRemote();
  if (ret) {
    labelConnection->setText(tr("connected"));
  }
}

void ConcordTest::onCloseConnection()
{
  concord.disconnectRemote();
  cleanup();
  labelConnection->setText(tr("dsconnected"));
}

void ConcordTest::onGetInfo()
{
  if (!concord.isInitialised()) {
    return;
  }

  auto mnf = concord.mnf();
  auto model = concord.model();
  auto fwVersion = concord.fwVersion();
  auto hwVersion = concord.hwVersion();

  QMessageBox msgBox(QMessageBox::Warning, tr("Info"),
      tr("You are connected to a %1 %2 (FW: %3, HW: %4)").arg(mnf, model,
          fwVersion, hwVersion), QMessageBox::Ok, this);
  msgBox.exec();
}

void ConcordTest::onGetTime()
{
  concord.readTime();
}

void ConcordTest::onSetTime()
{
  concord.setTime();
}

void ConcordTest::onReadConfig()
{
  onUpdateProgress("Read Config...", 0, 0);

  concord.readUserConfig();
}

void ConcordTest::onWriteConfig()
{
  if (!concord.isInitialised()) {
    return;
  }

  QString file = QFileDialog::getOpenFileName(this, tr("Open File"),
      QDir::homePath(), tr("hex Files (*.hex);;All Files (*)"), nullptr,
      QFileDialog::DontUseNativeDialog);
  if (file.isEmpty()) {
    return;
  }
  auto size = QFile(file).size();
  if (size == 0) {
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(tr("File is empty"));
    msgBox.exec();
    return;
  }

  onUpdateProgress("Write Config...", 0, 0);

  concord.updateUserConfig(file);
}

void ConcordTest::onLearnIrSingle()
{
  onUpdateProgress("Learn command...", 0, 0);

  concord.learnCommand();
}

void ConcordTest::onLearnIrStream()
{
  onUpdateProgress("Learn command...", 0, 0);

  concord.learnStream(2500);
}

void ConcordTest::onUpdateProgress(const QString text, int step, int of)
{
  int percent;

  if (of <= 0) {
    // unknown duration -> busy/activity indicator
    progressBar->setRange(0, 0);
  } else {
    // normal progress mode
    progressBar->setRange(0, 100);

    percent = (step * 100) / of;
    percent = qBound(0, percent, 100);
    progressBar->setValue(percent);
  }
  progressBar->setFormat(text);
}

void ConcordTest::onDisconnected(int time_s)
{
  if (disconnectMsg) {
    return; // already shown
  }

  disconnectMsg = new QMessageBox(QMessageBox::Warning, tr("Disconnected"),
      tr("Connection to your Harmony is lost. Reconnect "
          "your remote, wait 10s and click \"OK\"\n\n"
          "If you didn't disconnect your remote, disconnect it now, remove\n"
          "and reinsert the battery, wait for the main "
          "screen to show, reconnect it and then click \"OK\""),
      QMessageBox::Ok);
  disconnectMsg->setAttribute(Qt::WA_DeleteOnClose);

  connect(disconnectMsg, &QObject::destroyed, this, [this]() {
    disconnectMsg = nullptr;
  });

  disconnectMsg->show();
}

void ConcordTest::onDone(bool success, const QString &msg)
{
  emit writeMsg(msg);
  labelConnection->setText(msg);
  progressBar->setRange(0, 100);
  progressBar->setValue(100);
  if (success) {
    progressBar->setFormat(tr("%1: OK").arg(progressBar->text()));
  } else {
    progressBar->setFormat(tr("Error in %1").arg(progressBar->text()));
  }
}

void ConcordTest::onTime(const QString time)
{
  QMessageBox msgBox(QMessageBox::Warning, tr("Info"),
      tr("Harmony Time: %1").arg(time), QMessageBox::Ok, this);
  msgBox.exec();
}

void ConcordTest::onLearnWindowIsOpen(bool waits)
{
  if (waits) {
    if (waitMsg == nullptr) {
      waitMsg = new QMessageBox(this);
      waitMsg->setIcon(QMessageBox::Information);
      waitMsg->setWindowTitle(tr("Info"));
      waitMsg->setText(tr("Press button on source remote"));
      waitMsg->setStandardButtons(QMessageBox::NoButton);
      waitMsg->setModal(false);
    }
    waitMsg->show();
  } else {
    if (waitMsg) {
      waitMsg->hide();
    }
  }
}

void ConcordTest::onLearnDone(const lib::TimingStream &t, uint32_t carrier)
{
  if (t.timings().size() > 0) {
    emit writeMsg("Learned Command");
    labelConnection->setText("Learned Command");
  } else {
    emit writeMsg("Learn Error");
    labelConnection->setText("Learn Error");
  }
}

void ConcordTest::onReadUserConfigDone(bool success)
{
  if (!success) {
    return;
  }

  QString filePathXml = QFileDialog::getSaveFileName(this,
      tr("Save XML + Zip Config"), QDir::homePath(),
      tr("hex Files (*.hex);;All Files (*)"));
  if (!filePathXml.isEmpty()) {
    //write xml + zip file
    auto ret = concord.writeUserConfigFile(filePathXml, true);
    if (ret != 0) {
      emit writeLog(LogLevel::Error, tr("write file error"),
          ContentType::PlainText);
      return;
    }
  }
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

  buttonLearnIrSingle = new QPushButton(tr("Learn IR Single"), this);
  layout->addWidget(buttonLearnIrSingle, 5, 0);
  buttonLearnIrStream = new QPushButton(tr("Learn IR Stream"), this);
  layout->addWidget(buttonLearnIrStream, 5, 1);
  labelIrData = new QLabel(tr("IR Data"), this);
  layout->addWidget(labelIrData, 5, 2);

  progressBar = new QProgressBar(this);
  progressBar->setMinimum(0);
  progressBar->setMaximum(100);
  progressBar->setValue(0);
  progressBar->setTextVisible(true);
  progressBar->setFormat("");
  layout->addWidget(progressBar, 6, 0, 1, 3);

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
  connect(buttonLearnIrSingle, &QPushButton::clicked, this,
      &ConcordTest::onLearnIrSingle);
  connect(buttonLearnIrStream, &QPushButton::clicked, this,
      &ConcordTest::onLearnIrStream);

  connect(&concord, &Concord::updateProgress, this,
      &ConcordTest::onUpdateProgress);
  connect(&concord, &Concord::disconnected, this, &ConcordTest::onDisconnected);
  connect(&concord, &Concord::done, this, &ConcordTest::onDone);
  connect(&concord, &Concord::time, this, &ConcordTest::onTime);
  connect(&concord, &Concord::learnWindowIsOpen, this,
      &ConcordTest::onLearnWindowIsOpen);
  connect(&concord, &Concord::learnDone, this, &ConcordTest::onLearnDone);
  connect(&concord, &Concord::readUserConfigDone, this,
      &ConcordTest::onReadUserConfigDone);
}

void ConcordTest::cleanup()
{
}
