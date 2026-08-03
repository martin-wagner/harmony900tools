// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QList>
#include <QString>
#include <QWidget>

#include "context.h"
#include "logViewer.h"

class QAction;
class QToolBar;

namespace models
{
class DeviceModel;
class DeviceHardButtonModel;
}

namespace editors
{

class DeviceTreeView;
class BaseTreeView;
class DeviceHardButtonTreeView;

/**
 * @brief Device editor: one toolbar shared by a all members.
 *
 * The toolbar's actions apply to whichever child view currently
 * has focus.
 */
class DeviceEditor: public QWidget
{
  Q_OBJECT

  public:
    explicit DeviceEditor(Context &ctx, models::DeviceModel *model, QWidget *parent = nullptr);
    ~DeviceEditor() override;

    /** set new device model. nullptr = remove */
    void setModel(models::DeviceModel *model);

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

    /** Emitted whenever the device selection changes. row == -1
      * when nothing is selected.*/
    void selectionChanged(uint32_t deviceId);

  private slots:
    void onAddClicked();
    void onRemoveClicked();
    void onAvailabilityChanged();
    void onDeviceSelectionChanged(int row);

  private:
    Context &ctx;

  private:
    //views
    DeviceTreeView *deviceView = nullptr;
    /** all child views, in display order */
    DeviceHardButtonTreeView *hardButtonView = nullptr;
    QList<BaseTreeView *> childViews;
    //models
    models::DeviceHardButtonModel *hardButtonModel = nullptr;

    QToolBar *toolbar = nullptr;
    QAction *actionAdd = nullptr;
    QAction *actionRemove = nullptr;

    void createView();
    BaseTreeView *getActiveView();
    void setupToolbar();
    void createConnections();
    void updateActions();

    //create model for deviceId
    void updateHardButtonView(uint32_t deviceId);
};

} // namespace editors
