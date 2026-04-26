// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtWidgets>

#include "mainwindow.h"
#include "lib/settings.h"

using namespace std;

MainWindow::MainWindow(int logLevel) :
    logLevel(logLevel), textEdit(new QPlainTextEdit)
{
  readSettings();

  createStatusBar();
  createAds();
  createWidgets();
  createActions();

  setCurrentFile(QString());
  setUnifiedTitleAndToolBarOnMac(true);
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

void MainWindow::newFile()
{
  if (maybeSave()) {
    textEdit->clear();
    setCurrentFile(QString());
  }
}

void MainWindow::open()
{
  if (maybeSave()) {
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
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
  QFileDialog dialog(this);
  dialog.setWindowModality(Qt::WindowModal);
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  if (dialog.exec() != QDialog::Accepted)
    return false;
  return saveFile(dialog.selectedFiles().first());
}

void MainWindow::about()
{
  QMessageBox::about(this, tr("About Application"),
      tr("The <b>Application</b> example demonstrates how to "
          "write modern GUI applications using Qt, with a menu bar, "
          "toolbars, and a status bar."));
}

void MainWindow::documentWasModified()
{
  setWindowModified(textEdit->document()->isModified());
}

void MainWindow::createStatusBar()
{
  statusBar()->showMessage(tr("Ready"));
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

  auto settings = lib::getQSettings();
  dockManager->loadPerspectives(settings);

  dockMenu = new QMenu(tr("&View"), this);
}

void MainWindow::createWidgets()
{

  textEdit = new QPlainTextEdit;

  ads::CDockWidget *dockLeft = new ads::CDockWidget(dockManager,
      tr("Left Panel"));
  dockLeft->setWidget(textEdit);
  dockManager->addDockWidget(ads::LeftDockWidgetArea, dockLeft);
  dockMenu->addAction(dockLeft->toggleViewAction());

  auto textEdit2 = new QPlainTextEdit;

  ads::CDockWidget *dockRight = new ads::CDockWidget(dockManager,
      tr("Right Panel"));
  dockRight->setWidget(textEdit2);
  dockManager->addDockWidget(ads::RightDockWidgetArea, dockRight);
  dockMenu->addAction(dockRight->toggleViewAction());

  //widgets need to be available to restore docks
  auto settings = lib::getQSettings();
  auto docks = settings.value("dock");
  if (docks.isValid() && (docks.toString() != "")) {
    dockManager->restoreState(docks.toByteArray());
  }
}

void MainWindow::createActions()
{
  //menubar / toolbar

  QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
  QToolBar *fileToolBar = addToolBar(tr("File"));
  const QIcon newIcon = QIcon::fromTheme("document-new",
      QIcon(":/images/new.png"));
  QAction *newAct = new QAction(newIcon, tr("&New"), this);
  newAct->setShortcuts(QKeySequence::New);
  newAct->setStatusTip(tr("Create a new file"));
  connect(newAct, &QAction::triggered, this, &MainWindow::newFile);
  fileMenu->addAction(newAct);
  fileToolBar->addAction(newAct);

  const QIcon openIcon = QIcon::fromTheme("document-open",
      QIcon(":/images/open.png"));
  QAction *openAct = new QAction(openIcon, tr("&Open..."), this);
  openAct->setShortcuts(QKeySequence::Open);
  openAct->setStatusTip(tr("Open an existing file"));
  connect(openAct, &QAction::triggered, this, &MainWindow::open);
  fileMenu->addAction(openAct);
  fileToolBar->addAction(openAct);

  const QIcon saveIcon = QIcon::fromTheme("document-save",
      QIcon(":/images/save.png"));
  QAction *saveAct = new QAction(saveIcon, tr("&Save"), this);
  saveAct->setShortcuts(QKeySequence::Save);
  saveAct->setStatusTip(tr("Save the document to disk"));
  connect(saveAct, &QAction::triggered, this, &MainWindow::save);
  fileMenu->addAction(saveAct);
  fileToolBar->addAction(saveAct);

  const QIcon saveAsIcon = QIcon::fromTheme("document-save-as");
  QAction *saveAsAct = fileMenu->addAction(saveAsIcon, tr("Save &As..."), this,
      &MainWindow::saveAs);
  saveAsAct->setShortcuts(QKeySequence::SaveAs);
  saveAsAct->setStatusTip(tr("Save the document under a new name"));

  fileMenu->addSeparator();

  const QIcon exitIcon = QIcon::fromTheme("application-exit");
  QAction *exitAct = fileMenu->addAction(exitIcon, tr("E&xit"), this,
      &QWidget::close);
  exitAct->setShortcuts(QKeySequence::Quit);
  exitAct->setStatusTip(tr("Exit the application"));

  QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
  QToolBar *editToolBar = addToolBar(tr("Edit"));

  const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(":/images/cut.png"));
  QAction *cutAct = new QAction(cutIcon, tr("Cu&t"), this);
  cutAct->setShortcuts(QKeySequence::Cut);
  cutAct->setStatusTip(tr("Cut the current selection's contents to the "
      "clipboard"));
  connect(cutAct, &QAction::triggered, textEdit, &QPlainTextEdit::cut);
  editMenu->addAction(cutAct);
  editToolBar->addAction(cutAct);

  const QIcon copyIcon = QIcon::fromTheme("edit-copy",
      QIcon(":/images/copy.png"));
  QAction *copyAct = new QAction(copyIcon, tr("&Copy"), this);
  copyAct->setShortcuts(QKeySequence::Copy);
  copyAct->setStatusTip(tr("Copy the current selection's contents to the "
      "clipboard"));
  connect(copyAct, &QAction::triggered, textEdit, &QPlainTextEdit::copy);
  editMenu->addAction(copyAct);
  editToolBar->addAction(copyAct);

  const QIcon pasteIcon = QIcon::fromTheme("edit-paste",
      QIcon(":/images/paste.png"));
  QAction *pasteAct = new QAction(pasteIcon, tr("&Paste"), this);
  pasteAct->setShortcuts(QKeySequence::Paste);
  pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
      "selection"));
  connect(pasteAct, &QAction::triggered, textEdit, &QPlainTextEdit::paste);
  editMenu->addAction(pasteAct);
  editToolBar->addAction(pasteAct);

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

  menuBar()->addSeparator();

  QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
  QAction *aboutAct = helpMenu->addAction(tr("&About"), this,
      &MainWindow::about);
  aboutAct->setStatusTip(tr("Show the application's About box"));

  QAction *aboutQtAct = helpMenu->addAction(tr("About &Qt"), qApp,
      &QApplication::aboutQt);
  aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));

  //signal / slot

  cutAct->setEnabled(false);
  copyAct->setEnabled(false);
  connect(textEdit, &QPlainTextEdit::copyAvailable, cutAct,
      &QAction::setEnabled);
  connect(textEdit, &QPlainTextEdit::copyAvailable, copyAct,
      &QAction::setEnabled);

  connect(textEdit->document(), &QTextDocument::contentsChanged, this,
      &MainWindow::documentWasModified);

  connect(qApp, &QGuiApplication::commitDataRequest, this,
      &MainWindow::commitData);
}

void MainWindow::readSettings()
{
  auto settings = lib::getQSettings();
  const QByteArray geometry =
      settings.value("geometry", QByteArray()).toByteArray();
  if (geometry.isEmpty()) {
    const QRect availableGeometry = screen()->availableGeometry();
    resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
    move((availableGeometry.width() - width()) / 2,
        (availableGeometry.height() - height()) / 2);
  } else {
    restoreGeometry(geometry);
    restoreState(settings.value("window").toByteArray());
  }
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
  if (!textEdit->document()->isModified())
    return true;
  const QMessageBox::StandardButton ret = QMessageBox::warning(this,
      tr("Application"), tr("The document has been modified.\n"
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
  QFile file(fileName);
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    QMessageBox::warning(this, tr("Application"),
        tr("Cannot read file %1:\n%2.").arg(QDir::toNativeSeparators(fileName),
            file.errorString()));
    return;
  }

  QTextStream in(&file);
  QGuiApplication::setOverrideCursor(Qt::WaitCursor);
  textEdit->setPlainText(in.readAll());
  QGuiApplication::restoreOverrideCursor();

  setCurrentFile(fileName);
  statusBar()->showMessage(tr("File loaded"), 2000);
}

bool MainWindow::saveFile(const QString &fileName)
{
  QString errorMessage;

  QGuiApplication::setOverrideCursor(Qt::WaitCursor);
  QSaveFile file(fileName);
  if (file.open(QFile::WriteOnly | QFile::Text)) {
    QTextStream out(&file);
    out << textEdit->toPlainText();
    if (!file.commit()) {
      errorMessage = tr("Cannot write file %1:\n%2.").arg(
          QDir::toNativeSeparators(fileName), file.errorString());
    }
  } else {
    errorMessage = tr("Cannot open file %1 for writing:\n%2.").arg(
        QDir::toNativeSeparators(fileName), file.errorString());
  }
  QGuiApplication::restoreOverrideCursor();

  if (!errorMessage.isEmpty()) {
    QMessageBox::warning(this, tr("Application"), errorMessage);
    return false;
  }

  setCurrentFile(fileName);
  statusBar()->showMessage(tr("File saved"), 2000);
  return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
  curFile = fileName;
  textEdit->document()->setModified(false);
  setWindowModified(false);

  QString shownName = curFile;
  if (curFile.isEmpty())
    shownName = "untitled.txt";
  setWindowFilePath(shownName);
}

QString MainWindow::strippedName(const QString &fullFileName)
{
  return QFileInfo(fullFileName).fileName();
}

void MainWindow::commitData(QSessionManager &manager)
{
  if (manager.allowsInteraction()) {
    if (!maybeSave())
      manager.cancel();
  } else {
    // Non-interactive: save without asking
    if (textEdit->document()->isModified())
      save();
  }
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
  dockManager->restoreState(dockDefault);
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
