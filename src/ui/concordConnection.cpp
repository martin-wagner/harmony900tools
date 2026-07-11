// SPDX-License-Identifier: LGPL-2.1-or-later

#include <memory>

#include "concordConnection.h"
#include "lib/icon.h"
#include "document/config.h"

//plain c functions -- never use outside cpp file (->global namespace)
#include "wrappers/concordWrapper.h"
#include "bin/timing.h"

using namespace std;

ConcordConnection::ConcordConnection(Context &ctx, Concord &concord,
    QWidget *parent) :
    QWidget(parent), ctx(ctx), concord(concord)
{
  createSettings();
  createWidgets();
  createActions();
  updateActionStates();
}

ConcordConnection::~ConcordConnection()
{
  cleanup();
}

void ConcordConnection::addToToolbar(QToolBar *bar)
{
  if (actionConnect == nullptr) {
    return;
  }
  bar->addAction(actionConnect);
  bar->addAction(actionDisconnect);
  bar->addSeparator();
  bar->addAction(actionWriteConfig);
  bar->addSeparator();
  bar->addAction(actionLearnIrSingle);
  bar->addAction(actionLearnIrStream);
  bar->addAction(actionProgress);
}

void ConcordConnection::addToMenu(QMenu *menu)
{
  if (actionConnect == nullptr) {
    return;
  }

  menu->addAction(actionConnect);
  menu->addAction(actionDisconnect);
  menu->addSeparator();
  menu->addAction(actionGetInfo);
  menu->addAction(actionSetTime);
  menu->addSeparator();
  menu->addAction(actionReadConfig);
  menu->addAction(actionWriteConfig);
  menu->addSeparator();
  menu->addAction(actionBackupConfig);
  menu->addAction(actionBackupConfigRestore);
  menu->addSeparator();
  menu->addAction(actionLearnIrSingle);
  menu->addAction(actionLearnIrStream);
}

void ConcordConnection::onOpenConnection()
{
  if (concord.isInitialised()) {
    return;
  }
  cleanup();

  onUpdateProgress("Connecting...", 0, 0);

  auto ret = concord.connectRemote();
  if (ret) {
    updateActionStates();
    labelConnection->setText(tr("connected"));
  }
}

void ConcordConnection::onCloseConnection()
{
  concord.disconnectRemote();
  cleanup();
  updateActionStates();
  labelConnection->setText(tr("disconnected"));
}

void ConcordConnection::onGetInfo()
{
  auto mnf = concord.mnf();
  auto model = concord.model();
  auto fwVersion = concord.fwVersion();
  auto hwVersion = concord.hwVersion();

  QMessageBox msgBox(QMessageBox::Information, tr("Info"),
      tr("You are connected to a %1 %2 (FW: %3, HW: %4)").arg(mnf, model,
          fwVersion, hwVersion), QMessageBox::Ok, this);
  msgBox.exec();
}

void ConcordConnection::onGetTime()
{
  updateActionStates(true);
  concord.readTime();
}

void ConcordConnection::onSetTime()
{
  updateActionStates(true);
  concord.setTime();
}

void ConcordConnection::onReadConfig()
{
  emit writeLog(LogLevel::Debug, tr("load config from remote"),
      ContentType::PlainText);

  dataMode = true;
  updateActionStates(true);
  concord.readUserConfig();
}

void ConcordConnection::onWriteConfig()
{
  vector<uint8_t> data;

  emit writeLog(LogLevel::Debug, tr("write config to remote"),
      ContentType::PlainText);

  auto ret = ctx.config()->dumpZip(data, document::Type::H900);
  if (!ret) {
    emit writeLog(LogLevel::Error, tr("serialising failed"),
        ContentType::PlainText);
    return;
  }

  emit writeLog(LogLevel::Critical,
      tr("not ready for this yet -- rejecting write"), ContentType::PlainText);
  return; //todo

  updateActionStates(true);
  concord.updateUserConfigData(data, false);
}

void ConcordConnection::onBackupConfig()
{
  onUpdateProgress("Backup Config...", 0, 0);

  updateActionStates(true);
  dataMode = false;
  concord.readUserConfig();
}

void ConcordConnection::onBackupConfigRestore()
{
  auto file = QFileDialog::getOpenFileName(this, tr("Open File"),
      QDir::homePath(), tr("hex Files (*.hex);;All Files (*)"));
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

  updateActionStates(true);
  concord.updateUserConfig(file);
}

void ConcordConnection::onLearnIrSingle()
{
  onUpdateProgress("Learn command...", 0, 0);

  updateActionStates(true);
  concord.learnCommand();
}

void ConcordConnection::onLearnIrStream()
{
  onUpdateProgress("Learn command...", 0, 0);

  auto timeout =
      ctx.settings().value(defaults::learnStreamTimeout().key).toInt();
  updateActionStates(true);
  concord.learnStream(timeout);
}

void ConcordConnection::onUpdateProgress(const QString text, int step, int of)
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

void ConcordConnection::onDisconnected(int time_s)
{
  if (disconnectMsg != nullptr) {
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
    updateActionStates();
  });

  updateActionStates();
  disconnectMsg->show();
}

void ConcordConnection::onDone(bool success, const QString &msg)
{
  emit writeMsg(msg);
  labelConnection->setText(msg);
  progressBar->setRange(0, 100);
  progressBar->setValue(100);
  if (success) {
    progressBar->setFormat(tr("%1: OK").arg(progressBar->text()));
  } else {
    progressBar->setFormat(tr("Error in %1").arg(progressBar->text()));
    emit writeLog(LogLevel::Error,
        tr("libconcord error in: %1").arg(progressBar->text()),
        ContentType::PlainText);
  }
  updateActionStates();
}

void ConcordConnection::onTime(const QString time)
{
  QMessageBox msgBox(QMessageBox::Information, tr("Info"),
      tr("Harmony Time: %1").arg(time), QMessageBox::Ok, this);
  msgBox.exec();
}

void ConcordConnection::onLearnWindowIsOpen(bool waits)
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
    if (waitMsg != nullptr) {
      waitMsg->hide();
    }
  }
}

void ConcordConnection::onLearnDone(const binary::TimingStream &t,
    uint32_t carrier)
{
  if (t.timings().size() > 0) {
    emit writeMsg("Learned Command");
    labelConnection->setText("Learned Command");
  } else {
    emit writeMsg("Learn Error");
    labelConnection->setText("Learn Error");
  }
}

void ConcordConnection::onBackupConfigDone(bool success)
{
  if (!success) {
    dataMode = false;
    return;
  }

  if (dataMode) {
    dataMode = false;
    auto data = concord.getUserConfig();
    emit doImport(data);
    return;
  }

  auto file = QFileDialog::getSaveFileName(this, tr("Save XML + Zip Config"),
      QDir::homePath() + "/Backup.hex", tr("hex Files (*.hex);;All Files (*)"));
  if (!file.isEmpty()) {
    //write xml + zip file
    auto ret = concord.writeUserConfigFile(file, true);
    if (ret != 0) {
      emit writeLog(LogLevel::Error, tr("write file error"),
          ContentType::PlainText);
      return;
    }
  }
}

void ConcordConnection::createSettings()
{
  ctx.settings().addSetting(defaults::learnStreamTimeout());
}

void ConcordConnection::createWidgets()
{
  layout = new QGridLayout(this);

  auto label = new QLabel(tr("Test LibConcord"), this);
  layout->addWidget(label, 1, 0, 1, 3);

  buttonOpenConnection = new QPushButton(tr("Open connection"), this);
  layout->addWidget(buttonOpenConnection, 2, 0);
  buttonCloseConnection = new QPushButton(tr("Close connection"), this);
  layout->addWidget(buttonCloseConnection, 2, 1);
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
  progressBar->setFixedWidth(200);
  progressBar->setValue(0);
  progressBar->setTextVisible(true);
  progressBar->setFormat("");
  layout->addWidget(progressBar, 6, 0, 1, 3); //gets moved to toolbar when toolbar is connected. otherwise remains here.
}

void ConcordConnection::createActions()
{
  actionConnect = new QAction(tr("Connect"), this);
  actionConnect->setStatusTip(tr("Connect to remote"));
  actionConnect->setIcon(
      lib::getIcon(
          ":/res/icons/BreezeConverted/64x64/actions/network-connect.png"));
  connect(actionConnect, &QAction::triggered, this,
      &ConcordConnection::onOpenConnection);

  actionDisconnect = new QAction(tr("Disconnect"), this);
  actionDisconnect->setStatusTip(tr("Disconnect from remote"));
  actionDisconnect->setIcon(
      lib::getIcon(
          ":/res/icons/BreezeConverted/64x64/actions/network-disconnect.png"));
  connect(actionDisconnect, &QAction::triggered, this,
      &ConcordConnection::onCloseConnection);

  actionGetInfo = new QAction(tr("Remote Info"), this);
  actionGetInfo->setStatusTip(tr("Show remote device information"));
  actionGetInfo->setIcon(
      lib::getIcon(
          ":/res/icons/BreezeConverted/64x64/status/dialog-information.png",
          "dialog-information"));
  connect(actionGetInfo, &QAction::triggered, this,
      &ConcordConnection::onGetInfo);

  actionSetTime = new QAction(tr("Set Time"), this);
  actionSetTime->setStatusTip(tr("Sync remote time to system time"));
  actionSetTime->setIcon(
      lib::getIcon(
          ":/res/icons/BreezeConverted/64x64/misc/org.kde.plasma.analogclock.png",
          "clock"));
  connect(actionSetTime, &QAction::triggered, this,
      &ConcordConnection::onSetTime);

  actionReadConfig = new QAction(tr("Read Config from remote"), this);
  actionReadConfig->setStatusTip(tr("Read configuration from remote"));
  actionReadConfig->setIcon(
      lib::getIcon(
          ":/res/icons/BreezeConverted/64x64/actions/edit-download.png"));
  connect(actionReadConfig, &QAction::triggered, this,
      &ConcordConnection::onReadConfig);

  actionWriteConfig = new QAction(tr("Write Config to remote"), this);
  actionWriteConfig->setStatusTip(tr("Write configuration to remote"));
  actionWriteConfig->setIcon(
      lib::getIcon(
          ":/res/icons/BreezeConverted/64x64/actions/kt-set-max-upload-speed.png"));
  connect(actionWriteConfig, &QAction::triggered, this,
      &ConcordConnection::onWriteConfig);

  actionLearnIrSingle = new QAction(tr("Learn IR"), this);
  actionLearnIrSingle->setStatusTip(tr("Learn a single IR command"));
  actionLearnIrSingle->setIcon(
      lib::getIcon(
          ":/res/icons/BreezeConverted/64x64/actions/media-record.png"));
  connect(actionLearnIrSingle, &QAction::triggered, this,
      &ConcordConnection::onLearnIrSingle);

  actionLearnIrStream = new QAction(tr("Learn IR Stream"), this);
  actionLearnIrStream->setStatusTip(tr("Learn an IR command stream"));
  actionLearnIrStream->setIcon(
      lib::getIcon(
          ":/res/icons/BreezeConverted/64x64/actions/keyframe-record.png"));
  connect(actionLearnIrStream, &QAction::triggered, this,
      &ConcordConnection::onLearnIrStream);

  actionBackupConfig = new QAction(tr("Create remote backup"), this);
  actionBackupConfig->setStatusTip(
      tr("Backup configuration from remote to a file"));
  actionBackupConfig->setIcon(
      lib::getIcon(":/res/icons/BreezeConverted/64x64/actions/backup.png"));
  connect(actionBackupConfig, &QAction::triggered, this,
      &ConcordConnection::onBackupConfig);

  actionBackupConfigRestore = new QAction(tr("Restore Backup"), this);
  actionBackupConfigRestore->setStatusTip(
      tr("Restore remote from previous backup"));
  connect(actionBackupConfigRestore, &QAction::triggered, this,
      &ConcordConnection::onBackupConfigRestore);

  actionProgress = new QWidgetAction(this);
  actionProgress->setDefaultWidget(progressBar);

  connect(buttonOpenConnection, &QPushButton::clicked, this,
      &ConcordConnection::onOpenConnection);
  connect(buttonCloseConnection, &QPushButton::clicked, this,
      &ConcordConnection::onCloseConnection);
  connect(buttonGetInfo, &QPushButton::clicked, this,
      &ConcordConnection::onGetInfo);
  connect(buttonGetTime, &QPushButton::clicked, this,
      &ConcordConnection::onGetTime);
  connect(buttonSetTime, &QPushButton::clicked, this,
      &ConcordConnection::onSetTime);
  connect(buttonReadConfigFromRemote, &QPushButton::clicked, this,
      &ConcordConnection::onBackupConfig);
  connect(buttonWriteConfigToRemote, &QPushButton::clicked, this,
      &ConcordConnection::onBackupConfigRestore);
  connect(buttonLearnIrSingle, &QPushButton::clicked, this,
      &ConcordConnection::onLearnIrSingle);
  connect(buttonLearnIrStream, &QPushButton::clicked, this,
      &ConcordConnection::onLearnIrStream);

  connect(&concord, &Concord::updateProgress, this,
      &ConcordConnection::onUpdateProgress);
  connect(&concord, &Concord::disconnected, this,
      &ConcordConnection::onDisconnected);
  connect(&concord, &Concord::done, this, &ConcordConnection::onDone);
  connect(&concord, &Concord::time, this, &ConcordConnection::onTime);
  connect(&concord, &Concord::learnWindowIsOpen, this,
      &ConcordConnection::onLearnWindowIsOpen);
  connect(&concord, &Concord::learnDone, this, &ConcordConnection::onLearnDone);
  connect(&concord, &Concord::readUserConfigDone, this,
      &ConcordConnection::onBackupConfigDone);
}

void ConcordConnection::updateActionStates(bool setToBusy)
{
  bool state;

  auto connected = concord.isConnected();
  if (setToBusy) {
    state = false;
  } else {
    state = connected;
  }

  actionConnect->setEnabled(!connected);
  actionDisconnect->setEnabled(state);
  actionGetInfo->setEnabled(state);
  actionSetTime->setEnabled(state);
  actionReadConfig->setEnabled(state);
  actionWriteConfig->setEnabled(state);
  actionBackupConfig->setEnabled(state);
  actionBackupConfigRestore->setEnabled(state);
  actionLearnIrSingle->setEnabled(state);
  actionLearnIrStream->setEnabled(state);
  if (!setToBusy) {
    progressBar->setEnabled(state);
  }
}

void ConcordConnection::cleanup()
{
  dataMode = false;
  if (disconnectMsg != nullptr) {
    disconnectMsg->deleteLater();
    disconnectMsg = nullptr;
  }
  if (waitMsg != nullptr) {
    waitMsg->deleteLater();
    waitMsg = nullptr;
  }
}
