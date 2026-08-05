// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QList>
#include <QString>
#include <QWidget>

#include "context.h"
#include "logViewer.h"
#include "models/buttonListModel.h"

class QAction;
class QToolBar;

namespace editors
{

class BaseTreeView;

/**
 * @brief Base editor: Holds all views, has one toolbar shared by a all members.
 *
 * The toolbar's actions apply to whichever child view currently
 * has focus.
 */
class BaseEditor: public QWidget
{
  Q_OBJECT

  public:
    explicit BaseEditor(Context &ctx, QWidget *parent = nullptr);
    ~BaseEditor() override;

  signals:
    void writeLog(LogLevel level, const QString &message, ContentType contentType);
    void writeMsg(const QString &message);

    /** Emitted whenever the base selection changes. row == -1
      * when nothing is selected.*/
    void selectionChanged(uint32_t id);

  protected slots:
    virtual void onAddClicked();
    virtual void onRemoveClicked();
    virtual void onMoveUpClicked();
    virtual void onMoveDownClicked();
    virtual void onAvailabilityChanged();
    virtual void onSelectionChanged(int row) = 0;

  protected:
    Context &ctx;

  protected:
    //views
    BaseTreeView *mainView = nullptr;
    QList<BaseTreeView *> childViews;
    /** last active view (of base and child views) */
    BaseTreeView *lastActiveView = nullptr;

    QVBoxLayout *layout = nullptr;
    QToolBar *toolbar = nullptr;
    QAction *actionAdd = nullptr;
    QAction *actionRemove = nullptr;
    QAction *actionMoveUp = nullptr;
    QAction *actionMoveDown = nullptr;

    virtual void createView();
    virtual void setupToolbar();
    virtual void createConnections();
    virtual void updateActions();
};

} // namespace editors
