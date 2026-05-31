#pragma once

#include <QtWidgets>
#include <QString>

#include "context.h"
#include "document/config.h"
#include "logViewer.h"
#include "deviceEditor.h"
#include "models/deviceListModel.h"

class DocumentTest: public QWidget
{
  Q_OBJECT

  public:
    explicit DocumentTest(Context &ctx, editors::DeviceEditor *editor, QWidget *parent = nullptr);
    ~DocumentTest();

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

  protected slots:
    void onImport();
    void onDump();

    void onRead();
    void onWrite();
    void onSave();

  protected:
    void createWidgets();
    void createActions();

  protected:
    void updateModelViews();

  private:
    Context &ctx;
    editors::DeviceEditor *deviceEditor = nullptr;

    document::Config config;
    models::DeviceModel *deviceModel = nullptr;

  private:
    QGridLayout *layout;

    QPushButton *buttonImportConfig;
    QPushButton *buttonDumpConfig;
    QPushButton *buttonReadConfig;
    QPushButton *buttonWriteConfig;
    QPushButton *buttonSaveConfig;
    QLabel *labelFileSize;

};
