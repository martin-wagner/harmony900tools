// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QWidget>
#include <QVector>

#include "document/data/items/state.h"

class QListWidget;
class QToolButton;

namespace document
{
namespace data
{
namespace item
{

/** left-hand list of a device's state machines, with an "add" toolbar
 * button. Emits currentChanged with the row index whenever the selection
 * changes so the caller can swap the detail panel's content.
 */
class StateMachineListWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit StateMachineListWidget(QWidget *parent = nullptr);

    void setStateMachines(const QVector<StateMachine> &stateMachines);
    int currentRow() const;
    void setCurrentRow(int row);

  signals:
    void currentChanged(int row);
    void addRequested();

  private:
    void buildUi();
    QString labelForType(StateMachineType type) const;

    QListWidget *listWidget = nullptr;
    QToolButton *addButton = nullptr;
};

}
}
}
