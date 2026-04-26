#include <QTextBrowser>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QVBoxLayout>
#include <QClipboard>
#include <QApplication>
#include <QScrollBar>
#include <QDateTime>

#include "lib/icon.h"
#include "logViewer.h"

LogViewer::LogViewer(QWidget *parent) :
    QWidget(parent)
{
  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  buildToolBar();
  layout->addWidget(toolBar);

  textBrowser = new QTextBrowser(this);
  textBrowser->setReadOnly(true);
  textBrowser->setOpenLinks(false);
  textBrowser->setFont(QFont("Monospace", 9));
  layout->addWidget(textBrowser);
}

void LogViewer::buildToolBar()
{
  toolBar = new QToolBar(this);
  toolBar->setIconSize(QSize(16, 16));

  // Copy
  auto *copyAction = toolBar->addAction(
      lib::getIcon(":/res/copy.png", "edit-copy"), tr("Copy"));
  copyAction->setToolTip(tr("Copy log to clipboard"));
  connect(copyAction, &QAction::triggered, this, &LogViewer::onCopy);

  // Clear
  auto *clearAction = toolBar->addAction(
      lib::getIcon(":/res/clear.png", "edit-clear"), tr("Clear"));
  clearAction->setToolTip(tr("Clear log"));
  connect(clearAction, &QAction::triggered, this, &LogViewer::onClear);

  toolBar->addSeparator();

  // Scroll-lock
  scrollLock = toolBar->addAction(
      lib::getIcon(":/res/icons8-scroll-lock-key-24.png", "object-locked"),
      tr("Scroll Lock"));
  scrollLock->setToolTip(tr("Toggle scroll lock"));
  scrollLock->setCheckable(true);
  scrollLock->setChecked(false);
  connect(scrollLock, &QAction::toggled, this, &LogViewer::onScrollLockToggled);
}

void LogViewer::setStatusBar(QStatusBar *statusBar)
{
  this->statusBar = statusBar;
}

void LogViewer::setMarkdown(const QString &markdown)
{
  entries.clear();
  LogEntry entry;
  entry.level = LogLevel::Notice;
  entry.message = markdown;
  entry.isMarkdown = true;
  entries.append(entry);
  rebuildView();
}

void LogViewer::setMaxEntries(int maxEntries)
{
  this->maxEntries = maxEntries;
  trimEntries();
  rebuildView();
}

int LogViewer::getMaxEntries() const
{
  return maxEntries;
}

void LogViewer::setLoglevel(LogLevel l)
{
  loglevel = l;
}

LogLevel LogViewer::getLogLevel()
{
  return loglevel;
}

void LogViewer::addEntry(LogLevel level, const QString &message,
    bool isMarkdown)
{
  if (level > loglevel) {
    return;
  }

  LogEntry entry;
  entry.level = level;
  entry.message = message;
  entry.isMarkdown = isMarkdown;
  entries.append(entry);

  trimEntries();

  if (isMarkdown) {
    // Rebuild the whole view when markdown is added – mixing HTML+markdown
    // inline is fragile, so we re-render from scratch.
    rebuildView();
  } else {
    // Append a single formatted line without full rebuild for performance.
    textBrowser->append(formatEntry(entry));
  }

  if (statusBar != nullptr) {
    statusBar->showMessage(QString("%1").arg(message), 10000);
  }

  if (!scrollLocked) {
    textBrowser->verticalScrollBar()->setValue(
        textBrowser->verticalScrollBar()->maximum());
  }

  emit entryAdded(level, message);
}

void LogViewer::addMessage(LogLevel level, const QString &message)
{
  addEntry(level, message, false);
}

void LogViewer::addMessage(const QString &message)
{
  addEntry(LogLevel::Notice, message, false);
}

void LogViewer::onCopy()
{
  QClipboard *clipboard = QApplication::clipboard();
  clipboard->setText(textBrowser->toPlainText());
}

void LogViewer::onClear()
{
  entries.clear();
  textBrowser->clear();
}

void LogViewer::onScrollLockToggled(bool locked)
{
  scrollLocked = locked;
  if (!locked) {
    textBrowser->verticalScrollBar()->setValue(
        textBrowser->verticalScrollBar()->maximum());
  }
}

void LogViewer::trimEntries()
{
  if (maxEntries <= 0) {
    return;
  }
  while (entries.size() > maxEntries) {
    entries.removeFirst();
  }
}

void LogViewer::rebuildView()
{
  textBrowser->clear();

  for (const LogEntry &entry : entries) {
    if (entry.isMarkdown) {
      // QTextBrowser supports a subset of HTML; convert markdown via Qt.
      // Qt 6.4+ has QTextDocument::setMarkdown – use it if available,
      // otherwise fall back to plain-text append.
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
      QTextDocument doc;
      doc.setMarkdown(entry.message);
      textBrowser->append(doc.toHtml());
#else
      textBrowser->append(entry.message);
#endif
    } else {
      textBrowser->append(formatEntry(entry));
    }
  }

  if (!scrollLocked) {
    textBrowser->verticalScrollBar()->setValue(
        textBrowser->verticalScrollBar()->maximum());
  }
}

// Returns an HTML-formatted line for a plain-text log entry.
QString LogViewer::formatEntry(const LogEntry &entry)
{
  const QString timestamp = QDateTime::currentDateTime().toString(
      "hh:mm:ss.zzz");
  const QString color = levelColor(entry.level);
  const QString name = logLevelName(entry.level);

  // Escape HTML special chars in the message.
  QString escaped = entry.message.toHtmlEscaped();

  return QString("<span style='color:#888;'>%1</span> "
      "<span style='color:%2; font-weight:bold;'>[%3]</span> "
      "<span>%4</span>").arg(timestamp, color, name, escaped);
}

QString LogViewer::levelColor(LogLevel level)
{
  switch (level) {
    case LogLevel::Emergency:
      return "#ff0000";
    case LogLevel::Alert:
      return "#ff2222";
    case LogLevel::Critical:
      return "#ff4444";
    case LogLevel::Error:
      return "#ff6600";
    case LogLevel::Warning:
      return "#ffaa00";
    case LogLevel::Notice:
      return "#aaddff";
    case LogLevel::Info:
      return "#aaffaa";
    case LogLevel::Debug:
      return "#888888";
    default:
      return "#ffffff";
  }
}
