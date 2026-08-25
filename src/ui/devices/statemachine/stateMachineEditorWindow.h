// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QWidget>
#include <QVector>

#include "document/data/items/state.h"

namespace document
{
namespace data
{
namespace item
{

class StateMachineListWidget;
class StateMachineDetailPanel;

/** top-level widget: list of a device's state machines on the left, detail
 * editor on the right, "add" and "edit with wizard" both routed through
 * AddStateMachineWizard.
 */
class StateMachineEditorWindow : public QWidget
{
    Q_OBJECT

  public:
    explicit StateMachineEditorWindow(QWidget *parent = nullptr);

    void setStateMachines(const QVector<StateMachine> &stateMachines);
    QVector<StateMachine> getStateMachines() const;

  private slots:
    void onCurrentRowChanged(int row);
    void onAddRequested();
    void onRerunWizardRequested();
    void onDeleteRequested();
    void onDetailChanged();

  private:
    void buildUi();
    void storeCurrentDetailEdits();

    StateMachineListWidget *listWidget = nullptr;
    StateMachineDetailPanel *detailPanel = nullptr;

    QVector<StateMachine> stateMachines;
    int currentRow = -1;
};

}
}
}
