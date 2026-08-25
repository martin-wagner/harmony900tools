// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QWizard>
#include <QMap>

#include "document/data/items/state.h"

class QRadioButton;
class QSpinBox;
class QListWidget;
class QPushButton;
class QLabel;

namespace document
{
namespace data
{
namespace item
{

class DeviceActionEditor;

/** wizard for creating a new StateMachine, or re-running against an
 * existing one to edit it. The same page set is used either way --
 * setStateMachine() pre-fills every page from the passed-in StateMachine,
 * and an empty StateMachine (the default) starts every page blank.
 */
class AddStateMachineWizard : public QWizard
{
    Q_OBJECT

  public:
    enum PageId { PageChooseType, PageDefineStates, PageAssignActions, PageReview };

    explicit AddStateMachineWizard(QWidget *parent = nullptr);

    /** pre-fill every page from an existing state machine, for re-running
     * the wizard as an edit flow rather than a fresh add */
    void setStateMachine(const StateMachine &stateMachine);

    StateMachine getStateMachine() const;

  private slots:
    void onActionSlotChanged(int row);
    void onAddStateClicked();
    void onRemoveStateClicked();
    void onPageChanged(int id);

  private:
    void buildChooseTypePage();
    void buildDefineStatesPage();
    void buildAssignActionsPage();
    void buildReviewPage();

    void rebuildActionSlots();
    void storeCurrentSlotEdits();
    QString reviewSummaryText() const;

    QRadioButton *discreteRadio = nullptr;
    QRadioButton *relativeRadio = nullptr;
    QRadioButton *rangeRadio = nullptr; //disabled placeholder, see class comment in .cpp

    QListWidget *statesList = nullptr;
    QPushButton *addStateButton = nullptr;
    QPushButton *removeStateButton = nullptr;

    QListWidget *actionSlotsList = nullptr;
    DeviceActionEditor *actionEditor = nullptr;
    //one DeviceAction per slot label ("Set: Off", "Next action", ...) --
    //rebuilt whenever the state list changes, preserved across page visits
    QMap<QString, DeviceAction> actionsBySlot;
    QString currentSlotKey;

    QLabel *reviewSummaryLabel = nullptr;
};

}
}
}
