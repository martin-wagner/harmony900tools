// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QWidget>
#include <QList>

#include "document/data/items/action.h"

class QVBoxLayout;
class QPushButton;
class QLabel;

namespace document
{
namespace data
{
namespace item
{

class ActionRowWidget;

/** editor for a whole DeviceAction (an ordered list of Action steps).
 *
 * this single widget backs every action slot in the state machine editor --
 * SetAction, ChangeAction, NextAction, PrevAction, ResetAction, StartAction,
 * FinishAction all reuse it, one instance per slot when opened.
 */
class DeviceActionEditor : public QWidget
{
    Q_OBJECT

  public:
    explicit DeviceActionEditor(QWidget *parent = nullptr);

    /** heading shown above the step list, e.g. "Set action - state Off" */
    void setTitle(const QString &title);

    /** load rows from an existing DeviceAction */
    void setDeviceAction(const DeviceAction &deviceAction);

    /** read the current rows back into a DeviceAction */
    DeviceAction getDeviceAction() const;

  signals:
    void changed();

  private slots:
    void onAddStepClicked();
    void onRowRemoveRequested();

  private:
    void buildUi();
    void addRow(const DeviceAction &action);
    void refreshDragHandles();

    QLabel *titleLabel = nullptr;
    QVBoxLayout *rowsLayout = nullptr;
    QPushButton *addStepButton = nullptr;
    QList<ActionRowWidget*> rows;
};

}
}
}
