// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QWidget>

#include "context.h"
#include "document/data/items/state.h"

class QComboBox;
class QSpinBox;
class QPushButton;
class QStackedWidget;
class QLabel;
class QToolButton;
class QGroupBox;

namespace editors
{

class DiscreteStateEditor;
class RelativeStateEditor;

/** Type-specific editor for a single StateMachine.
 * The type-specific editor is chosen via a
 * QStackedWidget indexed by StateMachineType.
 */
class StateMachineDetailPanel : public QWidget
{
    Q_OBJECT

  public:
    explicit StateMachineDetailPanel(Context &ctx, QWidget *parent = nullptr);

    void setStateMachine(uint32_t devicePos, uint32_t smPos);

    void updateData();

    /** show an empty-state placeholder instead of an editor, e.g. when
     * nothing is selected in the state machine list */
    void showEmptyState();

  protected slots:
    void onEditingDelayFinished();
    void onRerunWizardClicked();
    void onEditStartActionClicked();
    void onEditFinishActionClicked();
    void onClearStartActionClicked();
    void onClearFinishActionClicked();

  private:
    Context &ctx;
    document::Config &config;
    uint32_t devicePos = 0xffffffff;
    uint32_t smPos = 0xffffffff;

    const document::data::item::StateMachine &getMachine() const;

    void createView(Context &ctx);
    void createConnections();
    int stackIndexForType(document::data::item::StateMachineType type) const;

    QLabel *smTypeLabel = nullptr;
    QSpinBox *delaySpinBox = nullptr;
    QToolButton *rerunWizardButton = nullptr;

    QGroupBox *startGroup;
    QPushButton *startActionButton = nullptr;
    QPushButton *startActionClearButton = nullptr;

    QGroupBox *finishGroup;
    QPushButton *finishActionButton = nullptr;
    QPushButton *finishActionClearButton = nullptr;

    QStackedWidget *typeStack = nullptr;
    QWidget *emptyStatePage = nullptr;
    DiscreteStateEditor *discreteEditor = nullptr;
    RelativeStateEditor *relativeEditor = nullptr;

    document::data::item::StateMachineType currentType = document::data::item::StateMachineType::Unknown;
};

}
