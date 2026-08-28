// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QWizard>
#include <QMap>

#include "context.h"
#include "document/data/items/device.h"
#include "document/data/items/state.h"

class QRadioButton;
class QSpinBox;
class QListWidget;
class QPushButton;
class QLabel;
class QTableWidget;

namespace editors
{

class DeviceActionEditor;

/** wizard for creating a new StateMachine, or re-running against an
 * existing one to edit it. The same page set is used either way --
 * setStateMachine() pre-fills every page from the passed-in StateMachine,
 * and an empty StateMachine (the default) starts every page blank.
 */
class StateMachineWizard : public QWizard
{
  friend class ChooseFunctionPage;
  friend class DefineInputsPage;
  friend class AssignCommandsPage;
  Q_OBJECT

  public:
    static constexpr int POWER_ON_DELAY_ms = 10000;
    static constexpr int INPUT_SWITCH_DELAY_ms = 1000;

    enum PageId { PageChooseType, PageChooseFunction, PageDefineInputStates, PageAssignCommands, PageSetupCommands, PageReview };

    explicit StateMachineWizard(Context &ctx, uint32_t devicePos, QWidget *parent = nullptr);

    /** pre-fill from an existing state machine, for re-running
     * the wizard as an edit flow rather than a fresh add
     * @return false -- statemachine can't be edited using wizard*/
    bool setStateMachine(const document::data::item::StateMachine &stateMachine);

    document::data::item::StateMachine getStateMachine() const;

  private slots:
    void onAddStateClicked();
    void onRemoveStateClicked();
    void onPageChanged(int id);

  private:
    const document::data::item::Device &device;
    QStringList availableCommands;

    void buildChooseTypePage();
    void buildChooseFunctionPage();
    void buildDefineInputStatesPage();
    void buildAssignCommandsPage();
    void buildSetupCommandsPage();
    void buildReviewPage();

    void fillComboBox(QComboBox *box, QString text);

    QString reviewSummaryText() const;

    QRadioButton *discreteRadio = nullptr;
    QRadioButton *relativeRadio = nullptr;
    QRadioButton *rangeRadio = nullptr; //disabled placeholder, see class comment in .cpp

    QRadioButton *powerRadio = nullptr;
    QRadioButton *inputRadio = nullptr;
    QSpinBox *delaySpinBox = nullptr;

    QWizardPage *pageInputStates = nullptr;
    QListWidget *statesList = nullptr;
    QPushButton *addStateButton = nullptr;
    QPushButton *removeStateButton = nullptr;

    QTableWidget *commandsTable = nullptr;

    QCheckBox *startCommandEnabled = nullptr;
    QComboBox *startCommandCombo = nullptr;
    QComboBox *nextStateCombo = nullptr;
    QCheckBox *previousStateEnabled = nullptr;
    QComboBox *previousStateCombo = nullptr;
    QCheckBox *finishCommandEnabled = nullptr;
    QComboBox *finishCommandCombo = nullptr;

    QLabel *reviewSummaryLabel = nullptr;
};

class ChooseFunctionPage: public QWizardPage
{
  Q_OBJECT

  public:
    //fixed labels for power
    inline static const QStringList powerItems = { "On", "Off" };

    ChooseFunctionPage(StateMachineWizard &w, QWidget *parent = nullptr) :
        QWizardPage(parent), w(w) {};

    int nextId() const override;

  private:
    StateMachineWizard &w;

};

class DefineInputsPage: public QWizardPage
{
  Q_OBJECT

  public:
    DefineInputsPage(StateMachineWizard &w, QWidget *parent = nullptr) :
        QWizardPage(parent), w(w) {};

    int nextId() const override;

  private:
    StateMachineWizard &w;

};

class AssignCommandsPage: public QWizardPage
{
  Q_OBJECT

  public:
      AssignCommandsPage(StateMachineWizard &w, QWidget *parent = nullptr) :
        QWizardPage(parent), w(w) {};

    int nextId() const override;

  private:
    StateMachineWizard &w;

};

}
