// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QtWidgets>
#include <QFileInfo>

#include "mainwindow.h"
#include "comm/concord.h"
#include "lib/settings.h"
#include "lib/icon.h"
#include "defaults.h"
#include "version.h"

using namespace std;

MainWindow::MainWindow(bool haveLogLevel, int logLevel)
{
  if (haveLogLevel) {
    this->logLevel = logLevel;
  }

  readSettings();

  createStatusBar();
  createLog();
  createData();
  createAds();
  createWidgets();
  createActions();
  applySettings();

  setUnifiedTitleAndToolBarOnMac(true);

  if (curFile.isEmpty()) {
    newFile();
  } else {
    loadFile(curFile); //can be overwritten by main.cpp -- loadFile (cli)
  }

  log->addMessage("Ready!");
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  if (maybeSave()) {
    writeSettings();
    event->accept();
  } else {
    event->ignore();
  }
}

void MainWindow::onRequestQuit()
{
  close();
}

void MainWindow::newFile()
{
  if (maybeSave()) {
    undo.clear();
    config->create();
    updateModelView();
    setCurrentFile(QString());
  }
}

void MainWindow::open()
{
  auto ret = maybeSave();
  if (ret) {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"),
        QDir::homePath(),
        tr("%1 Files (*.%1);;All Files (*)").arg(
            document::Config::defaultFilePostfix), nullptr);
    loadFile(fileName);
  }
}

bool MainWindow::save()
{
  if (curFile.isEmpty()) {
    return saveAs();
  } else {
    return saveFile(curFile);
  }
}

bool MainWindow::saveAs()
{
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save Project"),
      QDir::homePath() + tr("/myRemote."
          "%1").arg(document::Config::defaultFilePostfix),
      tr("%1 Files (*.%1);;All Files (*)").arg(
          document::Config::defaultFilePostfix));
  auto ret = saveFile(fileName);
  if (ret) {
    lib::getQSettings().setValue("file", fileName);
  }
  return ret;
}

void MainWindow::import()
{
  vector<uint8_t> data;

  auto ret = maybeSave();
  if (ret) {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import Config"),
        QDir::homePath(), tr("hex Files (*.hex);;All Files (*)"));

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    ret = concord->stripHeader(fileName, data);
    if (!ret) {
      QMessageBox::warning(this, tr("Application"),
          tr("Error importing config %1 (reading)").arg(
              QDir::toNativeSeparators(fileName)));
      QGuiApplication::restoreOverrideCursor();
      return;
    }
    importData(data);
    QGuiApplication::restoreOverrideCursor();
  }
}

void MainWindow::about()
{
  QMessageBox::about(this, tr("About %1").arg(QString(PROGRAM_NAME)),
      tr("<b>%1</b> helps you to program your Logitech Harmony "
          "remote control without the always-online (now always-offline) "
          "software. <br/><br/> %2 <br/><br/> "
          "<a href=\"https://github.com/martin-wagner/harmony900tools\">"
          "GitHub Project Home</a>").arg(QString(PROGRAM_NAME)).arg(
          infoText()));
}

void MainWindow::documentWasModified()
{
  // todo setWindowModified(textEdit->document()->isModified());
}

void MainWindow::createStatusBar()
{
  statusBar()->showMessage(tr("Ready"));
}

void MainWindow::createLog()
{
  log = new LogViewer;
  log->setLoglevel(static_cast<LogLevel>(logLevel));
  log->setStatusBar(statusBar());
  log->addEntry(LogLevel::Info, infoText(), ContentType::Html);
}

void MainWindow::createData()
{
  ctx = make_unique<Context>(*settings, *user, undo);

  concord = new Concord(*ctx, this);
  ctx->setConcord(concord);
  config = new document::Config(*ctx, false, this);
  ctx->setConfig(config);
}

void MainWindow::createAds()
{
  // Must be set before creating CDockManager
  // @formatter:off
  ads::CDockManager::setConfigFlag(ads::CDockManager::OpaqueSplitterResize, true);
  ads::CDockManager::setConfigFlag(ads::CDockManager::XmlCompressionEnabled, false);
  ads::CDockManager::setConfigFlag(ads::CDockManager::DockAreaHasUndockButton, false);
  ads::CDockManager::setConfigFlag(ads::CDockManager::AllTabsHaveCloseButton, true);
  ads::CDockManager::setConfigFlag(ads::CDockManager::DockAreaDynamicTabsMenuButtonVisibility, true);
  ads::CDockManager::setConfigFlag(ads::CDockManager::FocusHighlighting, true);
  ads::CDockManager::setConfigFlag(ads::CDockManager::EqualSplitOnInsertion, true);
  ads::CDockManager::setConfigFlag(ads::CDockManager::MiddleMouseButtonClosesTab, true);
  ads::CDockManager::setConfigFlag(ads::CDockManager::DisableTabTextEliding, true);
// @formatter:on
  dockManager = new ads::CDockManager(this);

  auto qsettings = lib::getQSettings();
  dockManager->loadPerspectives(qsettings);

  dockMenu = new QMenu(tr("&View"), this);
}

void MainWindow::createWidgets()
{
  //mainwindow
  setTitle();

  //create widgets
  concordConnection = new ConcordConnection(*ctx.get(), *concord, this);
  ads::CDockWidget *dockConcordTest = new ads::CDockWidget(dockManager,
      tr("Test LibConcord"));
  dockConcordTest->setWidget(concordConnection);
  dockManager->addDockWidget(ads::RightDockWidgetArea, dockConcordTest);
  dockMenu->addAction(dockConcordTest->toggleViewAction());

  deviceEditor = new editors::DeviceEditor(*ctx.get(), nullptr, this);
  ads::CDockWidget *dockDeviceEditor = new ads::CDockWidget(dockManager,
      tr("Edit Devices"));
  dockDeviceEditor->setWidget(deviceEditor);
  dockManager->addDockWidget(ads::LeftDockWidgetArea, dockDeviceEditor);
  dockMenu->addAction(dockDeviceEditor->toggleViewAction());

  activityEditor = new editors::ActivityEditor(*ctx.get(), nullptr, this);
  ads::CDockWidget *dockActivityEditor = new ads::CDockWidget(dockManager,
      tr("Edit Activities"));
  dockActivityEditor->setWidget(activityEditor);
  dockManager->addDockWidget(ads::LeftDockWidgetArea, dockActivityEditor);
  dockMenu->addAction(dockActivityEditor->toggleViewAction());

  QUndoView *undoView = new QUndoView(undo.getStack());
  undoView->setCleanIcon(
      lib::getIcon(":/res/icons/BreezeConverted/64x64/actions/edit-clear.png",
          "edit-clear"));
  ads::CDockWidget *dockUndo = new ads::CDockWidget(dockManager,
      tr("Undo History"));
  dockUndo->setWidget(undoView);
  dockManager->addDockWidget(ads::RightDockWidgetArea, dockUndo);
  dockMenu->addAction(dockUndo->toggleViewAction());

  //log viewer already created
  ads::CDockWidget *dockLog = new ads::CDockWidget(dockManager,
      tr("Log Viewer"));
  dockLog->setWidget(log);
  dockManager->addDockWidget(ads::BottomDockWidgetArea, dockLog);
  dockMenu->addAction(dockLog->toggleViewAction());

  //widgets need to be available to restore docks
  auto qsettings = lib::getQSettings();
  auto docks = qsettings.value("dock");
  if (docks.isValid() && (docks.toString() != "")) {
    dockManager->restoreState(docks.toByteArray());
  } else {
    onLoadDefaultView();
  }
}

void MainWindow::createActions()
{
  //menubar / toolbar

  //file
  QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
  QToolBar *fileToolBar = addToolBar(tr("File"));
  const QIcon newIcon = lib::getIcon(
      ":/res/icons/BreezeConverted/64x64/actions/document-new.png",
      "document-new");
  QAction *newAct = new QAction(newIcon, tr("&New"), this);
  newAct->setShortcuts(QKeySequence::New);
  newAct->setStatusTip(tr("Create a new file"));
  connect(newAct, &QAction::triggered, this, &MainWindow::newFile);
  fileMenu->addAction(newAct);
  fileToolBar->addAction(newAct);

  const QIcon openIcon = lib::getIcon(
      ":/res/icons/BreezeConverted/64x64/actions/document-open.png",
      "document-open");
  QAction *openAct = new QAction(openIcon, tr("&Open..."), this);
  openAct->setShortcuts(QKeySequence::Open);
  openAct->setStatusTip(tr("Open an existing file"));
  connect(openAct, &QAction::triggered, this, &MainWindow::open);
  fileMenu->addAction(openAct);
  fileToolBar->addAction(openAct);

  const QIcon saveIcon = lib::getIcon(
      ":/res/icons/BreezeConverted/64x64/actions/document-save.png",
      "document-save");
  QAction *saveAct = new QAction(saveIcon, tr("&Save"), this);
  saveAct->setShortcuts(QKeySequence::Save);
  saveAct->setStatusTip(tr("Save the document to disk"));
  connect(saveAct, &QAction::triggered, this, &MainWindow::save);
  fileMenu->addAction(saveAct);
  fileToolBar->addAction(saveAct);

  const QIcon saveAsIcon = lib::getIcon(
      ":/res/icons/BreezeConverted/64x64/actions/document-save-as.png",
      "document-save-as");
  QAction *saveAsAct = fileMenu->addAction(saveAsIcon, tr("Save &As..."), this,
      &MainWindow::saveAs);
  saveAsAct->setShortcuts(QKeySequence::SaveAs);
  saveAsAct->setStatusTip(tr("Save the document under a new name"));

  fileMenu->addSeparator();

  const QIcon importIcon = lib::getIcon(
      ":/res/icons/BreezeConverted/64x64/actions/document-import.png",
      "document-import");
  QAction *importAct = new QAction(importIcon, tr("&Import..."), this);
  importAct->setStatusTip(tr("Import config/backup created with concordance"));
  connect(importAct, &QAction::triggered, this, &MainWindow::import);
  fileMenu->addAction(importAct);

  fileMenu->addSeparator();

  const QIcon exitIcon = lib::getIcon(
      ":/res/icons/BreezeConverted/64x64/actions/application-exit.png",
      "application-exit");
  QAction *exitAct = fileMenu->addAction(exitIcon, tr("E&xit"), this,
      &QWidget::close);
  exitAct->setShortcuts(QKeySequence::Quit);
  exitAct->setStatusTip(tr("Exit the application"));

  //edit
  QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
  QToolBar *editToolBar = addToolBar(tr("Edit"));
  editToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

  QAction *undoAct = undo.createUndoAction(this, tr("&Undo"));
  undoAct->setShortcuts(QKeySequence::Undo);
  const QIcon undoIcon = lib::getIcon(
      ":/res/icons/BreezeConverted/64x64/actions/edit-undo.png", "edit-undo");
  undoAct->setIcon(undoIcon);
  editMenu->addAction(undoAct);
  editToolBar->addAction(undoAct);

  QAction *redoAct = undo.createRedoAction(this, tr("&Redo"));
  redoAct->setShortcuts(QKeySequence::Redo);
  const QIcon redoIcon = lib::getIcon(
      ":/res/icons/BreezeConverted/64x64/actions/edit-redo.png", "edit-redo");
  redoAct->setIcon(redoIcon);
  editMenu->addAction(redoAct);
  editToolBar->addAction(redoAct);

//  const QIcon cutIcon = lib::getIcon(
//      ":/res/icons/BreezeConverted/64x64/actions/edit-cut.png", "edit-cut");
//  QAction *cutAct = new QAction(cutIcon, tr("Cu&t"), this);
//  cutAct->setShortcuts(QKeySequence::Cut);
//  cutAct->setStatusTip(tr("Cut the current selection's contents to the "
//      "clipboard"));
//  connect(cutAct, &QAction::triggered, textEdit, &QPlainTextEdit::cut);
//  editMenu->addAction(cutAct);
//  editToolBar->addAction(cutAct);
//
//  const QIcon copyIcon = lib::getIcon(
//      "/res/icons/BreezeConverted/64x64/actions/edit-copy.png", "edit-copy");
//  QAction *copyAct = new QAction(copyIcon, tr("&Copy"), this);
//  copyAct->setShortcuts(QKeySequence::Copy);
//  copyAct->setStatusTip(tr("Copy the current selection's contents to the "
//      "clipboard"));
//  connect(copyAct, &QAction::triggered, textEdit, &QPlainTextEdit::copy);
//  editMenu->addAction(copyAct);
//  editToolBar->addAction(copyAct);
//
//  const QIcon pasteIcon = lib::getIcon(
//      ":/res/icons/BreezeConverted/64x64/actions/edit-paste.png", "edit-paste");
//  QAction *pasteAct = new QAction(pasteIcon, tr("&Paste"), this);
//  pasteAct->setShortcuts(QKeySequence::Paste);
//  pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
//      "selection"));
//  connect(pasteAct, &QAction::triggered, textEdit, &QPlainTextEdit::paste);
//  editMenu->addAction(pasteAct);
//  editToolBar->addAction(pasteAct); todo

  //connection
  QMenu *connectMenu = menuBar()->addMenu(tr("&Connection"));
  concordConnection->addToMenu(connectMenu);
  QToolBar *connectToolBar = addToolBar(tr("Connection"));
  connectToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  concordConnection->addToToolbar(connectToolBar);

  //view
  lockAction = dockMenu->addAction(tr("Lock UI"));
  lockAction->setCheckable(true);
  lockAction->setChecked(false);
  connect(lockAction, &QAction::toggled, this, &MainWindow::onLockUI);

  dockMenu->addSeparator();
  dockMenu->addAction(tr("Save View..."), this, &MainWindow::onSaveView);
  dockMenu->addAction(tr("Load View..."), this, &MainWindow::onLoadView);
  dockMenu->addAction(tr("Load Default View"), this,
      &MainWindow::onLoadDefaultView);
  dockMenu->addAction(tr("Delete View..."), this, &MainWindow::onDeleteView);
  dockMenu->addAction(tr("Copy View to Clipboard"), this,
      &MainWindow::onCopyViewToClipboard);

  menuBar()->addMenu(dockMenu);
  //todo we can save / restore the default view / custom views

  //settings
  QMenu *settingsMenu = menuBar()->addMenu(tr("&Settings"));
  QAction *settingsAct = new QAction(tr("&Settings"), this);
  connect(settingsAct, &QAction::triggered, this, &MainWindow::showSettings);
  settingsMenu->addAction(settingsAct);
  QAction *resetAct = new QAction(tr("&Revert settings"), this);
  resetAct->setToolTip(tr("Revert all settings to default values"));
  connect(resetAct, &QAction::triggered, this, &MainWindow::resetSettings);
  settingsMenu->addAction(resetAct);

  //help
  menuBar()->addSeparator();
  QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
  QAction *aboutAct = helpMenu->addAction(tr("&About project"), this,
      &MainWindow::about);
  aboutAct->setStatusTip(tr("Show the application's About box"));

  QAction *aboutQtAct = helpMenu->addAction(tr("About &Qt"), qApp,
      &QApplication::aboutQt);
  aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));

  //signal / slot

//  cutAct->setEnabled(false);
//  copyAct->setEnabled(false);
//  connect(textEdit, &QPlainTextEdit::copyAvailable, cutAct,
//      &QAction::setEnabled);
//  connect(textEdit, &QPlainTextEdit::copyAvailable, copyAct,
//      &QAction::setEnabled); todo

  connect(config, &document::Config::dirtyChanged, this,
      &MainWindow::documentWasModified);
  connect(config, &document::Config::writeLog, log, &LogViewer::addEntry);
  connect(config, &document::Config::writeMsg, log, &LogViewer::addMessage);

  connect(qApp, &QGuiApplication::commitDataRequest, this,
      &MainWindow::commitData);

  connect(settings, &Settings::settingsAccepted, this,
      &MainWindow::applySettings);

  connect(concord, &Concord::writeLog, log, &LogViewer::addEntry);
  connect(concord, &Concord::writeMsg, log, &LogViewer::addMessage);

  connect(concordConnection, &ConcordConnection::writeLog, log,
      &LogViewer::addEntry);
  connect(concordConnection, &ConcordConnection::writeMsg, log,
      &LogViewer::addMessage);
  connect(concordConnection, &ConcordConnection::doImport, this,
      &MainWindow::onDoImport);
  connect(concordConnection, &ConcordConnection::requestQuit, this,
      &MainWindow::onRequestQuit);

  connect(deviceEditor, &editors::DeviceEditor::writeLog, log,
      &LogViewer::addEntry);
  connect(deviceEditor, &editors::DeviceEditor::writeMsg, log,
      &LogViewer::addMessage);
  connect(deviceEditor, &editors::DeviceEditor::enableLearnMode,
      concordConnection, &ConcordConnection::enableLearnMode);
  connect(concordConnection, &ConcordConnection::learnedCommand, deviceEditor,
      &editors::DeviceEditor::setLearnedCommand);

  connect(activityEditor, &editors::ActivityEditor::writeLog, log,
      &LogViewer::addEntry);
  connect(activityEditor, &editors::ActivityEditor::writeMsg, log,
      &LogViewer::addMessage);
}

void MainWindow::readSettings()
{
  auto qsettings = lib::getQSettings();
  settings = new Settings(this);
  user = new lib::UserLevel(*settings, this);

  settings->addSetting(defaults::loglevel());
  if (logLevel < 0) {
    logLevel = settings->value(defaults::loglevel().key).toInt();
  } else {
    //keep cli value
    settings->setValue(defaults::loglevel().key, logLevel);
  }

  settings->addSetting(defaults::undoMacros());
  settings->addSetting(defaults::xmPretty());

  settings->addSetting(defaults::loadLastUsed());
  auto load = settings->value(defaults::loadLastUsed().key).toBool();
  if (load) {
    curFile = qsettings.value("file").toString();
  }

  settings->addSetting(defaults::columWithFactor());
  settings->addSetting(defaults::stackedView());

  const QByteArray geometry =
      qsettings.value("geometry", QByteArray()).toByteArray();
  if (geometry.isEmpty()) {
    const QRect availableGeometry = screen()->availableGeometry();
    resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
    move((availableGeometry.width() - width()) / 2,
        (availableGeometry.height() - height()) / 2);
  } else {
    restoreGeometry(geometry);
    restoreState(qsettings.value("window").toByteArray());
  }

  //todo add setting for this. windows default skin is still windows95 style
#ifdef _WIN32
  auto availableStyles = QStyleFactory::keys();
  if (availableStyles.contains("windows11", Qt::CaseInsensitive)) {
      QApplication::setStyle("windows11");
  } else if (availableStyles.contains("fusion", Qt::CaseInsensitive)) {
      QApplication::setStyle("fusion");
  }
#endif
}

void MainWindow::writeSettings()
{
  auto settings = lib::getQSettings();
  settings.setValue("geometry", saveGeometry());
  settings.setValue("window", saveState());
  settings.setValue("dock", dockManager->saveState());
  dockManager->savePerspectives(settings);
}

bool MainWindow::maybeSave()
{
  if (!config->isDirty()) {
    return true;
  }
  const QMessageBox::StandardButton ret = QMessageBox::warning(this,
      tr("Application"), tr("The project has been modified.\n"
          "Do you want to save your changes?"),
      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
  switch (ret) {
    case QMessageBox::Save:
      return save();
    case QMessageBox::Cancel:
      return false;
    default:
      break;
  }
  return true;
}

void MainWindow::loadFile(const QString &fileName)
{
  if (fileName.isEmpty()) {
    return;
  }

  if (!QFileInfo::exists(fileName) || !QFileInfo(fileName).isFile()) {
    QMessageBox::warning(this, tr("Application"),
        tr("Cannot read project %1.").arg(QDir::toNativeSeparators(fileName)));
    QGuiApplication::restoreOverrideCursor();
    return;
  }

  QGuiApplication::setOverrideCursor(Qt::WaitCursor);
  undo.clear();
  auto res = config->read(fileName);
  if (!res) {
    QMessageBox::warning(this, tr("Application"),
        tr("Error reading project %1.").arg(
            QDir::toNativeSeparators(fileName)));
    QGuiApplication::restoreOverrideCursor();
    return;
  }
  updateModelView();
  QGuiApplication::restoreOverrideCursor();

  setCurrentFile(fileName);
  log->addMessage(tr("Project loaded"));
}

bool MainWindow::saveFile(const QString &fileName)
{
  if (fileName.isEmpty()) {
    return false;
  }

  QGuiApplication::setOverrideCursor(Qt::WaitCursor);
  bool res = config->saveAs(fileName);
  if (!res) {
    QMessageBox::warning(this, tr("Application"),
        tr("Cannot write project %1.").arg(QDir::toNativeSeparators(fileName)));
    QGuiApplication::restoreOverrideCursor();
    return false;
  }
  QGuiApplication::restoreOverrideCursor();

  setCurrentFile(fileName);
  log->addMessage(tr("Project saved"));
  return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
  curFile = fileName;
  setWindowModified(false);

  QString shownName = curFile;
  if (curFile.isEmpty()) {
    shownName = "untitled." + document::Config::defaultFilePostfix;
  }
  setWindowFilePath(shownName);
  setTitle(shownName);
}

QString MainWindow::infoText()
{
  // @formatter:off
  const QString text = QString(
      "Version: %1.%2.%3%4<br>"
      "Git hash: %5<br>"
      "Compiler: %6<br>"
      "Build user: %7<br>"
      "Host OS: %8<br>"
      "Build date: %9 %10"
  )
  .arg(BuildInfo::versionMajor)
  .arg(BuildInfo::versionMinor)
  .arg(BuildInfo::versionPatch)
  .arg(BuildInfo::versionDirty ? " (dirty)" : "")
  .arg(QString::fromUtf8(BuildInfo::gitHash.data(),         BuildInfo::gitHash.size()))
  .arg(QString::fromUtf8(BuildInfo::compilerVersion.data(), BuildInfo::compilerVersion.size()))
  .arg(QString::fromUtf8(BuildInfo::buildUser.data(),       BuildInfo::buildUser.size()))
  .arg(QString::fromUtf8(BuildInfo::hostOs.data(),          BuildInfo::hostOs.size()))
  .arg(__DATE__)
  .arg(__TIME__);
// @formatter:on
  return text;
}

QString MainWindow::strippedName(const QString &fullFileName)
{
  return QFileInfo(fullFileName).fileName();
}

void MainWindow::commitData(QSessionManager &manager)
{
  if (manager.allowsInteraction()) {
    if (!maybeSave()) {
      manager.cancel();
    }
  } else {
    // Non-interactive: save without asking
    if (!curFile.isEmpty() && (config != nullptr) && (config->isDirty())) {
      config->saveAs(curFile);
    }
  }
}

void MainWindow::updateModelView()
{
  if (deviceModel != nullptr) {
    deviceModel->deleteLater();
  }
  deviceModel = new models::DeviceModel(*config, this);
  deviceEditor->setModel(deviceModel);
  connect(deviceModel, &models::DeviceModel::writeLog, log,
      &LogViewer::addEntry);
  connect(deviceModel, &models::DeviceModel::writeMsg, log,
      &LogViewer::addMessage);

  if (activityModel != nullptr) {
    activityModel->deleteLater();
  }
  activityModel = new models::ActivityModel(*config, this);
  activityEditor->setModel(activityModel);
  connect(activityModel, &models::ActivityModel::writeLog, log,
      &LogViewer::addEntry);
  connect(activityModel, &models::ActivityModel::writeMsg, log,
      &LogViewer::addMessage);
}

void MainWindow::showSettings()
{
  settings->show();
  settings->raise();
  settings->activateWindow();  // bring to front if already open
}

void MainWindow::resetSettings()
{
  QMessageBox::StandardButton reply;
  reply = QMessageBox::question(this, tr("Reset"),
      tr("Reset ALL settings to default and exit program without saving?"),
      QMessageBox::Yes | QMessageBox::No);
  if (reply == QMessageBox::Yes) {
    lib::resetQSettings();
    QCoreApplication::exit(); // not calling closeEvent
  }
}

void MainWindow::applySettings()
{
  logLevel = settings->value(defaults::loglevel().key).toInt();
  log->setLoglevel(static_cast<LogLevel>(logLevel));
  log->addMessage("Setting loglevel to " + QString(logLevelName(logLevel)));

  log->addMessage("Setting userlevel to " + user->levelToString());

  auto debugMacros = settings->value(defaults::undoMacros().key).toBool();
  undo.setMacrosDisabled(debugMacros);

  //todo weitere
}

void MainWindow::onLockUI(bool locked)
{
  if (locked) {
    dockManager->lockDockWidgetFeaturesGlobally();
  } else {
    dockManager->lockDockWidgetFeaturesGlobally(
        ads::CDockWidget::NoDockWidgetFeatures);
  }
}

void MainWindow::onSaveView()
{
  QString name = QInputDialog::getText(this, tr("Save View"), tr("View name:"));
  if (name.isEmpty()) {
    return;
  }
  dockManager->addPerspective(name);
}

void MainWindow::onLoadView()
{
  bool ok;

  QStringList views = dockManager->perspectiveNames();
  if (views.isEmpty()) {
    QMessageBox::information(this, tr("Load View"),
        tr("No saved views found."));
    return;
  }
  ok = false;
  QString name = QInputDialog::getItem(this, tr("Load View"),
      tr("Select view:"), views, 0, false, &ok);
  if (!ok || name.isEmpty()) {
    return;
  }
  dockManager->openPerspective(name);
}

void MainWindow::onLoadDefaultView()
{
  dockManager->restoreState(defaults::adsDock);
  resize(1400, 900);
}

void MainWindow::onDeleteView()
{
  bool ok;

  QStringList views = dockManager->perspectiveNames();
  if (views.isEmpty()) {
    QMessageBox::information(this, tr("Delete View"),
        tr("No saved views found."));
    return;
  }
  ok = false;
  QString name = QInputDialog::getItem(this, tr("Delete View"),
      tr("Select view:"), views, 0, false, &ok);
  if (!ok || name.isEmpty()) {
    return;
  }
  dockManager->removePerspective(name);
}

void MainWindow::onCopyViewToClipboard()
{
  QString view = dockManager->saveState();
  QApplication::clipboard()->setText(view);
}

void MainWindow::onDoImport(const vector<uint8_t> &data)
{
  importData(data);
}

void MainWindow::setTitle(const QString &append)
{
  setWindowTitle(
      QString(PROGRAM_NAME) + " " + QString::fromUtf8(BuildInfo::versionFull)
          + " -- " + append);
}

bool MainWindow::importData(const std::vector<uint8_t> &data)
{
  undo.clear();
  auto ret = config->read(data, document::Type::H900);
  if (!ret) {
    QMessageBox::warning(this, tr("Application"), tr("Error parsing config"));
    return false;
  }
  updateModelView();
  setCurrentFile(QString());
  log->addMessage(tr("Project loaded"));
  return true;
}
