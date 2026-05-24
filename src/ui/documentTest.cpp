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
//  if (!success) {
//    return;
//  }
//
//  QString filePathXml = QFileDialog::getSaveFileName(this,
//      tr("Save XML + Zip Config"), QDir::homePath(),
//      tr("hex Files (*.hex);;All Files (*)"));
//  if (!filePathXml.isEmpty()) {
//    //write xml + zip file
//    auto ret = concord.writeUserConfigFile(filePathXml, true);
//    if (ret != 0) {
//      emit writeLog(LogLevel::Error, tr("write file error"),
//          ContentType::PlainText);
//      return;
//    }
//  }
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
}

void DocumentTest::createActions()
{
  connect(buttonImportConfig, &QPushButton::clicked, this,
      &DocumentTest::onImport);
  connect(buttonDumpConfig, &QPushButton::clicked, this, &DocumentTest::onDump);

  connect(&config, &document::Config::writeLog, this, &DocumentTest::writeLog);
  connect(&config, &document::Config::writeMsg, this, &DocumentTest::writeMsg);
}
