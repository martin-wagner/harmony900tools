
#pragma once

#include <QMainWindow>

class QAction;
class QMenu;
class QPlainTextEdit;
class QSessionManager;

class MainWindow: public QMainWindow
{
  Q_OBJECT

  protected:
    int logLevel;

  public:
    MainWindow(int logLevel);

    void loadFile(const QString &fileName);

  protected:
    void closeEvent(QCloseEvent *event) override;

  private slots:
    void newFile();
    void open();
    bool save();
    bool saveAs();
    void about();
    void documentWasModified();
    void commitData(QSessionManager&);

  private:
    void createActions();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    bool maybeSave();
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);

    QPlainTextEdit *textEdit;
    QString curFile;
};

