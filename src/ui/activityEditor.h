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
class ActivityModel;
}

namespace editors
{

/**
 * @brief Flat activity list editor with a toolbar for CRUD and reorder operations.
 *
 * Wraps a ActivityModel in a QTreeView (single-level, alternating colors).
 */
class ActivityEditor: public QWidget
{
  Q_OBJECT

  public:
    explicit ActivityEditor(Context &ctx, models::ActivityModel *model, QWidget *parent = nullptr);
    ~ActivityEditor() override;

    /** set new model. nullptr = remove */
    void setModel(models::ActivityModel *model);

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

    /** Emitted whenever the selection changes. row == -1
      * when nothing is selected.*/
    void selectionChanged(int row, uint32_t activityId);

  private slots:
    void onViewSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onAddActivity();
    void onRemoveActivity();
    void onMoveUp();
    void onMoveDown();
    void onModelRowCountChanged();
    void onUserLevelChanged(lib::UserLevel::Level l);
    void onSettingsChanged();

  private:
    Context &ctx;

  private:
    models::ActivityModel *model = nullptr;

    QTreeView *treeView = nullptr;
    QToolBar *toolbar = nullptr;
    QAction *actionAdd = nullptr;
    QAction *actionRemove = nullptr;
    QAction *actionMoveUp = nullptr;
    QAction *actionMoveDown = nullptr;

    void createView();
    void setupToolbar();
    void setupTreeView();
    void createActions();
    void updateActions();

    int getCurrentRow() const;
};

} // namespace views
