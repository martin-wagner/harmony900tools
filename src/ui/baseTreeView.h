// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QString>
#include <QWidget>

#include "context.h"

class QAbstractItemModel;
class QItemSelection;
class QLabel;
class QTreeView;

namespace editors
{

/**
 * @brief Abstract base for flat, per-device/activity child tree views
 *
 * Wraps a QTreeView (single-level, alternating colors) for a model that
 * belongs to one selected device/activity. Assumes owner supplies a toolbar;
 * add/remove are driven through addRow()/removeRow()/canRemove().
 */
class BaseTreeView: public QWidget
{
  Q_OBJECT

  public:
    explicit BaseTreeView(Context &ctx, const QString &title = QString(), bool bold = true, QWidget *parent = nullptr);
    ~BaseTreeView() override;

    /** set the model, nullptr  */
    virtual void setModel(QAbstractItemModel *model) = 0;

    /** insert a new row after the current selection (or at the end) */
    void addRow();

    /** remove the currently selected row, if any */
    void removeRow();

    /** check for the tree implementing move operation */
    virtual bool supportsMoveOperation() const { return false; };

    /** check for available move operations */
    enum class MoveOperation { None, Up, Down, Both };
    MoveOperation availableMoveOperations() const;

    /** move the currently selected row one up */
    void moveUpRow();

    /** move the currently selected row one down */
    void moveDownRow();

    /** whether a row is currently selected and can be removed */
    bool canRemove() const;

    /** check wether an item is selected */
    bool hasSelection() const;

    /** check for the tree being active */
    bool isActive() const;

  signals:
    /** Emitted whenever the selection changes. row == -1
      * when nothing is selected.*/
    void selectionChanged(int row);

    /** Emitted whenever add/remove availability may have changed. */
    void availabilityChanged();

    /** Emitted when this view has a selection event */
    void activated(BaseTreeView *view);

  protected slots:
    void onViewSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onModelRowCountChanged();
    virtual void onUserLevelChanged(lib::UserLevel::Level l) {};
    virtual void onSettingsChanged();

  protected:
    bool eventFilter(QObject *object, QEvent *event) override;

  protected:
    Context &ctx;

    QAbstractItemModel *model = nullptr;

    QLabel *header = nullptr;
    QTreeView *treeView = nullptr;

    /** connect to the new model's rowsInserted/rowsRemoved and the
      * treeView's selectionModel. Call from the derived setModel(). */
    void bindModel(QAbstractItemModel *model);

    int getCurrentRow() const;
    int getCurrentColumn() const;

  private:
    void createView(const QString &title, bool bold);
    void setupTreeView();
    void createConnections();
    void createEventFilter();

};

} // namespace editors
