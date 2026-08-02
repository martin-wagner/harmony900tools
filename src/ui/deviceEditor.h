// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QWidget>
#include <QString>

#include "context.h"
#include "logViewer.h"

class QAction;
class QItemSelection;
class QToolBar;
class QTreeView;

namespace models
{
class DeviceModel;
}

namespace editors
{

/**
 * @brief Flat device list editor with a toolbar for CRUDs.
 *
 * Wraps a DeviceModel in a QTreeView (single-level, alternating colors).
 */
class DeviceEditor: public QWidget
{
  Q_OBJECT

  public:
    explicit DeviceEditor(Context &ctx, models::DeviceModel *model, QWidget *parent = nullptr);
    ~DeviceEditor() override;

    /** set new model. nullptr = remove */
    void setModel(models::DeviceModel *model);

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

    /** Emitted whenever the selection changes. row == -1
      * when nothing is selected.*/
    void selectionChanged(int row, uint32_t deviceId);

  private slots:
    void onViewSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onAddDevice();
    void onRemoveDevice();
    void onModelRowCountChanged();
    void onUserLevelChanged(lib::UserLevel::Level l);
    void onSettingsChanged();

  private:
    Context &ctx;

  private:
    models::DeviceModel *model = nullptr;

    QTreeView *treeView = nullptr;
    QToolBar *toolbar = nullptr;
    QAction *actionAdd = nullptr;
    QAction *actionRemove = nullptr;

    void createView();
    void setupToolbar();
    void setupTreeView();
    void createActions();
    void updateActions();

    int getCurrentRow() const;
};

} // namespace views
