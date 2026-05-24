// SPDX-License-Identifier: LGPL-2.1-or-later

#include <memory>

#include "documentTest.h"

using namespace std;

DocumentTest::DocumentTest(Context &ctx, QWidget *parent) :
    QWidget(parent), ctx(ctx), config(ctx, false)
{
  createWidgets();
  createActions();
}

DocumentTest::~DocumentTest()
{
}

void DocumentTest::onImport()
{
  QString file = QFileDialog::getOpenFileName(this, tr("Open File"),
      QDir::homePath(), tr("zip Files (*.zip);;All Files (*)"), nullptr,
      QFileDialog::DontUseNativeDialog);
  if (file.isEmpty()) {
    return;
  }
  auto f = QFile(file);
  if (!f.open(QIODevice::ReadOnly)) {
    return;
  }
  if (f.size() == 0) {
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(tr("File is empty"));
    msgBox.exec();
    return;
  }

  ctx.getUndoStack().clear();

  QByteArray data = f.readAll();
  std::vector<uint8_t> bytes(data.begin(), data.end());
  auto ret = config.read(bytes, document::Type::H900);
  if (!ret) {
    emit writeLog(LogLevel::Debug, tr("import fail"), ContentType::PlainText);
  }
}

void DocumentTest::onDump()
{
  std::vector<uint8_t> bytes;

  QString file = QFileDialog::getSaveFileName(this, tr("Save File"),
      QDir::homePath(), tr("zip Files (*.zip);;All Files (*)"));
  auto f = QFile(file);
  if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    emit writeMsg("File can't be opened");
    return;
  }

  //write data
  auto ret = config.dumpZip(bytes, document::Type::H900);
  if (!ret) {
    emit writeLog(LogLevel::Error, tr("write file error"),
        ContentType::PlainText);
    return;
  }
  QByteArray data(reinterpret_cast<const char*>(bytes.data()), bytes.size());
  f.write(data);
}

void DocumentTest::onRead()
{
  QString projectPath = QFileDialog::getExistingDirectory(this,
      tr("Select project dir"), QDir::homePath(),
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (!projectPath.isEmpty()) {
    //read project
    ctx.getUndoStack().clear();
    auto ret = config.read(projectPath);
    if (!ret) {
      emit writeLog(LogLevel::Error, tr("read project error"),
          ContentType::PlainText);
      return;
    }
  }
}

void DocumentTest::onWrite()
{
  QString projectPath = QFileDialog::getExistingDirectory(this,
      tr("Select project dir"), QDir::homePath(),
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (!projectPath.isEmpty()) {
    //write project
    auto ret = config.saveAs(projectPath);
    if (!ret) {
      emit writeLog(LogLevel::Error, tr("write project error"),
          ContentType::PlainText);
      return;
    }
  }
}

void DocumentTest::onSave()
{
  auto ret = config.save();
  if (!ret) {
    emit writeLog(LogLevel::Error, tr("write file error"),
        ContentType::PlainText);
  }
}

void DocumentTest::createWidgets()
{
  layout = new QGridLayout(this);

  auto label = new QLabel(tr("Test Data Handling"), this);
  layout->addWidget(label, 1, 0, 1, 3);

  buttonImportConfig = new QPushButton(tr("Import Config"), this);
  layout->addWidget(buttonImportConfig, 2, 0);
  buttonDumpConfig = new QPushButton(tr("Dump Config"), this);
  layout->addWidget(buttonDumpConfig, 2, 1);
  labelFileSize = new QLabel(tr("Block Size"), this);
  layout->addWidget(labelFileSize, 2, 2);

  buttonReadConfig = new QPushButton(tr("Read Config"), this);
  layout->addWidget(buttonReadConfig, 3, 0);
  buttonWriteConfig = new QPushButton(tr("Write Config"), this);
  layout->addWidget(buttonWriteConfig, 3, 1);
  buttonSaveConfig = new QPushButton(tr("Save Config"), this);
  layout->addWidget(buttonSaveConfig, 3, 2);
}

void DocumentTest::createActions()
{
  connect(buttonImportConfig, &QPushButton::clicked, this,
      &DocumentTest::onImport);
  connect(buttonDumpConfig, &QPushButton::clicked, this, &DocumentTest::onDump);
  connect(buttonReadConfig, &QPushButton::clicked, this, &DocumentTest::onRead);
  connect(buttonWriteConfig, &QPushButton::clicked, this,
      &DocumentTest::onWrite);
  connect(buttonSaveConfig, &QPushButton::clicked, this, &DocumentTest::onSave);

  connect(&config, &document::Config::writeLog, this, &DocumentTest::writeLog);
  connect(&config, &document::Config::writeMsg, this, &DocumentTest::writeMsg);
}
