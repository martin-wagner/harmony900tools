#pragma once

#include <QWidget>
#include <QString>
#include <QList>

#include "lib/logLevel.h"

class QTextBrowser;
class QToolBar;
class QStatusBar;
class QAction;

struct LogEntry
{
    LogLevel level;
    QString message;    // plain text or markdown
    bool isMarkdown;
};

class LogViewer: public QWidget
{
  Q_OBJECT

  public:
    explicit LogViewer(QWidget *parent = nullptr);

    // Attach an application status bar – last added message will be shown there.
    void setStatusBar(QStatusBar *statusBar);

    // Replace entire log content with markdown text.
    void setMarkdown(const QString &markdown);

    // Maximum number of entries kept (0 = unlimited).
    void setMaxEntries(int maxEntries);
    int getMaxEntries() const;

    // Minimum LogLevel
    void setLoglevel(LogLevel l);
    LogLevel getLogLevel();

  public slots:
    // Add a log entry. Message may be plain text or markdown.
    void addEntry(LogLevel level, const QString &message, bool isMarkdown = false);

    // Convenience overload: plain text shorthand.
    void addMessage(LogLevel level, const QString &message);
    void addMessage(const QString &message);

  signals:
    void entryAdded(LogLevel level, const QString &message);

  private slots:
    void onCopy();
    void onClear();
    void onScrollLockToggled(bool locked);

  private:
    void buildToolBar();
    void rebuildView();
    void trimEntries();
    static QString formatEntry(const LogEntry &entry);
    static QString levelColor(LogLevel level);

    QTextBrowser *textBrowser = nullptr;
    QToolBar *toolBar = nullptr;
    QStatusBar *statusBar = nullptr;
    QAction *scrollLock = nullptr;

    QList<LogEntry> entries;
    int maxEntries = 0;     // 0 = unlimited
    bool scrollLocked = false;
    LogLevel loglevel = LogLevel::Notice;
};
